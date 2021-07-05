//Core game loops and functions for game operation
#include "game.h"
#include "lua.h"

SDL_Window* window;
SDL_Renderer* renderer;

TTF_Font* Sans;
TTF_Font* ConsoleFont;

char *lua_script;

SDL_Texture *playerText;

struct Animation PlayerAnimation;

struct TextObject testText;
Mix_Music *bgMusic;

static SDL_Texture* BgText;
SDL_Rect BgRect;

bool GameActive;
struct GameObject doge;

bool jumping = false;
bool TextPaused = false;
bool intext = false;

int map_width, map_height;

// network stuff
static char *server;
static int port;
static char *bg;


void InitGame() // initializes the things like game objects and maps
{
	InitLua();
	RunLuaFile("../scripts/test.lua");
	lua_script = strdup("../scripts/test.lua");
	Sans = TTF_OpenFont("../game/baloo.ttf", 20);
	ConsoleFont = TTF_OpenFont("../game/Inconsolata.ttf", 20);

	if (bg) {
		BgText = TextureManager(bg,renderer);
		SDL_QueryTexture(BgText, NULL, NULL, &BgRect.w ,&BgRect.h);
	}

	InitPlayer();

	NewText(&testText, Sans, "FedoraEngine", White, 350 , 0); // EXAMPLE TEXT
	MapLoaded(); // make sure user loaded in a map

	
	NewParticleSystem(PARTICLE_SNOW, 0, 0);

	CreateEnemy(funny_mushroom_man, 100, 100);
	CreateObject(150,300,90,90,"../game/doge.png",&doge, "doge");
	PlayDialogue(1,2,"../game/test.dialogue"); // DIALOGUE IS 1 INDEXED
}

void InitPlayer() // change this for your player
{
	InitPlayerUI();
	playerText = TextureManager("../game/player.png", renderer);
	JumpSound = LoadSFX("../game/audio/jump.wav");
	PlayerAnimation = CreateAnimation(75, 100, 8, 2, "../game/help.png");
	dir = DIR_NONE;
}

void Physics() { // handles player movement

	/* Acceleration and deceleration */
	if (moving && !jumping) {
		if (acceleration < maxAccel + 0.03f)
			acceleration += 0.03f;
	} else if (moving && jumping) {
		if (acceleration > 1.0f)
			acceleration -= 0.01f;
	} else {
		if (acceleration > 1.0f)
			acceleration -= 0.1f;
		if (acceleration > 1.0f) {
			// Friction
			if (playerFlip == SDL_FLIP_NONE)
				PlayerMove(-acceleration * 3, 0);
			else
				PlayerMove(acceleration * 3, 0);
		}
	}

	// Velocity and gravity
	if (!gAbove(CollRect) && dir != DIR_ABOVE) {  // If player is not on ground

		if (velocity + 0.15f < maxvelocity)
			velocity += 0.15f;
		PlayerMove(0, velocity+gravity);

	} else if (!jumping)
		velocity = 0;
	
	if (jumping) {
		PlayerMove(0, velocity);

		if (CollRect.y < 0) // check bounds
			velocity = 0;

		if (gBelow(CollRect) || dir == DIR_BELOW) // Check we don't collide from above
			if (velocity < 0)
				velocity = 1;

		if (gAbove(CollRect) || dir == DIR_ABOVE) // Check when player is on ground again
			jumping = false;

	}
}

int *networking() {
	while (GameActive) {
		SDL_Delay(50);
		char *packet = get_packet();

		if (strlen(packet) > 0)
			parse_data(packet);

	}
	return 0;
}

int alive_check() {
	while (GameActive) {
		SDL_Delay(7000);
		send_packet("ALIVE");
	}
	return 0;
}

void Update()
{
	if (!TextPaused) {
		CollisionDetection(); // checks for collision with each game objects
		EnemyBehaviour();
		Physics(); // handles movement, jumping and gravity
	}
}

SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren)
{
	SDL_Surface* s = IMG_Load(texture); // we have this to check the image is valid
	if (s) {
		SDL_Surface* tmpSurface = IMG_Load(texture); 
		SDL_Texture* text = SDL_CreateTextureFromSurface(ren, tmpSurface);
		SDL_FreeSurface(tmpSurface);
		SDL_FreeSurface(s);
		return text;
	} else {
		warn("Texture %s not found", texture);
		SDL_Surface* tmpSurface = SDL_CreateRGBSurface(0,50,50,32,0,0,0,0);
		SDL_Texture* text = SDL_CreateTextureFromSurface(ren, tmpSurface);
		SDL_FreeSurface(tmpSurface);
        SDL_FreeSurface(s);
		return text;
	}
}

Mix_Music* LoadMusic(const char* path)
{
	Mix_Music* tmp = Mix_LoadMUS(path);
	if (!tmp) {
		warn("Could not load music %s, SDL_Error: %s", path, Mix_GetError());
	} else
		info("Loaded music %s", path);
	
	return tmp;
}

