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

/* Volume control (0 = muted, VOLUME_MAX = loudest) */
#define VOLUME_MAX     7
#define VOLUME_DEFAULT 5

int  audio_get_volume(void);
void audio_set_volume(int level);
int  audio_volume_up(void);
int  audio_volume_down(void);

#endif /* AUDIO_H */
