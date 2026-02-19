#ifndef CORE_AUDIO_TYPES_H
#define CORE_AUDIO_TYPES_H

typedef enum {
    SND_NAV,
    SND_SELECT,
    SND_EAT,
    SND_BONUS,
    SND_GAME_OVER,
    SND_COUNT
} SoundType_t;

#define VOLUME_MAX     7
#define VOLUME_DEFAULT 5

#endif /* CORE_AUDIO_TYPES_H */
