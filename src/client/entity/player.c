#include "../core/include/include.h"
#include "include/animation.h"
#include "include/player.h"
#include "include/trigger.h"
#include "../world/include/map.h"

#define IDLE_ANIMATION "player_idle.png"
#define WALK_ANIMATION "player_walk.png"
#define JUMP_ANIMATION "player_jump.png"

static const int PLAYER_MASS = 50;

static size_t PlayerCount = 0;

FE_Player *FE_Player_Create(float acceleration, float maxspeed, float jumpforce, SDL_Rect body)
{
    // basic player properties
    FE_Player *p = xmalloc(sizeof(FE_Player));
    p->player = xmalloc(sizeof(FE_PlayerProperties));

    p->player->id = PlayerCount++;
    p->player->moveforce = (acceleration * PLAYER_MASS / PHYS_SCALE);
    p->player->maxspeed = maxspeed;
    p->player->jumpforce = jumpforce;
    p->player->jump_elapsed = 0;
    p->player->jump_started = false;

    p->last_y_check = 0;

    // physics object
    p->player->PhysObj = FE_Physics_CreateBody(PLAYER_MASS, body);

    // light
    p->Light = FE_Light_Create(body, 128, "light.png");

    // bool values
    p->on_ground = false;
    p->moving = false;
    p->facing_right = true;

    // load animations
    p->idle_animation = FE_Animation_Create(IDLE_ANIMATION, 2, 30, 23, 250, true);
    p->walk_animation = FE_Animation_Create(WALK_ANIMATION, 3, 30, 23, 100, true);
    p->jump_animation = FE_Animation_Create(JUMP_ANIMATION, 4, 30, 23, 50, false);
    p->jump_animation->current_frame = 3;

    p->render_rect = p->player->PhysObj->body;

    // add to physics world
    FE_Physics_AddBody(p->player->PhysObj);

    return p;
}

void FE_Player_Destroy(FE_Player *player)
{
    if (!player) {
        warn("Tried to destroy NULL player");
        return;
    }
    
    FE_Physics_Remove(player->player->PhysObj);
    FE_Light_Destroy(player->Light);
    FE_Animation_Destroy(player->idle_animation);
    FE_Animation_Destroy(player->walk_animation);
    FE_Animation_Destroy(player->jump_animation);
    free(player->player);
    free(player);
}

void FE_Player_Render(FE_Player *player, FE_Camera *camera)
{
    if (!player || PlayerCount == 0)
        return;

    // direction that player is facing
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (!player->facing_right)
        flip = SDL_FLIP_HORIZONTAL;
    
    FE_Animation *current_animation = player->idle_animation;

    if (player->moving)
        current_animation = player->walk_animation;
    if (!player->player->PhysObj->grounded)
        current_animation = player->jump_animation;
    if (!player->moving && player->on_ground)
        current_animation = player->idle_animation;

    // render the player

    SDL_Rect player_srcrct = FE_Animation_GetFrame(current_animation);
    const SDL_Point center = (SDL_Point){player->render_rect.w/2, player->render_rect.h/2};
    SDL_RenderCopyEx(PresentGame->Client->Renderer,
        current_animation->spritesheet->Texture,
        &player_srcrct,
        &(SDL_Rect){player->render_rect.x * camera->zoom, player->render_rect.y * camera->zoom, player->render_rect.w * camera->zoom, player->render_rect.h * camera->zoom},
        0, &center, flip
    );

}

void FE_Player_Update(FE_Player *player)
{
    FE_DebugUI_Update(player);
    FE_Trigger_Check(&player->player->PhysObj->body);

    player->render_rect = player->player->PhysObj->body;
    player->on_ground = player->player->PhysObj->grounded;

    static vec2 last_position = {-1, -1};

    // Update the light position, keeping player centered
    if (last_position.x != player->player->PhysObj->body.x || last_position.y != player->player->PhysObj->body.y) {
        if (player->Light) {
            FE_Light_Move(player->Light, player->player->PhysObj->body.x, player->player->PhysObj->body.y);
            last_position = vec(player->player->PhysObj->body.x, player->player->PhysObj->body.y);
        }
    }
    
    if (player->player->PhysObj->velocity.x < 0.2 && player->player->PhysObj->velocity.x > -0.2) // don't animate small amounts
        player->moving = false;
    else {
        player->moving = true;
        if (player->player->PhysObj->velocity.x > 0)
            player->facing_right = true;
        else
            player->facing_right = false;
    }
}
