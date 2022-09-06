#include "include/internal.h"
#include "../ext/ini.h"
#include "../core/include/utils.h"
#include "../core/lib/string.h"

ServerConfig server_config;

void Server_DestroyConfig()
{
    server_config.has_message = false;
    if (server_config.message != NULL) {
        free(server_config.message);
        server_config.message = NULL;
    }
    if (server_config.map != NULL) {
        free(server_config.map);
        server_config.map = NULL;
    }
    if (server_config.rcon_pass != NULL) {
        free(server_config.rcon_pass);
        server_config.rcon_pass = NULL;
    }
}

static int ConfigParser(void *user, const char *section, const char *name, const char *value)
{
    (void)user;
    if (MATCH("SERVER", "port"))
        server_config.port = atoi(value);
    else if (MATCH("SERVER", "map"))
        server_config.map = mstrdup(value);
    else if (MATCH("SERVER", "maxplayers"))
        server_config.max_players = atoi(value);
    else if (MATCH("SERVER", "rcon_password"))
        server_config.rcon_pass = mstrdup(value);
    
    if (MATCH("MESSAGE", "has_message"))
        server_config.has_message = (bool)atoi(value);
    else if (MATCH("MESSAGE", "message"))
        server_config.message = mstrdup(value);
    
    return 1;
}

void Server_LoadConfig()
{
    // set defaults
    server_config.port = 0;
    server_config.map = 0;
    server_config.max_players = 0;
    server_config.rcon_pass = 0;
    server_config.has_message = false;
    server_config.message = 0;

    if (ini_parse("server.ini", ConfigParser, NULL) < 0)
        warn("[SERVER] Can't load server.ini file");

    // load default values for fields not set
    if (server_config.map == NULL)
        server_config.map = mstrdup("test");
    if (!server_config.port)
        server_config.port = 7777;
    if (!server_config.max_players)
        server_config.max_players = 32;
}