Mix_Chunk* LoadSFX(const char* path)
{
	Mix_Chunk* tmp = Mix_LoadWAV(path);
	if (!tmp) {
		warn("Could not load SFX %s, SDL_Error: %s", path, Mix_GetError());
	} else
		info("Loaded SFX %s", path);

	return tmp;
}

char *strseps(char **sp, char *sep)
{
	char *p, *s;
	if (sp == NULL || *sp == NULL || **sp == '\0') return(NULL);
	s = *sp;
	p = s + strcspn(s, sep);
	if (*p != '\0') *p++ = '\0';
	*sp = p;
	return(s);
}

void Render()
{
	SDL_RenderClear(renderer);

	SDL_Rect bgr = BgRect;
	bgr.y -= hscrollam;
	SDL_RenderCopy(renderer, BgText, NULL, &bgr);

	RenderMap();
	RenderObject();
	RenderEnemy();

	RenderParticles();
	RenderUI();

	RenderPlayerAnimation();

	if (multiplayer)
		render_multiplayer();

	// SDL_RenderCopyEx(renderer, playerText, NULL, &playerRect, 0, NULL, playerFlip); // renders the player
	
	SDL_RenderPresent(renderer);

}

int LoadConfig() // loads params from config.txt
{
	FILE *f = fopen("config.txt","r");
	char line[128];
	char *previous;
	char *parsed;
	int linenum = 0;

	if (!f) {
		error("Missing config file");
		return -1;
	} else
		info("Parsing config file");

	while (fgets(line, sizeof(line), f) != NULL) { // loop through each line and parse data
		linenum++;

		char *str = strdup(line);

		int cur = 0;

		while ((parsed = strseps(&str,"=")) != NULL) {
			if (cur == 1) { // here we set the variables

				parsed[strlen(parsed) -1] = 0;

				if (strcmp(previous, "multiplayer") == 0) {
					if (strcmp(parsed, "true") == 0)
						multiplayer = true;
					else
						multiplayer = false;
				}

				else if (strcmp(previous, "server") == 0)
					server = strdup(parsed);
				
				else if (strcmp(previous, "port") == 0)
					port = atoi(parsed);

				else if (strcmp(previous, "background") == 0)
					bg = strdup(parsed);
				
				else {
					error("Invalid syntax in config.txt on line %i",linenum);
					return -1;
				}

			}
			
			previous = strdup(parsed); // remember previous field for the key
			cur++;
		}
		
		// free(str); there is a heap corruption somewhere, funny
	}
	
	fclose(f);
	return 0;
}


void init(const char* window_title, int xpos, int ypos, int window_width, int window_height)
{

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		// Check that everything has loaded in correctly
		info("SDL Fully Initialised");

		window = SDL_CreateWindow(window_title, xpos, ypos, window_width, window_height, SDL_WINDOW_ALLOW_HIGHDPI);
		
		if (window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

		if (renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (TTF_Init() == -1)
    		error("TTF Failed to initialize. SDL_Error: %s", TTF_GetError());
		else
			info("Created TTF");

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", TTF_GetError());
		else
			info("Created IMG");

		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
            error("SDL_mixer failed to initialize! SDL_Error: %s", Mix_GetError());
		else
			info("Initialized Mixer");

		
		LoadConfig();

		// Connect to multiplayer
		if (multiplayer) {
			init_client(server, port);

			info("Trying to connect to server..");
			send_packet("SPAWN");

			char *packet = get_packet();
			while (strcmp(packet, "SUCCESS") != 0) {
				
				packet = get_packet();
				if (strlen(packet) > 0)
					break;
			}
			info("Success! Connected to server");


		}
		free(server);

		paused = false;

		// Set up the game here
		InitGame();
		GameActive = true;

		if (multiplayer) {
			SDL_CreateThread(networking, "networking", (int *)NULL);
			SDL_CreateThread(alive_check, "alive_check", (int *)NULL);
		}
		
	} else {
		GameActive = false;
		error("Could not start SDL. SDL_Error: %s", SDL_GetError());
	}
}

void CleanMemory() // Frees all memory by destroying all objects
{
	GameActive = false;
	Mix_FreeMusic(bgMusic);
	Mix_FreeChunk(JumpSound);
	SDL_DestroyTexture(playerText);
	SDL_DestroyTexture(BgText);
	free(bg);

	DestroyMap();
	if (TextPaused)
		KillMenu();
	if (ConsoleVisible)
		HideConsole();
	
	CleanUI();
	CleanObjects();


	//TODO: CLEAN MULTIPLAYER
}

void Clean()
{
	CleanMemory();
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	TTF_CloseFont(Sans);
	info("SDL Exited \n");
	IMG_Quit();
	Mix_Quit();
	log_close();
	SDL_Quit();
}
