#include <stdint.h>
#include <stddef.h>
#include <strarr.h>
#include <utils.h>
#include "include/include.h"

static size_t LeftTileRange(SDL_FRect *r) // calculates the left-most tile near the player to check for collision
{
    /*  check if the rect hasn't moved, if so we can just return the previous value
        Stores the last 5 calculations as we can return for 5 objects if there is no change. This is more
        efficient than searching the list every time when there has been no x movement
    */
    static uint16_t prev_x[5];
    static uint16_t prev_mid[5];
    static uint8_t arr_pt = 0;
    for (size_t i = 0; i < 5; i++) {
        if (r->x == prev_x[i]) {
            return prev_mid[i];
        }
    }
    if (arr_pt == 4)
        arr_pt = 0;
    else
        arr_pt++;

    /* find the tile closest to the player's left side using a binary search */
    uint16_t left = 0;
    uint16_t right = PresentMap->tilecount - 1;
    uint16_t mid = 0;
    while (left <= right && right < PresentMap->tilecount) {
        mid = (left + right) / 2;
        if (PresentMap->tiles[mid].position.x + PresentMap->tilesize == r->x) {
            break;
        } else if (PresentMap->tiles[mid].position.x + PresentMap->tilesize < r->x)
            left = mid + 1;
        else
            right = mid - 1;
    }

    prev_x[arr_pt] = r->x;
    prev_mid[arr_pt] = mid;

    return mid;

}

static size_t RightTileRange(size_t left, SDL_FRect *r) /* calculates the right-most tile near the player to check for collision */
{
    /* check if the rect hasn't moved, if so we can just return the previous value */
    static uint16_t prev_x[5];
    static uint16_t prev_mid[5];
    static uint8_t arr_pt = 0;
    
    for (size_t i = 0; i < 5; i++) {
        if (r->x == prev_x[i]) {
            return prev_mid[i];
        }
    }
    if (arr_pt == 4)
        arr_pt = 0;
    else
        arr_pt++;

    uint16_t r_right = r->x + r->w;
    prev_x[arr_pt] = r_right;

    // find the tile closest to the player's right side
    for (size_t i = left; i < PresentMap->tilecount; i++) {
        if (PresentMap->tiles[i].position.x > r_right)
            return i;
    }
    prev_mid[arr_pt] = PresentMap->tilecount -1;
    return PresentMap->tilecount -1;
}

void FE_Map_Collisions(Phys_AABB *aabb, FE_CollisionInfo *result)
{
    result->count = 0;
    result->collisions = 0;

    SDL_FRect tmp = (SDL_FRect) {aabb->min.x, aabb->min.y, aabb->max.x - aabb->min.x, aabb->max.y - aabb->min.y};
    size_t left = LeftTileRange(&tmp);
    size_t right = RightTileRange(left, &tmp);

    /* Checks every tile in the range for collision with the player */
    for (size_t i = left; i < right; i++) {
        FE_Map_Tile *t = &PresentMap->tiles[i];
        SDL_FRect tile = (SDL_FRect){t->position.x, t->position.y, PresentMap->tilesize, PresentMap->tilesize};
        
        SDL_FRect intersection;
        if (FE_AABB_Intersect(&tmp, &tile, &intersection)) {
            // calculate collision normal
            vec2 normal = {0, 0};

            if (tmp.y + tmp.h < tile.y + tile.h / 2)
                normal.y = 1;
            else if (tmp.y > tile.y + tile.h / 2)
                normal.y = -1;

            if (tmp.x + tmp.w < tile.x + tile.w / 2)
                normal.x = 1;
            else if (tmp.x > tile.x + tile.w / 2)
                normal.x = -1;

            // calculate penetration depth
            float depth = 0;
            depth = intersection.w > intersection.h ? intersection.h : intersection.w;

            result->collisions = xrealloc(result->collisions, sizeof(TileCollision) * (++result->count));
            result->collisions[result->count -1] = (TileCollision) {t->position, normal, depth};
        }
    }
}
