#include "../include/game.h"

#define AssetPath "game/sprites/particles/"

static const double PARTICLE_GRAVITY = 0.1;
static const double PARTICLE_DRAG = 0.1;
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

FE_ParticleSystem *FE_CreateParticleSystem(SDL_Rect emissionarea, Uint16 emission_rate, Uint16 max_particles, Uint16 particle_life, bool respawns, char *texture, Vector2D max_size, Vector2D initial_velocity, bool camera_locked)
{
    FE_ParticleSystem *p = xmalloc(sizeof(FE_ParticleSystem));
    p->emission_area = emissionarea;
    p->max_particles = max_particles;
    p->emission_rate = emission_rate; 
    p->respawns = respawns;
    p->max_size = max_size;
    p->initial_velocity = initial_velocity;
    p->num_particles = 0;
    p->particle_life = particle_life;
    
    p->emission_rate_timer = 0;
    p->camera_locked = camera_locked;

    char *path = AddStr(AssetPath, texture);
    p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    xfree(path);

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

void FE_UpdateParticles() // TODO - Optimise this
{
    if (!ParticleSystems)
        return;

    for (FE_List *ps = ParticleSystems; ps; ps = ps->next) { // loop through each particle system
        FE_ParticleSystem *p = ps->data;

        bool emit = false;
        if (p->respawns) {
            float rate = (1.0f / p->emission_rate);
            p->emission_rate_timer += FE_DT;
            if (p->emission_rate_timer > rate) {
                if (p->num_particles < p->max_particles)
                    emit = true;
                p->emission_rate_timer = 0;
            }
        }

        for (size_t i = 0; i < p->max_particles; i++) {
            FE_Particle *particle = &p->particles[i];    
            
            if (particle->is_dead) { // if we are already due to emit a new particle, use this one
                if (emit) {
                    GenerateParticle(p, i);
                    emit = false;
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
                            FE_DestroyParticleSystem(p);
                            return;
                        }
                    }
                    continue;
                }
            }

            // apply gravity and friction to all particles
            float new_velocity_y = particle->velocity.y += PARTICLE_GRAVITY;
            particle->velocity.y = clampf(new_velocity_y, -MAX_VELOCITY, MAX_VELOCITY);
            particle->velocity.x *= (1 - PARTICLE_DRAG);

            // separate position as floating point to hold deltatime small values
            particle->position.y += (particle->velocity.y * FE_DT_MULTIPLIER);
            particle->position.x += (particle->velocity.x * FE_DT_MULTIPLIER);

            FE_DT_RECT(particle->position, &particle->body);

            // kill particle if it's out of map bounds
            if (particle->body.x < 0 || particle->body.x > FE_Map_Width ||  particle->body.y > (FE_Map_Height - particle->body.h)) {
                particle->is_dead = true;
                p->num_particles--;
            }
            
        }
    }
    
}

void FE_RenderParticles(FE_Camera *camera)
{
    for (FE_List *l = ParticleSystems; l; l = l->next) {
        FE_ParticleSystem *p = l->data;
        for (size_t i = 0; i < p->max_particles; i++) {
            if (p->particles[i].is_dead)
                continue;

            if (p->camera_locked) {
                FE_RenderCopyEx(p->particles[i].texture, NULL, &p->particles[i].body, p->particles[i].rotation, SDL_FLIP_NONE);
            } else {
                SDL_Rect render_rect = (SDL_Rect){p->particles[i].body.x - camera->x, p->particles[i].body.y - camera->y, p->particles[i].body.w, p->particles[i].body.h};
                FE_RenderCopyEx(p->particles[i].texture, NULL, &render_rect, p->particles[i].rotation, SDL_FLIP_NONE);
            }

        }
    }
}

bool FE_DestroyParticleSystem(FE_ParticleSystem *p)
{
    if (!p) {
        warn("FE_DestroyParticleSystem() called with NULL particle system");
        return false;
    }

    xfree(p->particles);
    FE_DestroyResource(p->texture->path);

    FE_List_Remove(&ParticleSystems, p);
    
    free(p);
    return true;
}

void FE_CleanParticles()
{
    if (!ParticleSystems)
        return;

    for (FE_List *l = ParticleSystems; l; l = l->next) {
        FE_ParticleSystem *p = l->data;
        xfree(p->particles);
        FE_DestroyResource(p->texture->path);
        xfree(p);
    }

    FE_List_Destroy(&ParticleSystems);
}
