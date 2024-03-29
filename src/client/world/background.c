#include "../core/include/include.h"

#include "include/background.h"

#include <file.h>
#include "../../common/ext/inih/ini.h"

#ifdef _WIN32
	#include "../common/ext/dirent.h" // for finding textures in dir
#else
	#include <dirent.h> // for finding textures in dir
#endif

#define PARALLAX_DIRECTORY "game/map/parallax/"

static int layer_width = 1920;
static int layer_height = 1080; 

static float last_x;
static float last_y;
static float last_zoom;

static FE_Map_Parallax *parallax;
static size_t parallax_layers;
static float parallax_speed = 1.0f;
static bool parallax_dirty = false;
static float layer_scale = 1.0f;

static char *parallax_name;

FE_StrArr *FE_Parallax_Count()
{
    FE_StrArr *arr = FE_StrArr_Create();

    // calculate amount of subdirectories with a parallax.ini file inside
    DIR *dir = opendir(PARALLAX_DIRECTORY);
    if (dir == NULL)
        error("Could not open parallax directory");

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        char *path = xmalloc(strlen(PARALLAX_DIRECTORY) + strlen(ent->d_name) + 14);
        sprintf(path, "%s%s/parallax.ini", PARALLAX_DIRECTORY, ent->d_name);

        if (FE_File_DirectoryExists(path)) {
            FE_StrArr_Add(arr, ent->d_name);
        }
        
        free(path);
    }
    closedir(dir);
    return arr;
}

static int ConfigParser(void *user, const char *section, const char *name, const char *value)
{
    (void)user;

    static char *dir = 0;
    static short last_layer = 0;

    if (MATCH("CONFIG", "name")) { // load each layer name
        last_layer = 0;
        if (dir) free(dir);
        dir = mstradd(value, "/");
    } else if (MATCH("CONFIG", "ratio")) { // load each layer ratio movement
        parallax_speed = atof(value);
    } else if (MATCH("CONFIG", "scale")) { // load the factor to scale the bg by
        layer_scale = atof(value);
    } else if (MATCH("LAYERS", "path")) { // load each layer path
        char *path = mstradd(dir, value);
        FE_Parallax_Add(path, 1.0f);
        free(path);
    } else if (MATCH("SPEED", "value")) { // load each layer speed
        if (last_layer == (short)parallax_layers) {
            warn("Parallax config in incorrect order, or too many speed arguments");
            return 0;
        }
        parallax[last_layer++].scale = atof(value);
    } else {
        info("Unknown parallax config option: %s.%s", section, name);
    }
    
    return 1;
}

void FE_Parallax_Load(const char *name)
{
    FE_Parallax_Clean();
    // Check that parallax directory exists
    char *full_path = xmalloc(mstrlen(name) + mstrlen(PARALLAX_DIRECTORY) + 2);
    snprintf(full_path, mstrlen(name) + mstrlen(PARALLAX_DIRECTORY) + 2, "%s%s/", PARALLAX_DIRECTORY, name);
    if (!FE_File_DirectoryExists(full_path)) {
        warn("Parallax directory %s does not exist", name);
        free(full_path);
        return;
    }

    // Load config file
    char *ini_path = mstradd(full_path, "parallax.ini");
    if (ini_parse(ini_path, ConfigParser, NULL) < 0) {
        warn("Can't load parallax config file %s", ini_path);
        free(full_path);
        free(ini_path);
        return;
    }
    free(full_path);
    free(ini_path);
    parallax_name = mstrdup(name);

    if (parallax_layers < 1)
        warn("Parallax %s has no layers", name);
}

void FE_Parallax_SetSpeed(float speed)
{
    parallax_speed = speed;
    parallax_dirty = true;
}

void FE_Parallax_Add(const char *layer_name, float scale)
{
    if (!parallax)
        parallax = xmalloc(sizeof(FE_Map_Parallax));
    else
        parallax = xrealloc(parallax, sizeof(FE_Map_Parallax) * (parallax_layers + 1));
    

    char *path = mstradd(PARALLAX_DIRECTORY, layer_name);
    parallax[parallax_layers].texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    free(path);

    SDL_QueryTexture(parallax[0].texture->Texture, NULL, NULL, &layer_width, &layer_height);

    layer_width *= layer_scale;
    layer_height *= layer_scale;

    parallax[parallax_layers].scale = scale;

    int ypos = PresentGame->MapConfig.Loaded ? PresentGame->MapConfig.MapHeight - layer_height : 0;
    parallax[parallax_layers].r1 = (SDL_Rect){0, ypos, layer_width, layer_height};
    parallax[parallax_layers++].r2 = (SDL_Rect){-layer_width, ypos, layer_width, layer_height};
    parallax_dirty = true;
}

void FE_Parallax_Clean()
{
    if (parallax_layers == 0)
        return;
    
    for (size_t i = 0; i < parallax_layers; i++)
        FE_DestroyResource(parallax[i].texture->path);
    free(parallax);
    if (parallax_name)
        free(parallax_name);
        
    parallax_name = 0;
    parallax = 0;
    parallax_dirty = false;
    parallax_layers = 0;
    parallax_speed = 1.0f;
    last_x = 0;
    last_y = 0;
    last_zoom = 0;
    layer_width = 0;
    layer_height = 0;
    parallax_name = 0;
}

