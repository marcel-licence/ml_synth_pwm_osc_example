/*
 * Copyright (c) 2021 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file easySynth.ino
 * @author Marcel Licence
 * @date 17.12.2021
 *
 * @brief   This file contains a simple implementation for a polyphonic synthesizer
 *
 * Implementation of a simple polyphonic synthesizer module
 * - it supports polyphony
 * - implemented ADSR for velocity and filter
 */


#ifdef __CDT_PARSER__
#include "cdt.h"
#endif


#ifdef USE_ML_SYNTH_PRO
#include <ml_oscillator_pro.h>
#else
#include <ml_oscillator.h>
#endif

#ifdef USE_ML_SYNTH_PRO
static ML_OscillatorPro *getFreeOsc(void);
#else
static ML_Oscillator *getFreeOsc(void);
#endif

static struct notePlayerT *getFreeVoice(void);


/*
 * Param indices for Synth_SetParam function
 */
#define SYNTH_PARAM_VEL_ENV_ATTACK  0
#define SYNTH_PARAM_VEL_ENV_DECAY   1
#define SYNTH_PARAM_VEL_ENV_SUSTAIN 2
#define SYNTH_PARAM_VEL_ENV_RELEASE 3
#define SYNTH_PARAM_FIL_ENV_ATTACK  4
#define SYNTH_PARAM_FIL_ENV_DECAY   5
#define SYNTH_PARAM_FIL_ENV_SUSTAIN 6
#define SYNTH_PARAM_FIL_ENV_RELEASE 7

#define SYNTH_PARAM_MAIN_FILT_CUTOFF    10
#define SYNTH_PARAM_MAIN_FILT_RESO      11
#define SYNTH_PARAM_VOICE_FILT_RESO     12
#define SYNTH_PARAM_VOICE_NOISE_LEVEL   13

#define SYNTH_PARAM_PITCH_BEND_RANGE    14
#define SYNTH_PARAM_MODULATION_SPEED    15
#define SYNTH_PARAM_MODULATION_PITCH    16

/*
 * Following defines can be changed for different puprposes
 */
#define MAX_POLY_OSC    10 /* osc polyphony, always active reduces single voices max poly */
#define MAX_POLY_VOICE  10 /* max single voices, can use multiple osc */

/*
 * this is just a kind of magic to go through the waveforms
 * - WAVEFORM_BIT sets the bit length of the pre calculated waveforms
 */
#define WAVEFORM_BIT    10UL
#define WAVEFORM_CNT    (1<<WAVEFORM_BIT)
#define WAVEFORM_Q4     (1<<(WAVEFORM_BIT-2))
#define WAVEFORM_MSK    ((1<<WAVEFORM_BIT)-1)
#define WAVEFORM_I(i)   (((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK)

#define MIDI_NOTE_CNT 128
static uint32_t midi_note_to_add[MIDI_NOTE_CNT]; /* lookup to playback waveforms with correct frequency */

uint8_t synth_waveform = 1;

#ifdef MEMORY_FROM_HEAP
float *sine = NULL;
#else
float sine[WAVEFORM_CNT];
#endif
struct adsrT
{
    float a;
    float d;
    float s;
    float r;
};

static struct adsrT adsr_vol = {0.25f, 0.25f, 1.0f, 0.01f};
static struct adsrT adsr_fil = {1.0f, 0.25f, 1.0f, 0.01f};

typedef enum
{
    attack, decay, sustain, release
} adsr_phaseT;

/* this prototype is required .. others not -  i still do not know what magic arduino is doing */
inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase);

struct filterCoeffT
{
    float aNorm[2] = {0.0f, 0.0f};
    float bNorm[3] = {1.0f, 0.0f, 0.0f};
};

struct filterProcT
{
    struct filterCoeffT *filterCoeff;
    float w[3];
};

static struct filterCoeffT filterGlobalC;
static struct filterProcT mainFilterL, mainFilterR;

static float modulationDepth = 0.0f;
static float modulationSpeed = 5.0f;
static float modulationPitch = 1.0f;
static float pitchBendValue = 0.0f;
static float pitchMultiplier = 1.0f;

#ifdef USE_ML_SYNTH_PRO
static ML_OscillatorPro oscPlayer[MAX_POLY_OSC];
#else
static ML_Oscillator oscPlayer[MAX_POLY_OSC];
#endif

static uint32_t osc_act = 0;

