#ifndef H_GAMEOBJECT
#define H_GAMEOBJECT

#include "SDL.h"
#include "physics.h"
#include "camera.h"

enum FE_ObjectType {
	ENEMY,
};

typedef struct FE_GameObject {
	char *name;
	enum FE_ObjectType type;
	FE_PhysObj *phys;
	SDL_Texture* texture;
} FE_GameObject;

void FE_CleanGameObjects();
void FE_RenderGameObjects(FE_Camera *c);
int FE_DestroyGameObject(struct FE_GameObject *obj);
FE_GameObject *FE_CreateGameObject(SDL_Rect r, const char *texture_path, char *name, enum FE_ObjectType type, int mass);

#endif