static void FE_Parallax_Render(FE_Camera *camera)
{
    if (!parallax) return;

    SDL_Rect vis_rect = SCREEN_RECT(camera);

    static SDL_Texture *buff = 0;

    // Render to a buffer so we can just re-render texture if camera hasn't moved
    if (!buff) {
        buff = FE_CreateRenderTexture(PresentGame->WindowWidth, PresentGame->WindowWidth);
        SDL_SetTextureBlendMode(buff, SDL_BLENDMODE_BLEND);
    }


    if (last_x != camera->x || last_y != camera->y || parallax_dirty || last_zoom != camera->zoom) {
        last_x = camera->x; last_y = camera->y; last_zoom = camera->zoom;

        for (size_t i = 0; i < parallax_layers; i++) {
            float scale = parallax[i].scale * parallax_speed;
            
            // Apply scale to the camera position
            SDL_Rect dst1 = {
                .x = (parallax[i].r1.x - (camera->x * scale)) * camera->zoom,
                .y = (parallax[i].r1.y -camera->y) * camera->zoom,
                .w = layer_width * camera->zoom,
                .h = layer_height * camera->zoom
            };

            // Apply scale to the camera position (second rect for infinite scrolling)
            SDL_Rect dst2 = {
                .x = (parallax[i].r2.x - (camera->x * scale)) * camera->zoom,
                .y = dst1.y,
                .w = layer_width * camera->zoom,
                .h = layer_height * camera->zoom
            };

            // Check if the rects need to be moved to scroll horizontally
            if (dst2.x + dst2.w < 0)
                parallax[i].r2.x = parallax[i].r1.x + parallax[i].r1.w;
            if (dst1.x + dst1.w < 0)
                parallax[i].r1.x = parallax[i].r2.x + parallax[i].r2.w;
            if (dst1.x > PresentGame->WindowWidth)
                parallax[i].r1.x = parallax[i].r2.x - parallax[i].r2.w;
            if (dst2.x > PresentGame->WindowWidth)
                parallax[i].r2.x = parallax[i].r1.x - parallax[i].r1.w;

            SDL_Texture *prev = SDL_GetRenderTarget(PresentGame->Client->Renderer);
            SDL_SetRenderTarget(PresentGame->Client->Renderer, buff);

            SDL_RenderCopy(PresentGame->Client->Renderer, parallax[i].texture->Texture, &vis_rect, &dst1);
            SDL_RenderCopy(PresentGame->Client->Renderer, parallax[i].texture->Texture, &vis_rect, &dst2);

            SDL_SetRenderTarget(PresentGame->Client->Renderer, prev);
            parallax_dirty = false;
        }
    }
    SDL_RenderCopy(PresentGame->Client->Renderer, buff, &vis_rect, NULL);
}

char *FE_Parallax_GetName()
{
    return parallax_name;
}

void FE_Map_RenderBG(FE_Camera *camera, FE_LoadedMap *map)
{
    /*  Render background twice so that we can loop background for infinite effect
        This is not hugely efficient and will use a lot of SDL_Render time.
        NOTE: BG width must be at least screen size
        We render to a buffer so that we can just copy the texture if not dirty to improve render time
        Map is not required for a parallax background
    */

    if (!camera) {
        warn("Passing NULL camera on rendering BG");
        return;
    }

    if (parallax) {
        FE_Parallax_Render(camera);
        return;
    }

    if (!map) {
        warn("Passing NULL map on rendering BG");
        return;
    }
    
    SDL_Rect vis_rect = SCREEN_RECT(camera);

    static SDL_Texture *buffer = 0;

    static bool set = false;
    static SDL_Rect r1 = {0, 0, 0, 0};
    static SDL_Rect r2 = {0, 0, 0, 0};

    if (!set) {
        r1.w = layer_width; r1.h = layer_height;
        r2.w = layer_width; r2.h = layer_height;
        r2.x = -layer_width;
        set = true;
    }


    if (!buffer)
        buffer = FE_CreateRenderTexture(PresentGame->WindowWidth, PresentGame->WindowHeight);
    
    SDL_Texture *prev = SDL_GetRenderTarget(PresentGame->Client->Renderer);
    SDL_SetRenderTarget(PresentGame->Client->Renderer, buffer);

    if (last_x != camera->x || last_y != camera->y) {
        // note: this code is a monster and should not be reckoned with
        last_x = camera->x;
        last_y = camera->y;

        SDL_Rect bg1 = r1;
        SDL_Rect bg2 = r2;

        if (bg2.x - camera->x + bg2.w < 0)
            r2.x = r1.x + bg1.w;
        if (bg1.x - camera->x + bg1.w < 0)
            r1.x = r2.x + r1.w;
        if (bg1.x - camera->x + bg1.w > camera->x + PresentGame->WindowWidth)
            r1.x = r2.x - r2.w;
        
        bg1.y -= camera->y;
        bg1.x -= camera->x * camera->zoom;
        bg1.y *= camera->zoom;
        bg1.w *= camera->zoom;
        bg1.h *= camera->zoom;
        bg2 = bg1;
        bg2.x = (r2.x - camera->x) * camera->zoom;

        SDL_RenderCopy(PresentGame->Client->Renderer, map->r->bg->Texture, &vis_rect, &bg1);
        SDL_RenderCopy(PresentGame->Client->Renderer, map->r->bg->Texture, &vis_rect, &bg2);
    }
    
    SDL_SetRenderTarget(PresentGame->Client->Renderer, prev);
    SDL_RenderCopy(PresentGame->Client->Renderer, buffer, &vis_rect, NULL);
    
}