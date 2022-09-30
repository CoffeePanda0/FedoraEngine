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

FE_Player *FE_Player_Create(float acceleration, float maxspeed, float jumpforce, GPU_Rect body)
{
    // basic player vars
    FE_Player *p = xmalloc(sizeof(FE_Player));
    p->id = PlayerCount++;
    p->moveforce = (acceleration * PLAYER_MASS / PHYS_SCALE);
    p->maxspeed = maxspeed;
    p->jumpforce = jumpforce;
    p->jump_elapsed = 0;
    p->last_y_check = 0;

    // physics object
    p->PhysObj = FE_Physics_CreateBody(PLAYER_MASS, body);

    // light
    p->Light = FE_Light_Create(body, 128, "light.png");

    // bool values
    p->on_ground = false;
    p->moving = false;
    p->facing_right = true;
    p->jump_started = false;

    // load animations
    p->idle_animation = FE_Animation_Create(IDLE_ANIMATION, 2, 30, 23, 250, true);
    p->walk_animation = FE_Animation_Create(WALK_ANIMATION, 3, 30, 23, 100, true);
    p->jump_animation = FE_Animation_Create(JUMP_ANIMATION, 4, 30, 23, 50, false);
    p->jump_animation->current_frame = 3;

    p->render_rect = p->PhysObj->body;

    // add to physics world
    FE_Physics_AddBody(p->PhysObj);

    return p;
}

void FE_Player_Destroy(FE_Player *player)
{
    if (!player) {
        warn("Tried to destroy NULL player");
        return;
    }
    
    FE_Physics_Remove(player->PhysObj);
    FE_Light_Destroy(player->Light);
    FE_Animation_Destroy(player->idle_animation);
    FE_Animation_Destroy(player->walk_animation);
    FE_Animation_Destroy(player->jump_animation);
    free(player);
}

void FE_Player_Render(FE_Player *player, FE_Camera *camera)
{
    if (!player || PlayerCount == 0)
        return;

    // direction that player is facing
    GPU_FlipEnum flip = GPU_FLIP_NONE;
    if (!player->facing_right)
        flip = GPU_FLIP_HORIZONTAL;
    
    FE_Animation *current_animation = 0;

    if (player->moving)
        current_animation = player->walk_animation;
    if (!player->on_ground)
        current_animation = player->jump_animation;
    if (!player->moving && player->on_ground)
        current_animation = player->idle_animation;

    // render the player

    GPU_Rect player_srcrct = FE_Animation_GetFrame(current_animation);
    const SDL_Point center = (SDL_Point){player->render_rect.w/2, player->render_rect.h/2};
    GPU_BlitRectX(current_animation->spritesheet->Texture,
        &player_srcrct,
        camera->target,
        &(GPU_Rect){player->render_rect.x * camera->zoom, player->render_rect.y * camera->zoom, player->render_rect.w * camera->zoom, player->render_rect.h * camera->zoom},
        0,
        center.x,
        center.y,
        flip
    );

}

void FE_Player_Move(FE_Player *player, vec2 movement)
{
    if (!player)
        return;

    vec2 mov_vec = vec(movement.x * FE_DT_MULTIPLIER, movement.y * FE_DT_MULTIPLIER);

    vec2 min_speed = vec(-player->maxspeed, -9999);

    if (!player->on_ground) {
        // Only allow tiny X movements in the air
        float friction_force = player->PhysObj->friction * 0.1;
        if (movement.x < 0) friction_force *= -1;

        mov_vec = vec((mov_vec.x * 0.3) + friction_force, mov_vec.y);
        mov_vec = vec2_scale(mov_vec, FE_DT_MULTIPLIER);
    }

    if (movement.x > 0) {
        // Calculate the maximum force that we can apply to still be under the max speed
        float max_x_force = (player->maxspeed - player->PhysObj->velocity.x) * player->PhysObj->mass;
        vec2 max_force = vec(max_x_force, 9999);
        mov_vec = vec2_clamp(mov_vec, min_speed, max_force);
    } else if (movement.x < 0) {
        // Calculate the maximum force that we can apply to still be under the max speed
        float max_x_force = (-player->maxspeed - player->PhysObj->velocity.x) * player->PhysObj->mass;
        mov_vec = vec2_clamp(mov_vec, vec(max_x_force, -9999), vec(0, 9999));
    }

    // apply the force
    FE_Physics_ApplyForce(player->PhysObj, mov_vec);

}

void FE_Player_Update(FE_Player *player)
{
    FE_Trigger_Check(&player->PhysObj->body);

    player->render_rect = player->PhysObj->body;
    player->on_ground = player->PhysObj->grounded;

    static vec2 last_position = {-1, -1};

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

void FE_Player_StartJump(FE_Player *player)
{
    if (!player || player->jump_started || !player->on_ground)
        return;

    player->jump_started = true;
    player->jump_elapsed = 0;
}

void FE_Player_UpdateJump(FE_Player *player)
{
    if (!player || !player->jump_started)
        return;

    float min_jump = player->jumpforce / 4;
    player->jump_elapsed += FE_DT;

    if (player->PhysObj->velocity.y == 0) {
        FE_Physics_ApplyForce(player->PhysObj, vec(0, -min_jump * player->PhysObj->mass));
        return;
    }

    FE_Physics_ApplyForce(player->PhysObj, vec(0, -3 * FE_DT_MULTIPLIER * player->PhysObj->mass));

    if (player->PhysObj->velocity.y <= -player->jumpforce || player->jump_elapsed > 0.15) {
        player->PhysObj->velocity.y = -player->jumpforce;
        player->jump_started = false;
        player->jump_elapsed = 0;
    }
    
}