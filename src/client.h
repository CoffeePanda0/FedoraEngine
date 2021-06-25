#pragma once

void parse_data(char *data);
int init_client(const char* host, unsigned int port);
char* get_packet();
int send_packet(char *msg);
void render_multiplayer();
