#include <FE_Common.h>
#include "include/player.h"
#include "../../common/physics/include/physics.h"

void FE_Player_Move(FE_PlayerProperties *player, vec2 movement)
{
    if (!player)
        return;
    
    movement.x *= player->moveforce;

    vec2 mov_vec = vec(movement.x * FE_DT_MULTIPLIER, movement.y * FE_DT_MULTIPLIER);

    vec2 min_speed = vec(-player->maxspeed, -9999);

    if (!player->PhysObj->grounded) {
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

void FE_Player_StartJump(FE_PlayerProperties *player)
{
    if (!player || player->jump_started || !player->PhysObj->grounded)
        return;

    player->jump_started = true;
    player->jump_elapsed = 0;
}

void FE_Player_UpdateJump(FE_PlayerProperties *player)
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