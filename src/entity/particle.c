#include "../core/include/include.h"
#include "include/particle.h"

#define AssetPath "game/sprites/particles/"

static const double PARTICLE_GRAVITY = 0.1;
static const double PARTICLE_DRAG = 0.5;
static const double MAX_VELOCITY = 7;

static FE_List *ParticleSystems = 0;

static void GenerateParticle(FE_ParticleSystem *p, size_t index)
{
    if (!p) {
        warn("GenerateParticle() called with NULL particle system");
        return;
    }

    FE_Particle particle;

    // Generate random location inside emission area
    particle.body.x = p->emission_area.x + s_Rand(p->emission_area.w);
    particle.body.y = p->emission_area.y + s_Rand(p->emission_area.h);

    particle.position.x = particle.body.x;
    particle.position.y = particle.body.y;

    particle.body.w = s_Rand((int)p->max_size.x);
    particle.body.h = s_Rand((int)p->max_size.y);
    particle.rotation = 0;

    // Generate random velocity
    particle.velocity.x = -s_Rand((int)p->initial_velocity.x) + s_Rand((int)p->initial_velocity.x); // dispersion either side
    particle.velocity.y = p->initial_velocity.y;

    particle.texture = p->texture;
    particle.is_dead = false;

    if (p->particle_life > 0) {
        particle.expires = true;
        particle.life = p->particle_life;
        particle.creation_time = SDL_GetTicks();
    } else {
        particle.expires = false;
    }

    p->particles[index] = particle;
}

FE_ParticleSystem *FE_ParticleSystem_Create(GPU_Rect emissionarea, Uint16 emission_rate, Uint16 max_particles, Uint16 particle_life, bool respawns, char *texture, vec2 max_size, vec2 initial_velocity, bool camera_locked)
{
    FE_ParticleSystem *p = xmalloc(sizeof(FE_ParticleSystem));
    p->emission_area = emissionarea;
    p->max_particles = max_particles;
    p->emission_rate = emission_rate; 
    p->inv_rate = 1.0f / emission_rate;
    p->respawns = respawns;
    p->max_size = max_size;
    p->initial_velocity = initial_velocity;
    p->num_particles = 0;
    p->particle_life = particle_life;
    
    p->emission_rate_timer = 0;
    p->camera_locked = camera_locked;

    char *path = mstradd(AssetPath, texture);
    p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    free(path);

    p->particles = xmalloc(sizeof(FE_Particle) * max_particles);
    for (size_t i = 0; i < max_particles; i++) {
        // if the particles don't respawn, we want to emit them all at once (e.g explosion)
        if (p->respawns) {
            p->particles[i].is_dead = true;
            p->num_particles = 0;
        } else {
            GenerateParticle(p, i);
            p->num_particles++;
        }
    }
    
    FE_List_Add(&ParticleSystems, p);

    return p;
}

void FE_Particles_Update()
{
    if (!ParticleSystems)
        return;

    for (FE_List *ps = ParticleSystems; ps; ps = ps->next) { // loop through each particle system
        FE_ParticleSystem *p = ps->data;

        uint16_t to_emit = 0; // The amount of particles to emit this frame
        if (p->respawns) {
            p->emission_rate_timer += FE_DT;
            if (p->emission_rate_timer > p->inv_rate) {
                if (p->num_particles < p->max_particles) {
                    // Calculate how many particles to emit this frame based on FPS
                    to_emit = (uint16_t)(p->emission_rate * FE_DT);
                    to_emit = clamp(to_emit, 1, p->max_particles - p->num_particles);
                }
                p->emission_rate_timer = 0;
            }
        }

        // check each particle inside the system
        for (size_t i = 0; i < p->max_particles; i++) {
            FE_Particle *particle = &p->particles[i];    
            if (particle->is_dead) { // if we are already due to emit a new particle, use this one
                if (to_emit > 0) {
                    GenerateParticle(p, i);
                    to_emit--;
                    p->num_particles++;
                } else continue;
            }

            // update particle timer
            if (particle->expires) {
                Uint32 current_time = SDL_GetTicks() - particle->creation_time;
                if (current_time > particle->life) { // if particle has expired
                    particle->is_dead = true; // more efficient to set a particle to be re-written than moving contents of array
                    p->num_particles--;
                    if (!p->respawns) {
                        if (p->num_particles == 0) { // destroy particle systems if none remain
                            FE_ParticleSystem_Destroy(p);
                            return;
                        }
                    }
                    continue;
                }
            }

            // apply gravity and friction to all particles
            float new_velocity_y = particle->velocity.y += (PARTICLE_GRAVITY * FE_DT_MULTIPLIER);
            particle->velocity.y = clamp(new_velocity_y, -MAX_VELOCITY, MAX_VELOCITY);
            particle->velocity.x *= (1.0f - (PARTICLE_DRAG * FE_DT));

            // separate position as floating point to hold deltatime small values
            particle->position.y += (particle->velocity.y * FE_DT_MULTIPLIER);
            particle->position.x += (particle->velocity.x * FE_DT_MULTIPLIER);

            FE_UPDATE_RECT(particle->position, &particle->body);

            // kill particle if it's out of map bounds
            if (particle->body.x < 0 || particle->body.x > PresentGame->MapConfig.MapWidth ||  particle->body.y > (PresentGame->MapConfig.MapHeight - particle->body.h)) {
                particle->is_dead = true;
                p->num_particles--;
            }
            
        }
    }
    
}

void FE_Particles_Render(FE_Camera *camera)
{
    for (FE_List *l = ParticleSystems; l; l = l->next) {
        FE_ParticleSystem *p = l->data;
        for (size_t i = 0; i < p->max_particles; i++) {
            if (p->particles[i].is_dead)
                continue;

            FE_RenderCopyEx(0, camera, p->camera_locked, p->particles[i].texture, NULL, &p->particles[i].body, p->particles[i].rotation, GPU_FLIP_NONE);
        }
    }
}

bool FE_ParticleSystem_Destroy(FE_ParticleSystem *p)
{
    if (!p) {
        warn("FE_ParticleSystem_Destroy() called with NULL particle system");
        return false;
    }

    free(p->particles);
    FE_DestroyResource(p->texture->path);

    FE_List_Remove(&ParticleSystems, p);
    
    free(p);
    return true;
}

void FE_Particles_Clean()
{
    if (!ParticleSystems)
        return;

    for (FE_List *l = ParticleSystems; l; l = l->next) {
        FE_ParticleSystem *p = l->data;
        free(p->particles);
        FE_DestroyResource(p->texture->path);
        free(p);
    }

    FE_List_Destroy(&ParticleSystems);
}
