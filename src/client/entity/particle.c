#include "../core/include/include.h"
#include "include/particle.h"
#include "include/gameobject.h"
#include "../../common/physics/include/physics.h"
#include "../../common/world/include/collision.h"

#define AssetPath "game/sprites/particles/"

static const double PARTICLE_GRAVITY = 0.1;
static const double PARTICLE_DRAG = 0.5;
static const double MAX_VELOCITY = 7;
static const double PRODUCT = PARTICLE_DRAG * PARTICLE_GRAVITY;

static FE_List *ParticleSystems = 0;

static void GenerateParticle(FE_ParticleSystem *p, size_t index)
{
    if (!p) {
        warn("GenerateParticle() called with NULL particle system");
        return;
    }

    if (p->particles[index].spawned) {
        // If the particle has already had values set, we can just re-use it
        FE_Particle *particle = &p->particles[index];

        particle->body.x = particle->original_position.x;
        particle->body.y = particle->original_position.y;

        particle->position = particle->original_position;
        particle->velocity = particle->original_velocity;
        
        if (p->particle_life > 0) {
            particle->expires = true;
            particle->creation_time = FE_GetTicks64();
        } else {
            particle->expires = false;
        }
    } else {
        FE_Particle particle;

        // Generating particles for the first time
        particle.body.x = p->emission_area.x + s_Rand(p->emission_area.w);
        particle.body.y = p->emission_area.y + s_Rand(p->emission_area.h);

        particle.position.x = particle.body.x;
        particle.position.y = particle.body.y;

        particle.original_position = particle.position;

        particle.body.w = s_Rand((int)p->max_size.x);
        particle.body.h = s_Rand((int)p->max_size.y);
        particle.rotation = 0;

        // Generate random velocity
        particle.velocity.x = -s_Rand((int)p->initial_velocity.x) + s_Rand((int)p->initial_velocity.x); // dispersion either side
        particle.velocity.y = p->initial_velocity.y;

        particle.original_velocity = particle.velocity;

        if (p->particle_life > 0) {
            particle.expires = true;
            particle.creation_time = FE_GetTicks64();
        } else {
            particle.expires = false;
        }

        p->particles[index] = particle;
    }

    p->particles[index].spawned = true;
    p->particles[index].is_dead = false;
}

FE_ParticleSystem *FE_ParticleSystem_Create(SDL_Rect emissionarea, uint16_t emission_rate, uint32_t max_particles, uint32_t particle_life, bool respawns, char *texture, vec2 max_size, vec2 initial_velocity, bool camera_locked)
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
    p->has_splash = false;

    char *path = mstradd(AssetPath, texture);
    p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    free(path);

    p->particles = xmalloc(sizeof(FE_Particle) * max_particles);
    for (size_t i = 0; i < max_particles; i++) {
        // if the particles don't respawn, we want to emit them all at once (e.g explosion)
        p->particles[i].is_dead = true;
        p->particles[i].spawned = false;

        if (p->respawns) {
            p->num_particles = 0;
        } else {
            GenerateParticle(p, i);
            p->num_particles++;
        }
    }
    
    FE_List_Add(&ParticleSystems, p);

    return p;
}

static void Particle_Splash(vec2 position)
{
    FE_ParticleSystem_Create(
        (SDL_Rect){position.x, position.y - 3, 5, 1},
        10,
        10,
        1000,
        false,
        "splash.png",
        (vec2){10, 10},
        (vec2){-3, -2},
        false
    );
}

static void Particle_CheckCollision(FE_Particle *particle, FE_ParticleSystem *p)
{
    /* Checks collision with ground and gameobjects with mass */
    Phys_AABB aabb = {
        .min = (vec2){particle->body.x, particle->body.y},
        .max = (vec2){particle->body.x + particle->body.w, particle->body.y + particle->body.h}
    };

    /* Check collisions with objects */
    int result = FE_GameObject_CollisionAbove(&particle->body);

    /* If we didn't collide with object, check map for collisions */
    if (result == -1)
        result = FE_Map_CollisionAbove(&aabb);

    if (result != -1) {
        particle->is_dead = true;
        p->num_particles--;

        /* If there are multiple particles, have a 10% chance of splashing */
        if (p->has_splash) {
            if (p->emission_rate > 100) {
                if (s_Rand(100) < 5)
                    Particle_Splash((vec2){particle->body.x, result});
            }
        }
    }
}

void FE_Particles_Update(FE_Camera *camera)
{
    if (!camera) {
        warn("FE_Particles_Update() called with NULL camera");
        return;
    }

    if (!ParticleSystems)
        return;

    float CurrentDrag = (1.0f - (PARTICLE_DRAG * FE_DT));

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

        uint64_t time = FE_GetTicks64();

        // check each particle inside the system
        for (size_t i = 0; i < p->max_particles; i++) {
            FE_Particle *particle = &p->particles[i];    
            if (particle->is_dead) { // if we are already due to emit a new particle, use this one
                if (to_emit > 0) {
                    GenerateParticle(p, i);
                    to_emit--;
                    p->num_particles++;
                }
                continue;
            }

            // update particle timer
            if (particle->expires) {
                Uint32 current_time = time - particle->creation_time;
                if (current_time > p->particle_life) { // if particle has expired
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
            float new_velocity_y = particle->velocity.y += PRODUCT;
            particle->velocity.y = clamp(new_velocity_y, -MAX_VELOCITY, MAX_VELOCITY);
            particle->velocity.x *= CurrentDrag;

            // separate position as floating point to hold deltatime small values
            particle->position.y += (particle->velocity.y * FE_DT_MULTIPLIER);
            particle->position.x += (particle->velocity.x * FE_DT_MULTIPLIER);

            FE_UPDATE_RECT(particle->position, &particle->body);

            // kill particle if it's out of map bounds
            if (particle->body.x < 0 || particle->body.x > PresentGame->MapConfig.MapWidth ||  particle->body.y > (PresentGame->MapConfig.MapHeight - particle->body.h)) {
                particle->is_dead = true;
                p->num_particles--;
                continue;
            }

            // If particle is out of render bounds, respawn it after map height
            if (particle->position.x > camera->x + PresentGame->WindowWidth || particle->position.x < camera->x) {
                continue;
            }

            // Check collisions with map and gameobjects
            if (!particle->is_dead)
                Particle_CheckCollision(particle, p);
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

            FE_RenderCopyEx(camera, p->camera_locked, p->texture, NULL, &p->particles[i].body, p->particles[i].rotation, SDL_FLIP_NONE);
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
