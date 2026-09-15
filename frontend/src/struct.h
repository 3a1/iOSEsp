#pragma once 
#include "pch.h"

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct {
    float x;
    float y;
} vec2_t;

typedef struct __attribute__((packed)) {
    float max_health;
    float health;
    vec2_t screen_box_x;
    vec2_t screen_box_y;
} player_info_t;

typedef struct __attribute__((packed)) {
    int player_count;
} package_header_t;

typedef struct __attribute__((packed)) {
    package_header_t header;
    player_info_t players_info[MAX_PLAYERS];
} network_package_t;