#pragma once
#include "pch.h"

/* Pull primitives initialization from jb library */
extern int jbclient_initialize_primitives_internal(bool physrwPTE);

void game_loop(uint64_t ttep, uint64_t uworld_pointer);
int main(int argc, char *argv[]);
