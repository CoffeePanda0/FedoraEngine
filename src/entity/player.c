#include "../include/game.h"

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
    if (!player->on_ground)
        current_animation = player->jump_animation;
    if (!player->moving && player->on_ground)
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
        if (player->render_rect.x + player->render_rect.w > screen_width / 2 && camera->x < camera->x_bound) {
            FE_MoveCamera(player->PhysObj->velocity.x, 0, camera);
        } else {
            if (player->render_rect.x + player->render_rect.w + player->PhysObj->velocity.x >= screen_width)
                player->render_rect.x = screen_width - player->render_rect.w;
            else
                player->render_rect.x += player->PhysObj->velocity.x;
        }
    } else if (player->PhysObj->velocity.x < 0) {
        if (player->render_rect.x  + player->render_rect.w < screen_width / 2 && camera->x > camera->x_min) {
            FE_MoveCamera(player->PhysObj->velocity.x, 0, camera);
        } else {
            if (player->render_rect.x + player->PhysObj->velocity.x <= 0)
                player->render_rect.x = 0;
            else
                player->render_rect.x += player->PhysObj->velocity.x;
        }
    }

}  

void FE_SetPlayerWorldPos(FE_Player *player, FE_Camera *camera, Vector2D position)
{
    if (!player)
        return;
    
    player->PhysObj->position.x = position.x;
    player->PhysObj->position.y = position.y;

    // calculate position relative on screen for render rect
    player->render_rect.x = player->PhysObj->body.x + player->render_rect.w - camera->x;
    player->render_rect.y = player->render_rect.h - (screen_height / 2); 

    // centre camera on player
    camera->x = clamp(player->PhysObj->body.x  - (screen_width / 2), camera->x_min, camera->x_bound);

    //set camera y so that map height is always the same
    camera->y = FE_Map_Height - screen_height;
}

void FE_MovePlayer(FE_Player *player, FE_Camera *camera, Vector2D movement)
{
    if (!player || !camera)
        return;

    Vector2D mov_vec = FE_NewVector(movement.x * FE_DT_MULTIPLIER, movement.y * FE_DT_MULTIPLIER);
    FE_ApplyForce(player->PhysObj, mov_vec);
    PlayerCameraFollow(player, camera);

    static float fps_timer = 0;
    fps_timer += FE_DT;

}

void FE_UpdatePlayer(FE_Player *player, FE_Camera *camera)
{
    player->render_rect = (SDL_Rect){player->PhysObj->body.x, player->PhysObj->body.y , player->PhysObj->body.w, player->PhysObj->body.h};
    player->render_rect.x -= camera->x;
    player->render_rect.y -= camera->y;

    PlayerCameraFollow(player, camera);
    
    // update bool values
    if (player->PhysObj->velocity.y == 0)
        player->on_ground = true;
    else
        player->on_ground = false;
    
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

    if (player->jump_elapsed >= (5 / FE_DT_MULTIPLIER)) {
        player->jump_started = false;
        player->jump_elapsed = 0;
    } else {
        player->jump_elapsed++;
        FE_ApplyForce(player->PhysObj, FE_NewVector(0, -player->jumpforce * FE_DT_MULTIPLIER));
    }
    
}