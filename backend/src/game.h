#pragma once
#include "pch.h"

#define M_PI_F 3.1415926535f
#define RAD2DEG(x) ((x) * (180.0f / M_PI_F))
#define DEG2RAD(x) ((x) * (M_PI_F / 180.0f))

typedef struct {
  uint64_t wstring;
  int32_t length;
  int32_t capacity;
} FString;

typedef struct {
  uint64_t array;
  int32_t count;
  int32_t max;
} TArray;

typedef struct {
  float x, y;
} vec2_t;

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float x, y, z, w;
} vec4_t;

typedef struct {
  float pitch, yaw;
} rot2_t;

typedef struct {
  float pitch, yaw, roll;
} rot3_t;

// Object Name: Class Engine.World
// Size: 0xe38 // Inherited bytes: 0x28
typedef struct {
	// Fields
	char pad_0x28[0x30]; // Offset: 0x28 // Size: 0x08
	uint64_t PersistentLevel; // Offset: 0x30 // Size: 0x08
	uint64_t NetDriver; // Offset: 0x38 // Size: 0x08
} UWorld;

// Object Name: ScriptStruct Engine.MinimalViewInfo
// Size: 0x5b0 // Inherited bytes: 0x00
typedef struct {
	// Fields
	vec3_t Location; // Offset: 0x00 // Size: 0x0c
	vec3_t LocationLocalSpace; // Offset: 0x0c // Size: 0x0c
	rot3_t Rotation; // Offset: 0x18 // Size: 0x0c
  float FOV; // Offset: 0x24 // Size: 0x04
} FMinimalViewInfo;

// Object Name: ScriptStruct Engine.CameraCacheEntry
// Size: 0x5c0 // Inherited bytes: 0x00
typedef struct {
	// Fields
	float TimeStamp; // Offset: 0x00 // Size: 0x04
	char pad_0x4[0xc]; // Offset: 0x04 // Size: 0x0c
	FMinimalViewInfo POV; // Offset: 0x10 // Size: 0x5b0
} FCameraCacheEntry;

//
//  Network packages information.
//

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

extern network_package_t network_package;

uint64_t decrypt_uworld_pointer(uint64_t ttep, uint64_t a1);
bool check_uid(uint64_t ttep, uint64_t fstring);
void broadcast_players_data(int player_count);
bool world_to_screen(vec3_t local_position, rot3_t local_rotation, vec3_t world_position, float fov, vec2_t *out_screen);
float calculate_distance(vec3_t v1, vec3_t v2);
uint64_t DecryptActorsArray(uint64_t ttep, uint64_t uLevel, int Actors_Offset, int EncryptedActors_Offset);
