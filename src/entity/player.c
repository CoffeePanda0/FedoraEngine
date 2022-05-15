#include "../core/include/include.h"
#include "include/animation.h"
#include "include/player.h"
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

    // physics object
    p->PhysObj = FE_CreatePhysObj(PLAYER_MASS, maxspeed, body, true);

    // light
    SDL_Rect l_r = {body.x, body.y, 256, 256};
    p->Light = FE_CreateLight(l_r, "light.png");

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
    SDL_RenderCopyEx(PresentGame->renderer,
        current_animation->spritesheet->Texture,
        &player_srcrct,
        &(SDL_Rect){player->render_rect.x * camera->zoom, player->render_rect.y * camera->zoom, player->render_rect.w * camera->zoom, player->render_rect.h * camera->zoom},
        0, &center, flip
    );

}

void FE_MovePlayer(FE_Player *player, Vector2D movement)
{
    if (!player)
        return;

    Vector2D mov_vec = FE_NewVector(movement.x * FE_DT_MULTIPLIER, movement.y * FE_DT_MULTIPLIER);
    FE_ApplyForce(player->PhysObj, mov_vec);
}


static bool OnGround(FE_Player *player)
{
    // keep track of last y as there is no point checking if player hasnt moved
    static int last_y;

    if (last_y != player->PhysObj->body.y) {
        last_y = player->PhysObj->body.y;

        if (player->PhysObj->velocity.y != 0) // if velocity is 0, player will never be on ground
            return false;
        
        Vector2D GroundCollision = FE_CheckMapCollisionAbove(&player->PhysObj->body);
        if (FE_VecNULL(GroundCollision))
            return false;
        else
            return true;
    }

    return player->on_ground;
}

void FE_UpdatePlayer(FE_Player *player)
{
    player->render_rect = player->PhysObj->body;
    player->on_ground = OnGround(player);

    // Update the light position, keeping player centered
    if (player->Light) {
        player->Light->Rect.x = player->PhysObj->body.x + player->PhysObj->body.w/2 - player->Light->Rect.w/2;
        player->Light->Rect.y = player->PhysObj->body.y + player->PhysObj->body.h/2 - player->Light->Rect.h/2;
    }
    
    if (player->PhysObj->velocity.x < 0.15 && player->PhysObj->velocity.x > -0.15) // don't animate small amounts
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
        FE_ApplyForce(player->PhysObj, FE_NewVector(0, -min_jump));
        return;
    }

    FE_ApplyForce(player->PhysObj, FE_NewVector(0, -3 * FE_DT_MULTIPLIER));

    if (player->PhysObj->velocity.y <= -player->jumpforce || player->jump_elapsed > 0.15) {
        player->PhysObj->velocity.y = -player->jumpforce;
        player->jump_started = false;
        player->jump_elapsed = 0;
    }
    
}