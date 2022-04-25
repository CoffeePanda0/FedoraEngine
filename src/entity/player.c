#include "../include/game.h"

#define IDLE_ANIMATION "player_idle.png"
#define WALK_ANIMATION "player_walk.png"
#define JUMP_ANIMATION "player_jump.png"

const int PLAYER_MASS = 70;

static size_t PlayerCount = 0;

FE_Player *FE_CreatePlayer(float movespeed, float maxspeed, float jumpforce, SDL_Rect body)
{
    // basic player vars
    FE_Player *p = xmalloc(sizeof(FE_Player));
    p->id = PlayerCount++;
    p->movespeed = movespeed;
    p->jumpforce = jumpforce;

    // physics object
    p->PhysObj = xmalloc(sizeof(FE_PhysObj));
    p->PhysObj->body = body;
    p->PhysObj->velocity = VEC_EMPTY;
    p->PhysObj->maxvelocity = (Vector2D){maxspeed, 0};
    p->PhysObj->mass = PLAYER_MASS;

    // bool values
    p->on_ground = false;
    p->moving = false;
    p->facing_right = true;

    // load animations
    p->idle_animation = FE_CreateAnimation(IDLE_ANIMATION, 2, 32, 64, 1000, true);
    p->walk_animation = FE_CreateAnimation(WALK_ANIMATION, 1, 32, 64, 1000, false);
    p->jump_animation = FE_CreateAnimation(JUMP_ANIMATION, 1, 32, 64, 1000, false);

    p->render_rect = p->PhysObj->body;

    // add to physics world
    FE_AddPhysInteractable(p->PhysObj);

    return p;
}

void FE_RenderPlayer(FE_Player *player)
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
    else if (!player->on_ground)
        current_animation = player->jump_animation;
    else
        current_animation = player->idle_animation;

    // render the player
    SDL_Rect player_srcrct = FE_GetAnimationFrame(current_animation);

    FE_RenderCopyEx(current_animation->spritesheet, &player_srcrct, &player->render_rect, 0, flip);
}

static void PlayerCameraFollow(FE_Player *player, FE_Camera *camera)
{
    if (!player || !camera)
        return;

    // only move the camera if past half of the screen
    if (player->PhysObj->velocity.x > 0) {
        if (player->render_rect.x > screen_width / 2 && camera->x < camera->x_bound) {
            FE_MoveCamera(player->PhysObj->velocity.x, 0, camera);
        } else {
            if (player->render_rect.x + player->render_rect.w + player->PhysObj->velocity.x >= screen_width)
                player->render_rect.x = screen_width - player->render_rect.w;
            else
                player->render_rect.x += player->PhysObj->velocity.x;
        }
    } else if (player->PhysObj->velocity.x < 0) {
        if (player->render_rect.x < screen_width / 2 && camera->x > camera->x_min) {
            FE_MoveCamera(player->PhysObj->velocity.x, 0, camera);
        } else {
            if (player->render_rect.x + player->PhysObj->velocity.x <= 0)
                player->render_rect.x = 0;
            else
                player->render_rect.x += player->PhysObj->velocity.x;
        }
    }

    player->render_rect.y = (screen_height / 2);
    camera->y = -player->render_rect.y + player->PhysObj->body.y;
}  

void FE_SetPlayerWorldPos(FE_Player *player, FE_Camera *camera, Vector2D position)
{
    if (!player)
        return;
    
    player->PhysObj->body.x = position.x;
    player->PhysObj->body.y = position.y;

    // calculate position relative on screen for render rect
    player->render_rect.x = player->PhysObj->body.x - camera->x;
    player->render_rect.y = player->render_rect.h - (screen_height / 2); 

    // centre camera on player
    camera->y = clamp(player->PhysObj->body.y - (screen_height / 2), 0, camera->y_bound);
    camera->x = clamp(player->PhysObj->body.x - (screen_width / 2), camera->x_min, camera->x_bound);
}

void FE_MovePlayer(FE_Player *player, FE_Camera *camera, Vector2D movement)
{
    if (!player || !camera)
        return;

    FE_ApplyForce(player->PhysObj, movement);
    PlayerCameraFollow(player, camera);
}

void FE_PlayerJump(size_t jump_duration, FE_Player *player, FE_Camera *camera)
{
    // make jump force depend on how long the jump key was pressed for
    float duration = jump_duration;
    if (duration > 20) duration = 20;

    float max_force = -player->jumpforce;
    float min_force = -player->jumpforce / 1.5;

    float jump_force = (duration / (max_force - min_force)) * 10;
    if (jump_force < max_force) jump_force = max_force;
    if (jump_force > min_force) jump_force = min_force;

    if (player->on_ground)
        FE_MovePlayer(player, camera, FE_NewVector(0, jump_force));

}

void FE_UpdatePlayer(FE_Player *player, FE_Camera *camera)
{
    player->render_rect =(SDL_Rect){player->PhysObj->body.x, screen_height / 2, player->PhysObj->body.w, player->PhysObj->body.h};
    player->render_rect.x -= camera->x;
    PlayerCameraFollow(player, camera);
    
    // update bool values
    if (player->PhysObj->velocity.y == 0)
        player->on_ground = true;
    else
        player->on_ground = false;
    
    if (player->PhysObj->velocity.x == 0)
        player->moving = false;
    else {
        player->moving = true;
        if (player->PhysObj->velocity.x > 0)
            player->facing_right = true;
        else
            player->facing_right = false;
    }
}
