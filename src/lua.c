// Running lua functions and lua wrappers
#include "game.h"
#include <lauxlib.h>
#include <lualib.h>

static lua_State *L;

static bool LuaReady;

static int l_LoadMusic(lua_State *l)
{
    const char *path;
    path = luaL_checkstring(l, 1);
    lua_newtable(l);

    bgMusic = LoadMusic(path);
    Mix_PlayMusic(bgMusic, -1);
    return 0;
}

static int l_SaveGame(lua_State *l)
{
    const char *path;
    path = luaL_checkstring(l, 1);
    lua_newtable(l);

    SaveGame(path);
    return 0;
}

static int l_LoadSave(lua_State *l)
{
    const char *path;
    path = luaL_checkstring(l, 1);
    lua_newtable(l);

    LoadSave(path);
    return 0;
}

static int l_InitMap(lua_State *l) // Load map wrapper
{
    const char *path;
    path = luaL_checkstring(l, 1);
    lua_newtable(l);

    InitMap(path);
    return 0;
}

static int l_Quit() // Quit application
{
    GameActive = false;
    return 0;
}

static int l_MovePlayer()
{
    int x,y;
    x = luaL_checknumber(L, 1);
    y = luaL_checknumber(L, 2);

    SpawnPlayer(x, y, playerRect.w, playerRect.h);
    info("New position - X:%i Y:%i", x, y);
    return 0;
}

static int l_Pos()
{
    info("SCR X: %i SCR Y: %i - WORLD X: %i WORLD Y: %i", playerRect.x, playerRect.y, CollRect.x, CollRect.y);
    return 0;
}


int InitLua()
{
    L = luaL_newstate();
	luaL_openlibs(L);

    // Register wrapper functions..
    lua_register(L, "initmap", l_InitMap);
    lua_register(L, "quit", l_Quit);
    lua_register(L, "move", l_MovePlayer);
    lua_register(L, "pos", l_Pos);
    lua_register(L, "save", l_SaveGame);
    lua_register(L, "load", l_LoadSave);
    lua_register(L, "playmusic", l_LoadMusic);

    info("Lua ready");

    LuaReady = true;
    return 0;
}

void RunLuaFile(char *f) // Loads lua game script
{
    if (LuaReady) {
        if (luaL_loadfile(L, f))
            error("Lua: Failed to load file %s", f);

        if (lua_pcall(L, 0, 0, 0))
            error("lua_pcall() failed.");

        lua_getglobal(L, "init"); // Run init() function in lua to setup game

        if (lua_pcall(L, 0, 0, 0))
            error("Lua: Unable to call init() function in file %s",f);
    } else
        error("Lua was not ready to run Lua file (was InitLua() called?)");

}

void RunLua(char *s) // Runs direct lua from strings
{
    if (LuaReady) {
        if (strlen(s) < 1)
            return;

        luaL_loadstring(L, s);

        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            info("LUA ERR: %s", luaL_checkstring(L, -1));

    } else
        warn("RunLua() was called but lua was not ready");

}