#include <stdio.h>
#include <stdlib.h>
#include "game.h"

#define ZED_NET_IMPLEMENTATION
#include "ext/zed_net.h"

static zed_net_address_t address;
static zed_net_udp_socket_t *sock;

struct Player {
    SDL_Rect rect;
    SDL_RendererFlip flip;
    char *name;
};

int playercount = 0;

SDL_Texture *text;

struct Player *PlayerList;

void spawn_multiplayer(char **command) {
    struct Player NewPlayer;
    SDL_Rect NewRect;

    NewPlayer.name = strdup(command[3]);

    NewRect.w = playerRect.w;
    NewRect.h = playerRect.h;
    NewRect.x = atoi(command[1]);
    NewRect.y = atoi(command[2]);
    NewPlayer.flip = SDL_FLIP_NONE;
    NewPlayer.rect = NewRect;
    PlayerList[playercount] = NewPlayer;
    playercount++;
    info("Player has joined the game %s", NewPlayer.name);
    PlayerList = realloc(PlayerList, (sizeof(struct Player) * (playercount + 1)));
}

void despawn_multiplayer(char **command) {
    for (int i = 0; i < playercount; i++) {
        if (strcmp(PlayerList[i].name, command[1]) == 0 ) {
            free(PlayerList[i].name);
            for (int a = i; i < playercount -1; a++)
                PlayerList[a] = PlayerList[a+1];
        }

    }
    playercount--;
    PlayerList = realloc(PlayerList, (sizeof(struct Player) * (playercount + 1)));
}

void render_multiplayer() {
    if (playercount > 0) {
        for (int i = 0; i < playercount; i++) {
            SDL_Rect tmp = PlayerList[0].rect;
            tmp.x -= scrollam;
            tmp.y -= hscrollam;

            SDL_RenderCopyEx(renderer, text, NULL, &tmp, 0, NULL, PlayerList[i].flip);
        }
    }
}

int send_packet(char *msg) {
    if (zed_net_udp_socket_send(sock, address, msg, strlen(msg)) == -1) {
        warn("Error sending packet: %s\n", zed_net_get_error());
        return -1;
    } else
        return 0;
}

char* get_packet() {
    zed_net_address_t sender;
    static char buf[1024];
    memset(buf, 0, 1024);
    zed_net_udp_socket_receive(sock, &sender, &buf, 1024);
    
    return buf;
}

int init_client(const char* host, unsigned int port) {
    text = TextureManager("../game/player.png", renderer);
    PlayerList = malloc(sizeof(struct Player));
    if (zed_net_init() == 1) {
        warn("Error: Failed to initialise UDP client");
        return -1;
    }
    
    sock = zed_net_udp_socket_open(0, 1);

    if (!sock)
        printf("Error opening socket: %s\n", zed_net_get_error());

    if (zed_net_get_address(&address, host, port) != 0) {
        warn("Error connecting to IP: %s\n", zed_net_get_error());
        zed_net_udp_socket_close(sock);
        zed_net_shutdown();
        return -1;
    }
    
    info("Started UDP client, listening IP: %s PORT: %i", host, port);
    return 0;
}

void move_multiplayer(char **command) {

    int x,y;

    x = atoi(command[1]);
    y = atoi(command[2]);

    if (x < 0 || y < 0)
        return;

    for (int i = 0; i < playercount; i++) {
        if (strcmp(PlayerList[i].name, command[3]) == 0) {

            if (PlayerList[i].rect.x - x > 0)
                PlayerList[i].flip = SDL_FLIP_NONE;
            else
                PlayerList[i].flip = SDL_FLIP_HORIZONTAL;

            PlayerList[i].rect.x = x;
            PlayerList[i].rect.y = y;
        }
    }

}

void parse_data(char *data) {
    if (strlen(data) < 4)
        return;

    if (strcmp(data, "SUCCESS") == 0)
        return;

    if (strcmp(data, "Unknown command") == 0)
        return;

    char *s, *string,*parsed,*parsed2;

    s = strdup(data);
    string = s;

    size_t index = 0;
    size_t argindex = 0;

    char **command;

    int args = 4;

    command = malloc(sizeof(*command) * (args)); // allow up to 64 chars per arg

    while ((parsed = strseps(&string,":")) != NULL) {
        if (index == 0) {
            index++;
            command[0] = strdup(parsed);
        } else {
            while ((parsed2 = strseps(&parsed,",")) != NULL) {
                command[argindex+1] = strdup(parsed2);
                argindex++;
            }
        }
    }
    
    if (s) free(s);

    if (strcmp(command[0], "SPAWN") == 0)
        spawn_multiplayer(command);
    else if (strcmp(command[0], "MOV") == 0)
        move_multiplayer(command);
    else if (strcmp(command[0], "DESPAWN") == 0)
        despawn_multiplayer(command);
    else
        info("Unknown Command from server:  %s",command[0]);
    
    for (size_t i = 0; i <= argindex; i++)
        free(command[i]);
    free(command);

}

