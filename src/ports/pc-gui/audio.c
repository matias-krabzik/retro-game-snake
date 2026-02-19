#include "audio.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SAMPLE_RATE  44100
#define PI_F         3.14159265358979f

static Sound sounds[SND_COUNT];
static int current_volume = VOLUME_DEFAULT;

static void apply_volume(void)
{
    SetMasterVolume((float)current_volume / (float)VOLUME_MAX);
}

/* --- Helpers --- */

static void add_tone(int16_t *buf, int offset, int samples,
                     float freq, float vol)
{
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float v = sinf(2.0f * PI_F * freq * t) * vol * 32767.0f;
        int32_t sum = (int32_t)buf[offset + i] + (int32_t)v;
        if (sum > 32767) sum = 32767;
        if (sum < -32767) sum = -32767;
        buf[offset + i] = (int16_t)sum;
    }
}

static void add_sweep(int16_t *buf, int offset, int samples,
                      float f0, float f1, float vol)
{
    float phase = 0.0f;
    for (int i = 0; i < samples; i++) {
        float progress = (float)i / (float)samples;
        float freq = f0 + (f1 - f0) * progress;
        phase += 2.0f * PI_F * freq / SAMPLE_RATE;
        float v = sinf(phase) * vol * 32767.0f;
        int32_t sum = (int32_t)buf[offset + i] + (int32_t)v;
        if (sum > 32767) sum = 32767;
        if (sum < -32767) sum = -32767;
        buf[offset + i] = (int16_t)sum;
    }
}

static void apply_envelope(int16_t *buf, int samples, int attack, int release)
{
    for (int i = 0; i < attack && i < samples; i++) {
        buf[i] = (int16_t)((float)buf[i] * (float)i / (float)attack);
    }
    for (int i = 0; i < release && i < samples; i++) {
        int idx = samples - 1 - i;
        buf[idx] = (int16_t)((float)buf[idx] * (float)i / (float)release);
    }
}

static Sound build_sound(int16_t *data, int total_samples)
{
    Wave wave = {
        .frameCount = (unsigned int)total_samples,
        .sampleRate = SAMPLE_RATE,
        .sampleSize = 16,
        .channels = 1,
        .data = data
    };
    Sound s = LoadSoundFromWave(wave);
    free(data);
    return s;
}

#define MS_TO_SAMPLES(ms) ((SAMPLE_RATE * (ms)) / 1000)

/* --- Sound generators --- */

static Sound gen_nav(void)
{
    int n = MS_TO_SAMPLES(30);
    int16_t *buf = calloc((size_t)n, sizeof(int16_t));
    add_tone(buf, 0, n, 1000.0f, 0.15f);
    add_tone(buf, 0, n, 2000.0f, 0.08f);
    apply_envelope(buf, n, MS_TO_SAMPLES(2), MS_TO_SAMPLES(8));
    return build_sound(buf, n);
}

static Sound gen_select(void)
{
    int half = MS_TO_SAMPLES(60);
    int n = half * 2;
    int16_t *buf = calloc((size_t)n, sizeof(int16_t));
    /* Two ascending tones (fifth interval) */
    add_tone(buf, 0,    half, 880.0f,  0.15f);
    add_tone(buf, 0,    half, 1760.0f, 0.07f);
    add_tone(buf, half, half, 1320.0f, 0.15f);
    add_tone(buf, half, half, 2640.0f, 0.07f);
    apply_envelope(buf, n, MS_TO_SAMPLES(2), MS_TO_SAMPLES(15));
    return build_sound(buf, n);
}

static Sound gen_eat(void)
{
    int n = MS_TO_SAMPLES(70);
    int16_t *buf = calloc((size_t)n, sizeof(int16_t));
    /* Ascending chirp with octave harmonic */
    add_sweep(buf, 0, n, 800.0f, 1600.0f, 0.15f);
    add_sweep(buf, 0, n, 1600.0f, 3200.0f, 0.06f);
    apply_envelope(buf, n, MS_TO_SAMPLES(2), MS_TO_SAMPLES(15));
    return build_sound(buf, n);
}

static Sound gen_bonus(void)
{
    int note = MS_TO_SAMPLES(60);
    int n = note * 3;
    int16_t *buf = calloc((size_t)n, sizeof(int16_t));
    /* Three-note polyphonic arpeggio: C5+E5, E5+G5, G5+C6 */
    add_tone(buf, 0,        note, 523.0f, 0.12f);
    add_tone(buf, 0,        note, 659.0f, 0.12f);
    add_tone(buf, note,     note, 659.0f, 0.12f);
    add_tone(buf, note,     note, 784.0f, 0.12f);
    add_tone(buf, note * 2, note, 784.0f, 0.12f);
    add_tone(buf, note * 2, note, 1046.0f, 0.12f);
    apply_envelope(buf, n, MS_TO_SAMPLES(2), MS_TO_SAMPLES(20));
    return build_sound(buf, n);
}

static Sound gen_game_over(void)
{
    int n = MS_TO_SAMPLES(500);
    int16_t *buf = calloc((size_t)n, sizeof(int16_t));
    /* Descending dual sweep */
    add_sweep(buf, 0, n, 800.0f, 200.0f, 0.15f);
    add_sweep(buf, 0, n, 1200.0f, 300.0f, 0.08f);
    apply_envelope(buf, n, MS_TO_SAMPLES(5), MS_TO_SAMPLES(100));
    return build_sound(buf, n);
}

/* --- Public API --- */

void audio_init(void)
{
    InitAudioDevice();
    sounds[SND_NAV]       = gen_nav();
    sounds[SND_SELECT]    = gen_select();
    sounds[SND_EAT]       = gen_eat();
    sounds[SND_BONUS]     = gen_bonus();
    sounds[SND_GAME_OVER] = gen_game_over();
    apply_volume();
}

void audio_play(SoundType_t type)
{
    if (type >= 0 && type < SND_COUNT) {
        PlaySound(sounds[type]);
    }
}

void audio_shutdown(void)
{
    for (int i = 0; i < SND_COUNT; i++) {
        UnloadSound(sounds[i]);
    }
    CloseAudioDevice();
}

int audio_get_volume(void)
{
    return current_volume;
}

void audio_set_volume(int level)
{
    if (level < 0) level = 0;
    if (level > VOLUME_MAX) level = VOLUME_MAX;
    current_volume = level;
    apply_volume();
}

int audio_volume_up(void)
{
    audio_set_volume(current_volume + 1);
    return current_volume;
}

int audio_volume_down(void)
{
    audio_set_volume(current_volume - 1);
    return current_volume;
}