struct notePlayerT
{
    float lastSample[2][SAMPLE_BUFFER_SIZE];

    float velocity;
    bool active;
    adsr_phaseT phase;
    uint8_t midiNote;

    float control_sign;
    float out_level;

    struct filterCoeffT filterC;
    struct filterProcT filterL;
    struct filterProcT filterR;
    float f_control_sign;
    float f_control_sign_slow;
    adsr_phaseT f_phase;
};

static struct notePlayerT voicePlayer[MAX_POLY_VOICE];

static uint32_t voc_act = 0;


void Synth_Init(void)
{
#ifdef USE_ML_SYNTH_PRO
    ML_OscillatorPro::Setup(&Serial, SAMPLE_RATE);
#else
    ML_Oscillator::Setup(&Serial, SAMPLE_RATE);
#endif

    randomSeed(34547379);

    /*
     * we do not check if malloc was successful
     * if there is not enough memory left the application will crash
     */
#ifdef MEMORY_FROM_HEAP
    sine = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
#endif

    /*
     * let us calculate some waveforms
     * - using lookup tables can save a lot of processing power later
     * - but it does consume memory
     */
    for (int i = 0; i < WAVEFORM_CNT; i++)
    {
        float val = (float)sin(i * 2.0 * PI / WAVEFORM_CNT);
        sine[i] = val;
    }

    /*
     * initialize all oscillators
     */
    for (int i = 0; i < MAX_POLY_OSC; i++)
    {
#ifdef USE_ML_SYNTH_PRO
        ML_OscillatorPro *osc = &oscPlayer[i];
#else
        ML_Oscillator *osc = &oscPlayer[i];
#endif
        osc->Stop();
    }

    /*
     * initialize all voices
     */
    for (int i = 0; i < MAX_POLY_VOICE; i++)
    {
        notePlayerT *voice = &voicePlayer[i];
        voice->active = false;
        for (int n = 0; n < SAMPLE_BUFFER_SIZE; n++)
        {
            voice->lastSample[0][n] = 0.0f;
            voice->lastSample[1][n] = 0.0f;
        }
        voice->filterL.filterCoeff = &voice->filterC;
        voice->filterR.filterCoeff = &voice->filterC;
    }

    /*
     * prepare lookup for constants to drive oscillators
     */
    for (int i = 0; i < MIDI_NOTE_CNT; i++)
    {
        float f = ((pow(2.0f, (float)(i - 69) / 12.0f) * 440.0f));
        uint32_t add = (uint32_t)(f * ((float)(1ULL << 32ULL) / ((float)SAMPLE_RATE)));
        midi_note_to_add[i] = add;
#ifdef USE_UNISON
        /* filling the table which will be used for detuning */
        float f1 = (pow(2.0f, ((float)(i - 69) + 0.5f) / 12.0f) * 440.0f);
        float f2 = (pow(2.0f, ((float)(i - 69) - 0.5f) / 12.0f) * 440.0f);

        midi_note_to_add50c[i] = (uint32_t)((f1 - f2) * ((float)(1ULL << 32ULL) / ((float)SAMPLE_RATE)));
#endif
    }

    /*
     * assign main filter
     */
    mainFilterL.filterCoeff = &filterGlobalC;
    mainFilterR.filterCoeff = &filterGlobalC;
}

static struct filterCoeffT mainFilt;

/*
 * filter calculator:
 * https://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
 *
 * some filter implementations:
 * https://github.com/ddiakopoulos/MoogLadders/blob/master/src/Filters.h
 *
 * some more information about biquads:
 * https://www.earlevel.com/main/2003/02/28/biquads/
 */

static float filtCutoff = 1.0f;
static float filtReso = 0.5f;
static float soundFiltReso = 0.5f;
static float soundNoiseLevel = 0.0f;

/*
 * calculate coefficients of the 2nd order IIR filter
 */
inline void Filter_Calculate(float c, float reso, struct filterCoeffT *const  filterC)
{
    float *aNorm = filterC->aNorm;
    float *bNorm = filterC->bNorm;

    float Q = reso;
    float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

    /*
     * change curve of cutoff a bit
     * maybe also log or exp function could be used
     */
    c = c * c * c;

    if (c >= 1.0f)
    {
        omega = 1.0f;
    }
    else if (c < 0.0025f)
    {
        omega = 0.0025f;
    }
    else
    {
        omega = c;
    }

    /*
     * use lookup here to get quicker results
     */
    cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
    sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];

    alpha = sinOmega / (2.0 * Q);
    b[0] = (1 - cosOmega) / 2;
    b[1] = 1 - cosOmega;
    b[2] = b[0];
    a[0] = 1 + alpha;
    a[1] = -2 * cosOmega;
    a[2] = 1 - alpha;

    // Normalize filter coefficients
    float factor = 1.0f / a[0];

    aNorm[0] = a[1] * factor;
    aNorm[1] = a[2] * factor;

    bNorm[0] = b[0] * factor;
    bNorm[1] = b[1] * factor;
    bNorm[2] = b[2] * factor;
}

