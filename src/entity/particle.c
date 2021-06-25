#include "../game.h"
#include <math.h>
#include <time.h>

struct Particle *ParticleList;
static SDL_Rect TmpRect;
static int ParticleSystems;

struct Particle {
    SDL_Rect area;
    SDL_Rect *particles;
    int moveamount;
    int death; // point at which particles despawn /respawn (y position)
    int num;
    SDL_Texture *text;
    bool respawn; // Do we want the particle to respawn forever (until manually killed) or despawn?
};


void NewParticleSystem(enum ParticleTypes t, int x, int y) // used to generate particles from enum
{
    SDL_Rect r;
    r.x = x;
    r.y = y;

    switch (t) {
        case PARTICLE_FIRE:
            break;
        case PARTICLE_SNOW:
            r.x = 0;
	        r.y = -screen_height;
	        r.w = map_width;
	        r.h = screen_height;
            GenerateParticles(map_width, 1, 5, r, "../game/snow.png", 5, screen_height, true);
            break;
        default:
            warn("The ParticleType has not been programmed in the initialiser (particle.c)");
            break;
    }
}

void GenerateParticles(int num, int maxsize, int minsize, SDL_Rect area, char *texture, int speed, int death, bool respawn) // Creates each particle rect and add to list arr
{
    if (ParticleSystems == 0)
        ParticleList = malloc(sizeof(struct Particle)); // malloc the struct if its empty
    ParticleList = realloc(ParticleList, sizeof(struct Particle) * (ParticleSystems + 1));

    struct Particle p;

    p.text = TextureManager(texture,renderer);
    p.particles = malloc(sizeof(SDL_Rect) * num);
    p.num = 0;
    p.area = area;
    p.moveamount = speed;
    p.death = death;
    p.respawn = respawn;

    // Init rand
    time_t t;
    srand((unsigned) time(&t)); 

    int minimumx = area.x;
    int maximumx = area.x + area.w;
    int minimumy = area.y;
    int maximumy = area.y - area.h;

    for (int i = 0; i < num; i++) { // Create lots of rects for each particle
        SDL_Rect tmp;
        // we want random size and location (within spawn area)
        tmp.h = (rand() % (maxsize - minsize + 1)) + minsize;
        tmp.w = (rand() % (maxsize - minsize + 1)) + minsize;
        tmp.x = (rand() % (maximumx - minimumx + 1)) + minimumx;
        tmp.y = (rand() % (maximumy - minimumy + 1)) + minimumy;

        p.particles[p.num] = tmp;
        p.num++;
    }
    ParticleList[ParticleSystems++] = p;
    
}

void DestroyParticle(struct Particle *p, int pos)
{
    if (p->num == 1) {
        p->num--;
        p->particles = realloc(p->particles, sizeof(SDL_Rect) * p->num);
        SDL_DestroyTexture(p->text);
        free(p->particles);
        ParticleSystems--;
        ParticleList = realloc(ParticleList, 0);
    }
 
    for (int i = 0; i < p->num; i++) {
        if (i == pos)
            for (int a = i; a < p->num -1; a++) 
                p->particles[a] = p->particles[a+1];     
    }
    
    if (p->num > 1) {
        p->num--;
        p->particles = realloc(p->particles, sizeof(SDL_Rect) * p->num);
    }
}

void ParticleBehaviour(struct Particle *p, SDL_Rect *r, int pos)
{
    r->y += p->moveamount;

    if (r->y > p->death + hscrollam) {
        if (p->respawn) { // if the particle is respawnable, move it to new random spawn
            int maximumx = p->area.x + p->area.w;
            int maximumy = p->area.y - p->area.h;
            r->x = (rand() % (maximumx - p->area.x + 1)) + p->area.x;
            r->y = (rand() % (maximumy - p->area.y + 1)) + p->area.y;
        } else
            DestroyParticle(p, pos);
        
   }

}

void RenderParticles()
{
    if (ParticleSystems > 0) {
        for (int i = 0; i < ParticleSystems; i++) { // loop through each system
            struct Particle *p = &ParticleList[i];
            for (int i = 0; i < p->num; i++) { // through each particle in each system
                TmpRect = p->particles[i];
                TmpRect.x -= scrollam;
                TmpRect.y -= hscrollam;

                if (TmpRect.x <= screen_width && TmpRect.x + TmpRect.w >= 0) 
                    SDL_RenderCopy(renderer, p->text, NULL, &TmpRect);

                ParticleBehaviour(p, &p->particles[i], i);
            }
        }
    } 
}
