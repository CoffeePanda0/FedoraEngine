#ifndef H_PARTICLE
#define H_PARTICLE

#include <vector.h>
#include <stdint.h>

typedef struct FE_Particle {
    vec2 velocity; // Velocity of the particle
    vec2 original_velocity; // Original velocity of the particle
    
    SDL_Rect body; // Body of the particle
    vec2 position; // Position of the particle
    vec2 original_position; // Original position of the particle
    
    uint16_t rotation; // Rotation of the particle

    bool expires; // If the particle expires with time
    uint64_t creation_time; // in ms (only applicable if expires is true)

    bool is_dead; // If the particle is dead
    bool spawned; // If the particle's properties have been set yet
} FE_Particle;

typedef struct FE_ParticleSystem {
    FE_Particle *particles; // Array of particles to render
    FE_Texture *texture; // Texture of the particles (add support for multiple textures later on)

    vec2 initial_velocity; // Initial velocity of the particles
    uint32_t particle_life; // Life of the particles in ms

    SDL_Rect emission_area; // Area to emit particles from
    vec2 max_size; // Max size of the particles

    uint32_t max_particles; // Maximum number of particles
    uint16_t num_particles; // Number of particles currently active

    uint16_t emission_rate; // Number of particles to emit per second
    float inv_rate; // Inverse of emission rate
    float emission_rate_timer; // Timer for the emission rate

    bool respawns; // If the particle system is dead (no particles left) then it will be removed from the world
    bool camera_locked; // If the particle system is not moved by the camera

    bool has_splash; // If the particle system has a splash effect
} FE_ParticleSystem;


/** Creates a new particle system
* \param emissionarea - Area to emit particles from
* \param emission_rate - Number of particles to emit per second
* \param max_particles - Maximum number of particles at any times
* \param particle_life - in ms (or 0 for infinite time)
* \param respawns - If the particle system is dead (no particles left) then it will be removed from the world
* \param texture - File name of the texture to use for the particles (in sprites/particles)
* \param max_size - Max size of the particles
* \param initial_velocity - Initial velocity of the particles
* \param camera_locked - If the particle system is not moved by the camera when the player moves
*/
FE_ParticleSystem *FE_ParticleSystem_Create(SDL_Rect emissionarea, uint16_t emission_rate, uint32_t max_particles, uint32_t particle_life, bool respawns, char *texture, vec2 max_size, vec2 initial_velocity, bool camera_locked);


/** Destroys a particle system, frees the pointer
 * \param p - Pointer to the particle system to destroy
 * \returns true on success, -false on failure
*/
bool FE_ParticleSystem_Destroy(FE_ParticleSystem *p);


/*
* Updates a particle system
* \param camera - Pointer to the camera for the particle system to follow
*/
void FE_Particles_Update(FE_Camera *camera);


/** Renders all particlesystems
* \param camera - The camera to render from
*/
void FE_Particles_Render(FE_Camera *camera);


// Destroys all particle systems
void FE_Particles_Clean();

#endif