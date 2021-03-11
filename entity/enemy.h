#pragma once

enum EnemyType {
	funny_mushroom_man,
};

struct enemylist { // create a linked list for all enemies
	SDL_Texture *Text;
	SDL_Rect rect;
	SDL_Rect render_rect;
	int health;
	int dir;
	enum EnemyType type;
	struct enemylist *next;
};

void CreateEnemy(enum EnemyType type, int xpos, int ypos);
void EnemyBehaviour();
void KillEnemy(struct enemylist *l);
void RenderEnemy();