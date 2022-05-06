#include "include/game.h"

static bool BGMPlaying = false;
static bool InitAudio = false;

#define AUDIO_DIRECTORY "game/audio/"

static Mix_Music *bgm = 0;

void FE_PlayBGM() {
    if (!bgm) {
        return;
    }
    if (BGMPlaying) {
        Mix_ResumeMusic();
    } else {
        Mix_PlayMusic(bgm, -1);
    }
    BGMPlaying = true;
}

void FE_LoadBGM(const char *path) {
    char *fp = AddStr(AUDIO_DIRECTORY, path);
    if (!InitAudio) {
      if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
         error("Failed to initialize audio: %s", Mix_GetError());
      }
      InitAudio = true; 
    }

    if (bgm) {
        Mix_FreeMusic(bgm);
        bgm = 0;
    }

    bgm = Mix_LoadMUS(fp);

    if (!bgm) {
        warn("Failed to load BGM: %s", Mix_GetError());
        free(fp);
        return;
    } else {
        info("Loaded BGM: '%s'", path);
    }
    FE_PlayBGM();
    BGMPlaying = true;
    free(fp);
}

void FE_PauseBGM() {
    if (!bgm) {
        return;
    }
    Mix_PauseMusic();
    BGMPlaying = false;
}

Mix_Chunk *FE_LoadSFX(const char *path) {
    if (!InitAudio) {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            error("Failed to initialize audio: %s", Mix_GetError());
        }
        InitAudio = true;
    }

    Mix_Chunk *sfx = Mix_LoadWAV(path);

    if (!sfx) {
        warn("Failed to load SFX: %s", Mix_GetError());
        return 0;
    }

    return sfx;
}

void FE_PlaySFX(Mix_Chunk *sfx) {
    if (!sfx) {
        warn("Passing nullptr! (PlaySFX)");
        return;
    }
    Mix_PlayChannel(-1, sfx, 0);
}

void FE_FreeSFX(Mix_Chunk *sfx) {
    if (!sfx) {
        warn("Passing nullptr! (FreeSFX)");
        return;
    }
    Mix_FreeChunk(sfx);
}

void FE_CleanAudio() {
    if (bgm) {
        Mix_FreeMusic(bgm);
        bgm = 0;
    }
    if (InitAudio) {
        Mix_CloseAudio();
        InitAudio = false;
    }
}

void FE_ChangeVolume(int volume)
{
    if (!bgm) {
        return;
    }
    
    PresentGame->AudioConfig.Muted = false;

    PresentGame->AudioConfig.Volume += volume;
    PresentGame->AudioConfig.Volume = clamp(PresentGame->AudioConfig.Volume, 0, 100);
    
    Mix_VolumeMusic(PresentGame->AudioConfig.Volume);
}

void FE_MuteAudio()
{
    if (PresentGame->AudioConfig.Muted) {
        PresentGame->AudioConfig.Muted = false;
        Mix_VolumeMusic(PresentGame->AudioConfig.Volume);
    } else {
        PresentGame->AudioConfig.Muted = true;
        Mix_VolumeMusic(0);
    }
}