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

FE_Player *FE_CreatePlayer(float movespeed, float maxspeed, float jumpforce, SDL_Rect body)
{
    // basic player vars
    FE_Player *p = xmalloc(sizeof(FE_Player));
    p->id = PlayerCount++;
    p->movespeed = movespeed;
    p->jumpforce = jumpforce;
    p->jump_elapsed = 0;
    p->last_y_check = 0;

    // physics object
    p->PhysObj = FE_CreatePhysObj(PLAYER_MASS, body);

    // light
    p->Light = FE_Light_Create(body, 128, "light.png");

    // bool values
    p->on_ground = false;
    p->moving = false;
    p->facing_right = true;
    p->jump_started = false;

    // load animations
    p->idle_animation = FE_CreateAnimation(IDLE_ANIMATION, 2, 30, 23, 250, true);
    p->walk_animation = FE_CreateAnimation(WALK_ANIMATION, 3, 30, 23, 100, true);
    p->jump_animation = FE_CreateAnimation(JUMP_ANIMATION, 4, 30, 23, 50, false);
    p->jump_animation->current_frame = 3;

    p->render_rect = p->PhysObj->body;

    // add to physics world
    FE_AddPhysInteractable(p->PhysObj);

    return p;
}

void FE_DestroyPlayer(FE_Player *player)
{
    if (!player) {
        warn("Tried to destroy NULL player");
        return;
    }
    
    FE_RemovePhysInteractable(player->PhysObj);
    FE_Light_Destroy(player->Light);
    FE_DestroyAnimation(player->idle_animation);
    FE_DestroyAnimation(player->walk_animation);
    FE_DestroyAnimation(player->jump_animation);
    free(player);
}

void FE_RenderPlayer(FE_Player *player, FE_Camera *camera)
{
    if (!player || PlayerCount == 0)
        return;

    // direction that player is facing
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (!player->facing_right)
        flip = SDL_FLIP_HORIZONTAL;
    
    FE_Animation *current_animation = 0;

    if (player->moving)
        current_animation = player->walk_animation;
    if (!player->on_ground)
        current_animation = player->jump_animation;
    if (!player->moving && player->on_ground)
        current_animation = player->idle_animation;

    // render the player

    SDL_Rect player_srcrct = FE_GetAnimationFrame(current_animation);
    const SDL_Point center = (SDL_Point){player->render_rect.w/2, player->render_rect.h/2};
    SDL_RenderCopyEx(PresentGame->Renderer,
        current_animation->spritesheet->Texture,
        &player_srcrct,
        &(SDL_Rect){player->render_rect.x * camera->zoom, player->render_rect.y * camera->zoom, player->render_rect.w * camera->zoom, player->render_rect.h * camera->zoom},
        0, &center, flip
    );

}

void FE_MovePlayer(FE_Player *player, vec2 movement)
{
    if (!player)
        return;

    vec2 mov_vec = vec(movement.x * FE_DT_MULTIPLIER, movement.y * FE_DT_MULTIPLIER);

    int lim = 10;
    if (!(player->PhysObj->velocity.x > lim) && player->PhysObj->velocity.x > -lim)
        FE_ApplyForce(player->PhysObj, mov_vec);
}


bool FE_PlayerOnGround(FE_Player *player)
{
    // keep track of last y as there is no point checking if player hasnt moved

    if (player->last_y_check != player->PhysObj->body.y) {
        player->last_y_check = player->PhysObj->body.y;

        if (player->PhysObj->velocity.y != 0) // if velocity is not 0, player will never be on ground
            return false;
        
        vec2 GroundCollision = FE_CheckMapCollisionAbove(&player->PhysObj->body);
        if (vec2_null(GroundCollision))
            return false;
        else
            return true;
    }

    return player->on_ground;
}

void FE_UpdatePlayer(FE_Player *player)
{
    FE_Trigger_Check(&player->PhysObj->body);

    player->render_rect = player->PhysObj->body;
    player->on_ground = FE_PlayerOnGround(player);

    static vec2 last_position = VEC_NULL;

    // Update the light position, keeping player centered
    if (last_position.x != player->PhysObj->body.x || last_position.y != player->PhysObj->body.y) {
        if (player->Light) {
            FE_Light_Move(player->Light, player->PhysObj->body.x, player->PhysObj->body.y);
            last_position = vec(player->PhysObj->body.x, player->PhysObj->body.y);
        }
    }
    
    if (player->PhysObj->velocity.x < 0.2 && player->PhysObj->velocity.x > -0.2) // don't animate small amounts
        player->moving = false;
    else {
        player->moving = true;
        if (player->PhysObj->velocity.x > 0)
            player->facing_right = true;
        else
            player->facing_right = false;
    }
}

void FE_StartPlayerJump(FE_Player *player)
{
    if (!player || player->jump_started || !player->on_ground)
        return;

    player->jump_started = true;
    player->jump_elapsed = 0;
}

void FE_UpdatePlayerJump(FE_Player *player)
{
    if (!player || !player->jump_started)
        return;

    float min_jump = player->jumpforce / 4;
    player->jump_elapsed += FE_DT;

    if (player->PhysObj->velocity.y == 0) {
        FE_ApplyForce(player->PhysObj, vec(0, -min_jump * player->PhysObj->mass));
        return;
    }

    FE_ApplyForce(player->PhysObj, vec(0, -3 * FE_DT_MULTIPLIER * player->PhysObj->mass));

    if (player->PhysObj->velocity.y <= -player->jumpforce || player->jump_elapsed > 0.15) {
        player->PhysObj->velocity.y = -player->jumpforce;
        player->jump_started = false;
        player->jump_elapsed = 0;
    }
    
}