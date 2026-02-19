#ifndef AUDIO_PORT_H
#define AUDIO_PORT_H

/* Re-export core audio types */
#include "../../core/audio_types.h"

/* Initialize audio device and generate all sounds */
void audio_init(void);

/* Play a sound by type */
void audio_play(SoundType_t type);

/* Cleanup audio resources */
void audio_shutdown(void);

/* Volume control (0 = muted, VOLUME_MAX = loudest) */
int  audio_get_volume(void);
void audio_set_volume(int level);
int  audio_volume_up(void);
int  audio_volume_down(void);

#endif /* AUDIO_PORT_H */
