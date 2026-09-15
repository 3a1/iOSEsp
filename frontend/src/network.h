#pragma once 
#include "pch.h"

/* Extern from Obj-C */
void update_screen_overlay(player_info_t* players_info, int player_count);
void clear_screen_overlay(void);

int start_network_server();