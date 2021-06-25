#include "../game.h"
#include <math.h>

struct enemylist *enemylist;




// README: THIS FILE DOES NOT WORK, ENEMIES SEEM TO NOCLIP AND COMMIT SUICIDE. DONT SPAWN ENEMIES!!!!!!!




void CreateEnemy(enum EnemyType type, int xpos, int ypos) // add enemy to linked list
{ 
	struct enemylist *newenemy;
	SDL_Rect enemyrect;
	SDL_Texture *text;
	newenemy = malloc(sizeof(*newenemy));

	switch (type) { // DEFINE PARAMS FOR DIFFERENT ENEMIES HERE
		case funny_mushroom_man:
				newenemy->health = 20;
				enemyrect.w = 50;
				enemyrect.h = 50;
				newenemy->dir = 0;
				text = TextureManager("game/doge.png", renderer);
			break;
		default:
			warn("No enemy of that class exists");
			break;
	}

	if (text) {
		enemyrect.x = xpos;
		enemyrect.y = ypos;
		newenemy->Text = text;
		newenemy->type = type;
		newenemy->rect = enemyrect;
		newenemy->render_rect = enemyrect;
		newenemy->next = enemylist;
		enemylist = newenemy;
	}

}

void KillEnemy(struct enemylist *l)  // remove enemy from list
{
	struct enemylist *tmp;

	if (enemylist == l) { // if object we want to remove is the current one
		SDL_DestroyTexture(l->Text);
		enemylist = enemylist->next;
	} else {
		for (struct enemylist *t = enemylist; t && t->next; t = t->next) {
			if (t->next == l) {
				SDL_DestroyTexture(t->next->Text);
				tmp = t->next;
				t->next = t->next->next;
				free(tmp);
			}
		}
	}

	int r = rand() % 200;
	info("Enemy died of natural causes at the age of %i", r);
}

void EnemyInteract(int dir, struct enemylist *o) // what happens when enemy types collide with player
{
	switch (o->type) { // different enemies different properties
		case funny_mushroom_man:
			if (dir == 0) 
				o->health = 0;
			else if (dir == 1) {
				HealthChange(-20);
				PlayerMove(-10,0);
			} else if (dir == 2) {
				HealthChange(-20);
				PlayerMove(10,0);
			}
	}
}

void EnemyCollision(struct enemylist *o)
{
	if (SDL_HasIntersection(&playerRect, &o->rect)) { // handle collision
		//The sides of the rectangles
		int leftA, leftB;
		int rightA, rightB;
		int topB, bottomA;

		//Calculate the sides of player rect
		leftA = playerRect.x;
		rightA = playerRect.x + playerRect.w;
		bottomA = playerRect.y + playerRect.h;

		//Calculate the sides of obj rect
		leftB = o->rect.x;
		rightB = o->rect.x + o->rect.w;
		topB = o->rect.y;

		if (bottomA <= topB + 5) // collision from above enemy
			EnemyInteract(0, o);

		if (rightA <= leftB + 5) // collision from left of enemy
			EnemyInteract(1, o);

		if (leftA >= rightB - 5) // collision from right of enemy
			EnemyInteract(2, o);

	}
}

void EnemyPhysics(struct enemylist *o)
{
	if (!gAbove(o->render_rect)) {// gravity
		o->render_rect.y += 4;
		o->dir = 2;
	} else if (o->dir == 2)
		o->dir = 0;

	if (o->render_rect.y > screen_height) // if player falls through world
		KillEnemy(o);
}

void EnemyBehaviour() // responsible for collision, movement etc
{
	for (struct enemylist *o = enemylist; o; o = o->next) { // for each object
		EnemyCollision(o);

		if (o->health <= 0)
			KillEnemy(o);

		switch (o->type) { // INDIVIDUAL ENEMY BEHAVIOUR HERE - add your own enemy types
			case funny_mushroom_man:
				EnemyPhysics(o); // add physics


				// Change direction when hitting a wall
				if (o->dir == 0) {
					if (!gLeft(o->render_rect))
						o->render_rect.x += 2;
					else
						o->dir = 1;
				} else if (o->dir == 1) {		
					if (!gRight(o->render_rect))
						o->render_rect.x -= 2;
					else 
						o->dir = 0;
				}
				
			break;
			
			default:
				warn("An enemy of that type has no configured behaviour");
				break;
			
		}
	}
	
}

void RenderEnemy() { // render enemy
	
	for (struct enemylist *o = enemylist; o; o = o->next) {
		o->rect.x = o->render_rect.x - scrollam; // scroll enemy with tiles
		o->rect.y = o->render_rect.y - hscrollam;
		if (o->render_rect.x - (scrollam - o->render_rect.w)  >= 0 && o->render_rect.x -scrollam <= screen_width) // only bother rendering stuff in bounds
			SDL_RenderCopy(renderer, o->Text, NULL, &o->rect);
	}
}