inline void Filter_Process(float *const signal, struct filterProcT *const filterP)
{
    const float out = filterP->filterCoeff->bNorm[0] * (*signal) + filterP->w[0];
    filterP->w[0] = filterP->filterCoeff->bNorm[1] * (*signal) - filterP->filterCoeff->aNorm[0] * out + filterP->w[1];
    filterP->w[1] = filterP->filterCoeff->bNorm[2] * (*signal) - filterP->filterCoeff->aNorm[1] * out;
    *signal = out;
}

/*
 * very bad and simple implementation of ADSR
 * - but it works for the start
 */
inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase)
{
    switch (*phase)
    {
    case attack:
        *ctrlSig += ctrl->a;
        if (*ctrlSig > 1.0f)
        {
            *ctrlSig = 1.0f;
            *phase = decay;
        }
        break;
    case decay:
        *ctrlSig -= ctrl->d;
        if (*ctrlSig < ctrl->s)
        {
            *ctrlSig = ctrl->s;
            *phase = sustain;
        }
        break;
    case sustain:
        break;
    case release:
        *ctrlSig -= ctrl->r;
        if (*ctrlSig < 0.0f)
        {
            *ctrlSig = 0.0f;
            //voice->active = false;
            return false;
        }
    }
    return true;
}

void Voice_Off(uint32_t i)
{
    notePlayerT *voice = &voicePlayer[i];
    for (int f = 0; f < MAX_POLY_OSC; f++)
    {
#ifdef USE_ML_SYNTH_PRO
        ML_OscillatorPro *osc = &oscPlayer[f];
#else
        ML_Oscillator *osc = &oscPlayer[f];
#endif
        if (osc->isCon(voice->lastSample[0]))
        {
            osc->Stop();
            osc_act -= 1;
        }
    }
    voc_act -= 1;
}

inline
float SineNorm(float alpha_div2pi)
{
    uint32_t index = ((uint32_t)(alpha_div2pi * ((float)WAVEFORM_CNT))) % WAVEFORM_CNT;
    return sine[index];
}

inline
float GetModulation(void)
{
    float modSpeed = modulationSpeed;
    return modulationDepth * modulationPitch * (SineNorm((modSpeed * ((float)millis()) / 1000.0f)));
}

//[[gnu::noinline, gnu::optimize ("fast-math")]]
inline void Synth_Process(float *left, float *right, uint32_t len)
{
    {
        float pitchVar = pitchBendValue + GetModulation();
        pitchMultiplier = pow(2.0f, pitchVar / 12.0f);
    }

    /*
     * prepare all voices
     */
    for (int i = 0; i < MAX_POLY_VOICE; i++) /* one loop is faster than two loops */
    {
        notePlayerT *voice = &voicePlayer[i];
        for (uint32_t n = 0; n < len; n++)
        {
            voice->lastSample[0][n] = 0;
            voice->lastSample[1][n] = 0;
        }
    }

#if 1
    /*
     * oscillator processing -> mix to voice
     */
    for (int i = 0; i < MAX_POLY_OSC; i++)
    {
        oscPlayer[i].SetPitchMultiplier(pitchMultiplier);
        oscPlayer[i].ProcessBuffer(len);
        //oscPlayer[i].ProcessBuffer(left, right, 1);
    }

#endif

    float noise_signal[len];

    for (uint32_t n = 0; n < len; n++)
    {
        /* generate a noise signal */
        noise_signal[n] = ((random(1024) / 512.0f) - 1.0f) * soundNoiseLevel;
    }

    for (int i = 0; i < MAX_POLY_VOICE; i++) /* one loop is faster than two loops */
    {
        notePlayerT *voice = &voicePlayer[i];
        if (voice->active)
        {

            for (uint32_t n = 0; n < len; n++)
            {
                /* add some noise to the voice */
                voice->lastSample[0][n] += noise_signal[n];
                voice->lastSample[1][n] += noise_signal[n];
            }

            for (uint32_t n = 0; n < len; n++)
            {
                if (n % 4 == 0)
                {
                    voice->active = ADSR_Process(&adsr_vol, &voice->control_sign, &voice->phase);
                    if (voice->active == false)
                    {
                        Voice_Off(i);
                    }
                    /*
                     * make is slow to avoid bad things .. or crying ears
                     */
                    (void)ADSR_Process(&adsr_fil, &voice->f_control_sign, &voice->f_phase);
                }

                voice->lastSample[0][n] *= voice->control_sign * voice->velocity;
                voice->lastSample[1][n] *= voice->control_sign * voice->velocity;

                if (n % 32 == 0)
                {
                    voice->f_control_sign_slow = 0.05 * voice->f_control_sign + 0.95 * voice->f_control_sign_slow;
                    Filter_Calculate(voice->f_control_sign_slow, soundFiltReso, &voice->filterC);
                }

                Filter_Process(&voice->lastSample[0][n], &voice->filterL);
                Filter_Process(&voice->lastSample[1][n], &voice->filterR);

                left[n] += voice->lastSample[0][n] * 0.4f * 0.25f ;
                right[n] += voice->lastSample[1][n] * 0.4f * 0.25f ;
            }
        }
    }

    /*
     * process main filter
     */
    for (uint32_t n = 0; n < len; n++)
    {
        Filter_Process(&left[n], &mainFilterL);
        Filter_Process(&right[n], &mainFilterR);
    }
}

