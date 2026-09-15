#pragma once
#include "pch.h"

bool init_network_client(int port);
void broadcast_data(const uint8_t *data, size_t len);