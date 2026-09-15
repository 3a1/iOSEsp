#include "network.h"

int start_network_server() 
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) return -1;

    /* Set a one second timeout */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) 
    {
        close(sock_fd);
        return -1;
    }

    struct sockaddr_in rx_addr;
    memset(&rx_addr, 0, sizeof(rx_addr));
    rx_addr.sin_family = AF_INET;
    rx_addr.sin_port = htons(SERVER_PORT);
    rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (struct sockaddr *)&rx_addr, sizeof(rx_addr)) < 0) 
    {
        close(sock_fd);
        return -1;
    }

    /* Create incoming players buffer */
    network_package_t network_package = { 0 };
    /* Max buffer size is the size of entire package structure */
    size_t max_buffer_size = sizeof(network_package_t);

    while (true) 
    {
        /* Receive players data */
        ssize_t bytes_received = recvfrom(sock_fd, &network_package, max_buffer_size, 0, NULL, NULL);
        
        /* Sanity check on received bytes */
        if (bytes_received < 0) 
        {
            /* We receive timeout? */
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                /* Yes? Let's clear the screen and wait again */
                clear_screen_overlay();
                continue; 
            }

            /* Handle errors */
            if (errno == EBADF || errno == EINVAL) 
            {
                break; 
            }

            continue; 
        }

        /* We receive package with no players? */
        if (network_package.header.player_count == 0) {
            /* Clear the screen! */
            clear_screen_overlay();
            continue;
        }

        /* Send players list into Obj-C layer to draw on the overlay */
        update_screen_overlay(network_package.players_info, network_package.header.player_count);
    }

    close(sock_fd);
    return 0;
}