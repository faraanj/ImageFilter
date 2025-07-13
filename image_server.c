#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

#include "socket.h"
#include "request.h"
#include "response.h"

#ifndef PORT
#define PORT 30000
#endif

#define BACKLOG 10
#define MAX_CLIENTS 10

// Read data from a client socket and spawn child process to respond
int handle_client(ClientState *client)
{
    // Read in data from the client's socket into its buffer,
    // and update num_bytes. If no bytes were read, return 1.

    // IMPLEMENT THIS

    int num_bytes = 0;
    if ((num_bytes = read_from_client(client)) == -1)
    {
        return 1;
    }

    int parse = 0;
    if ((parse = parse_req_start_line(client)) == -1)
    {
        return 1;
    }

    // Fork process - parent returns 1, child handles request
    pid_t result = fork();

    if (result < 0)
    {
        perror("Fork error");
        return 1;
    }
    else if (result == 0)
    {
        if (strcmp(client->reqData->method, GET) == 0)
        {
            if (strcmp(client->reqData->path, MAIN_HTML) == 0)
            {
                main_html_response(client->sock);
            }
            else if (strcmp(client->reqData->path, IMAGE_FILTER) == 0)
            {
                image_filter_response(client->sock, client->reqData);
            }
            else
            {
                not_found_response(client->sock);
            }
        }
        else if (strcmp(client->reqData->method, POST) == 0)
        {
            if (strcmp(client->reqData->path, IMAGE_UPLOAD) == 0)
            {
                image_upload_response(client);
            }
            else
            {
                not_found_response(client->sock);
            }
        }
        else
        {
            not_found_response(client->sock);
        }
        exit(0);
    }
    else
    {
        return 1;
    }

    close(client->sock);
    exit(0);
}

int main(int argc, char **argv)
{
    ClientState *clients = init_clients(MAX_CLIENTS);

    struct sockaddr_in *servaddr = init_server_addr(PORT);

    // Create an fd to listen to new connections
    int listenfd = setup_server_socket(servaddr, BACKLOG);

    // Print out information about this server
    char host[MAX_HOSTNAME];
    if ((gethostname(host, sizeof(host))) == -1)
    {
        perror("gethostname");
        exit(1);
    }
    fprintf(stderr, "Server hostname: %s\n", host);
    fprintf(stderr, "Port: %d\n", PORT);

    // Set up the arguments for select
    int maxfd = listenfd;
    fd_set allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // Set up a timer for select (This is only necessary for debugging help)
    struct timeval timer;

    // Main server loop.
    while (1)
    {
        fd_set rset = allset;
        timer.tv_sec = 2;
        timer.tv_usec = 0;
        int nready = select(maxfd + 1, &rset, NULL, NULL, &timer);
        if (nready == -1)
        {
            perror("select");
            exit(1);
        }

        if (nready == 0)
        { // timer expired
            // Check if any children have failed
            int status;
            int pid;
            errno = 0;
            if ((pid = waitpid(-1, &status, WNOHANG)) > 0)
            {
                if (WIFSIGNALED(status))
                {
                    fprintf(stderr, "Child [%d] failed with signal %d\n", pid,
                            WTERMSIG(status));
                }
            }
            continue;
        }

        if (FD_ISSET(listenfd, &rset))
        { // New client connection
            int new_client_fd = accept_connection(listenfd);
            if (new_client_fd >= 0)
            {
                maxfd = (new_client_fd > maxfd) ? new_client_fd : maxfd;
                FD_SET(new_client_fd, &allset); // Add new descriptor to set

                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i].sock < 0)
                    {
                        clients[i].sock = new_client_fd;
                        break;
                    }
                }
            }

            nready -= 1;
        }

        // Check all client sockets for ready data
        for (int i = 0; i < MAX_CLIENTS && nready > 0; i++)
        {
            // Check whether clients[i] has an active, ready socket
            if (clients[i].sock < 0 || !FD_ISSET(clients[i].sock, &rset))
            {
                continue;
            }

            int done = handle_client(&clients[i]);
            if (done)
            {
                FD_CLR(clients[i].sock, &allset);
                remove_client(&clients[i]);
            }
            nready -= 1;
        }
    }
}