#ifdef USE_ML_SYNTH_PRO
static ML_OscillatorPro *getFreeOsc(void)
#else
static ML_Oscillator *getFreeOsc(void)
#endif
{
    for (int i = 0; i < MAX_POLY_OSC ; i++)
    {
        if (oscPlayer[i].isFree())
        {
            return &oscPlayer[i];
        }
    }
    return NULL;
}

static struct notePlayerT *getFreeVoice(void)
{
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if (voicePlayer[i].active == false)
        {
            return &voicePlayer[i];
        }
    }
    return NULL;
}

inline void Filter_Reset(struct filterProcT *filter)
{
    filter->w[0] = 0.0f;
    filter->w[1] = 0.0f;
    filter->w[2] = 0.0f;
}

inline void Synth_NoteOn(uint8_t ch, uint8_t note, float vel)
{
    struct notePlayerT *voice = getFreeVoice();
#ifdef USE_ML_SYNTH_PRO
    ML_OscillatorPro *osc = getFreeOsc();
#else
    ML_Oscillator *osc = getFreeOsc();
#endif

    /*
     * No free voice found, return otherwise crash xD
     */
    if ((voice == NULL) || (osc == NULL))
    {
        //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
        return ;
    }

    voice->midiNote = note;
#ifdef MIDI_USE_CONST_VELOCITY
    voice->velocity = 1.0f;
#else
    voice->velocity = vel;
#endif
    for (int n = 0; n < SAMPLE_BUFFER_SIZE; n++)
    {
        voice->lastSample[0][n] = 0.0f;
        voice->lastSample[1][n] = 0.0f;
    }
    voice->control_sign = 0.0f;

    voice->f_phase = attack;
    if (adsr_fil.a == 1)
    {
        voice->f_phase = decay;
    }
    voice->f_control_sign = adsr_fil.a;
    voice->f_control_sign_slow = adsr_fil.a;
    voice->active = true;
    voice->phase = attack;

    /* update all values to avoid audible artifacts */
    ADSR_Process(&adsr_vol, &voice->control_sign, &voice->phase);
    ADSR_Process(&adsr_fil, &voice->f_control_sign, &voice->f_phase);

    Filter_Calculate(voice->f_control_sign_slow, soundFiltReso, &voice->filterC);

    voc_act += 1;

    /*
     * add oscillator
     */
    osc->Start(voice->lastSample[0], voice->lastSample[1], midi_note_to_add[note]);
    osc->SetWaveform(&synth_waveform); /* link selected waveform */

    osc_act += 1;

    /*
     * trying to avoid audible suprises
     */
    Filter_Reset(&voice->filterL);
    Filter_Reset(&voice->filterR);
    Filter_Process(&voice->lastSample[0][0], &voice->filterL);
    Filter_Process(&voice->lastSample[0][0], &voice->filterL);
    Filter_Process(&voice->lastSample[0][0], &voice->filterL);

    Filter_Process(&voice->lastSample[1][0], &voice->filterR);
    Filter_Process(&voice->lastSample[1][0], &voice->filterR);
    Filter_Process(&voice->lastSample[1][0], &voice->filterR);
}

