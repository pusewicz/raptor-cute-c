#pragma once

typedef struct CF_Audio CF_Audio;
typedef enum Audio {
    MUSIC_BACKGROUND,
    SOUND_REVEAL,
    SOUND_GAME_OVER,
    SOUND_DEATH,
    SOUND_LASER,
    SOUND_EXPLOSION,
    SOUND_HIT,
    AUDIO_COUNT
} Audio;

CF_Audio load_audio(const char* path);
void     load_audios(void);
CF_Audio get_audio(const Audio audio);
void     play_sound(const Audio audio);
void     play_music(const Audio audio);
