#include "socket.h"

static int server_fd = -1;
static struct sockaddr_in client_addr;

bool init_network_client(int port) 
{
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) return false;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);

    /* Set client address to localhost */
    inet_pton(AF_INET, SERVER_IP, &client_addr.sin_addr);

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    return true;
}

void broadcast_data(const uint8_t *data, size_t len) 
{
    /* Sanity check */
    if (server_fd < 0 || !data || len == 0) return;

    /* Broadcast the data to the server */
    sendto(server_fd, data, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
}
