#include "../core/include/include.h"
#include "../physics/include/physics.h"
#include "../physics/include/collision.h"
#include "include/trigger.h"

static FE_List *triggers;
static FE_Texture *default_texture;

FE_Trigger *FE_Trigger_Create(vec2 position, int range, void (*event)(), void *event_data, bool visible)
{
	FE_Trigger *t = xmalloc(sizeof(FE_Trigger));
	t->position = position;
	t->range = (SDL_Rect) {
		.x = position.x + (range / 2),
		.y = position.y + (range / 2),
		.w = range / 2,
		.h = range / 2
	};
	t->event = event;
	t->event_data = event_data;
	t->enabled = true;
	t->triggered = false;

	t->visible = visible;
	t->display_rect = (SDL_Rect) {
		.x = t->range.x,
		.y = t->range.y,
		.w = 128,
		.h = 128
	};
	
	if (!default_texture)
		default_texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/trigger.png");
	t->display_texture = default_texture;

	FE_List_Add(&triggers, t);
	return t;
}

void FE_Trigger_Render(FE_Camera *camera)
{
	for (FE_List *l = triggers; l; l = l->next) {
		FE_Trigger *t = l->data;
		if (!PresentGame->DebugConfig.ShowTriggers || t->visible)
			FE_RenderCopy(camera, false, t->display_texture, NULL, &t->display_rect);
	}
}

void FE_Trigger_Clean()
{
	for (FE_List *l = triggers; l; l = l->next) {
		FE_Trigger *t = l->data;
		FE_DestroyResource(t->display_texture->path);
		free(t);
	}
	FE_List_Destroy(&triggers);
	triggers = 0;
}

void FE_Trigger_Destroy(FE_Trigger *trigger)
{
	if (!trigger) {
		warn("Passing NULL to trigger (FE_Trigger_Destroy)");
		return;
	}
	FE_DestroyResource(trigger->display_texture->path);
	FE_List_Remove(&triggers, trigger);
	free(trigger);
}

bool FE_Trigger_Check(SDL_Rect *rect)
{
	bool result = false;

	for (FE_List *l = triggers; l; l = l->next) {
		FE_Trigger *t = l->data;
		if (!t->enabled) continue;
		if (FE_AABB_Collision(rect, &t->range)) {
			if (t->triggered != true)
				t->event(t->event_data);
			result = true;
			t->triggered = true;
		} else {
			t->triggered = false;
		}
	}
	return result;
}