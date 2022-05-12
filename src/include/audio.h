#ifndef H_AUDIO
#define H_AUDIO

#include <SDL_mixer.h>

/* Loads a wav for into memory and returns a pointer to i
    * \param path Filename of the wav to load
    * \return Pointer to the loaded wav
*/
Mix_Chunk *FE_LoadSFX(const char *path);


/* Plays the sound effect
    * \param sfx Pointer to the sound effect to play
 */
void FE_PlaySFX(Mix_Chunk *sfx);


/* Wrapper to destroy a Mix_Chunk
    * \param sfx Pointer to the sound effect to destroy
 */
void FE_FreeSFX(Mix_Chunk *sfx);


/* Sets and loads the active background music for the game
    * \param path Filename of the music to load
*/
void FE_LoadBGM(const char *path);

void FE_CleanAudio();
void FE_ChangeVolume(int volume);
void FE_MuteAudio();

#endif