inline void Synth_NoteOff(uint8_t ch, uint8_t note)
{
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if ((voicePlayer[i].active) && (voicePlayer[i].midiNote == note))
        {
            voicePlayer[i].phase = release;
        }
    }
}

void Synth_ModulationWheel(uint8_t ch, float value)
{
    modulationDepth = value;
}

void Synth_ModulationSpeed(uint8_t ch, float value)
{
    modulationSpeed = value * 10;
    //Status_ValueChangedFloat("ModulationSpeed", modulationSpeed);
}

void Synth_ModulationPitch(uint8_t ch, float value)
{
    modulationPitch = value * 5;
    //Status_ValueChangedFloat("ModulationDepth", modulationPitch);
}

void Synth_PitchBend(uint8_t ch, float bend)
{
    pitchBendValue = bend;
    //Serial.printf("pitchBendValue: %0.3f\n", pitchBendValue);
}

void Synth_SetWaveForm(uint8_t waveform, float value)
{
    if (value > 0)
    {
        synth_waveform = waveform;
        //Status_ValueChangedInt("waveform", waveform);
    }
}

void Synth_SetParam(uint8_t slider, float value)
{
    switch (slider)
    {
    case SYNTH_PARAM_VEL_ENV_ATTACK:
        if (value == 0)
        {
            adsr_vol.a = 1.0f;
        }
        else
        {
            adsr_vol.a = (0.00005 * pow(5000, 1.0f - value));
        }
        Serial.printf("voice volume attack: %0.6f\n", adsr_vol.a);
        break;
    case SYNTH_PARAM_VEL_ENV_DECAY:
        adsr_vol.d = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice volume decay: %0.6f\n", adsr_vol.d);
        break;
    case SYNTH_PARAM_VEL_ENV_SUSTAIN:
        adsr_vol.s = (0.01 * pow(100, value));
        Serial.printf("voice volume sustain: %0.6f\n", adsr_vol.s);
        break;
    case SYNTH_PARAM_VEL_ENV_RELEASE:
        adsr_vol.r = (0.0001 * pow(100, 1.0f - value));
        Serial.printf("voice volume release: %0.6f\n", adsr_vol.r);
        break;

    case SYNTH_PARAM_FIL_ENV_ATTACK:
        if (value == 0)
        {
            adsr_fil.a = 1.0f;
        }
        else
        {
            adsr_fil.a = (0.00005 * pow(5000, 1.0f - value));
        }
        Serial.printf("voice filter attack: %0.6f\n", adsr_fil.a);
        break;
    case SYNTH_PARAM_FIL_ENV_DECAY:
        adsr_fil.d = (0.00005 * pow(5000, 1.0f - value));
        Serial.printf("voice filter decay: %0.6f\n", adsr_fil.d);
        break;
    case SYNTH_PARAM_FIL_ENV_SUSTAIN:
        adsr_fil.s = value;
        Serial.printf("voice filter sustain: %0.6f\n", adsr_fil.s);
        break;
    case SYNTH_PARAM_FIL_ENV_RELEASE:
        adsr_fil.r = (0.0001 * pow(100, 1.0f - value));
        Serial.printf("voice filter release: %0.6f\n", adsr_fil.r);
        break;

    case SYNTH_PARAM_MAIN_FILT_CUTOFF:
        filtCutoff = value;
        Serial.printf("main filter cutoff: %0.3f\n", filtCutoff);
        Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
        break;
    case SYNTH_PARAM_MAIN_FILT_RESO:
        filtReso =  0.5f + 10 * value * value * value; /* min q is 0.5 here */
        Serial.printf("main filter reso: %0.3f\n", filtReso);
        Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
        break;

    case SYNTH_PARAM_VOICE_FILT_RESO:
        soundFiltReso = 0.5f + 10 * value * value * value; /* min q is 0.5 here */
        Serial.printf("voice filter reso: %0.3f\n", soundFiltReso);
        break;

    case SYNTH_PARAM_VOICE_NOISE_LEVEL:
        soundNoiseLevel = value;
        Serial.printf("voice noise level: %0.3f\n", soundNoiseLevel);
        break;

    default:
        /* not connected */
        break;
    }
}

