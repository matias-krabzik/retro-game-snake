#ifndef AUDIO_H
#define AUDIO_H

typedef enum {
    SND_NAV,        /* menu navigation click */
    SND_SELECT,     /* menu option selected */
    SND_EAT,        /* snake eats food */
    SND_BONUS,      /* snake eats bonus */
    SND_GAME_OVER,  /* snake dies */
    SND_COUNT
} SoundType_t;

/* Initialize audio device and generate all sounds */
void audio_init(void);

/* Play a sound by type */
void audio_play(SoundType_t type);

/* Cleanup audio resources */
void audio_shutdown(void);

#endif /* AUDIO_H */
