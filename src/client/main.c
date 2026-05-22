#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "app_info.h"

#define BUFFER_SIZE 256

static void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

static void strip_newline(char *text)
{
    if (text == NULL) {
        return;
    }
    text[strcspn(text, "\r\n")] = '\0';
}

int main(void)
{
    printf("%s client %s\n", lanchess_name(), lanchess_version());

    char host[128];
    char port_str[32];

    printf("Enter server host (default 127.0.0.1): ");
    if (fgets(host, sizeof(host), stdin) == NULL) {
        fprintf(stderr, "Failed to read host.\n");
        return EXIT_FAILURE;
    }
    strip_newline(host);
    if (host[0] == '\0') {
        strcpy(host, "127.0.0.1");
    }

    printf("Enter server port: ");
    if (fgets(port_str, sizeof(port_str), stdin) == NULL) {
        fprintf(stderr, "Failed to read port.\n");
        return EXIT_FAILURE;
    }
    strip_newline(port_str);
    if (port_str[0] == '\0') {
        fprintf(stderr, "Port cannot be empty.\n");
        return EXIT_FAILURE;
    }

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, port_str, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    int sock_fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock_fd == -1) {
            continue;
        }

        if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }

        close(sock_fd);
        sock_fd = -1;
    }

    freeaddrinfo(result);

    if (sock_fd == -1) {
        fprintf(stderr, "Unable to connect to %s:%s\n", host, port_str);
        return EXIT_FAILURE;
    }

    printf("Connected to %s:%s\n", host, port_str);

    char buffer[BUFFER_SIZE];
    ssize_t received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (received == -1) {
        die("recv");
    }
    if (received > 0) {
        buffer[received] = '\0';
        printf("Server says: %s\n", buffer);
    }

    const char *message = "LANChess client: connection established\n";
    if (send(sock_fd, message, strlen(message), 0) == -1) {
        die("send");
    }

    printf("Connection ready. Commands:\n");
    printf(" - peers                     : request current peer list\n");
    printf(" - challenge <host> <port>   : challenge a peer to a game\n");
    printf(" - yes / no                  : respond to incoming challenge prompts\n");
    printf(" - quit / exit               : close connection and exit\n");
    static char pending_host[64] = {0};
    static char pending_port[16] = {0};

    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sock_fd, &read_fds);
        FD_SET(fileno(stdin), &read_fds);

        int max_fd = sock_fd > fileno(stdin) ? sock_fd : fileno(stdin);
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            die("select");
        }

        if (FD_ISSET(sock_fd, &read_fds)) {
            ssize_t received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) {
                if (received == 0) {
                    printf("Server closed the connection.\n");
                } else {
                    perror("recv");
                }
                break;
            }
            buffer[received] = '\0';

            /* Handle server control messages */
            if (strncmp(buffer, "PEER_UPDATE", 11) == 0) {
                printf("%s", buffer);
            } else if (strncmp(buffer, "INCOMING_CHALLENGE ", 19) == 0) {
                /* INCOMING_CHALLENGE <host> <port> */
                char ch_host[64], ch_port[16];
                if (sscanf(buffer + 19, "%63s %15s", ch_host, ch_port) == 2) {
                    printf("Incoming challenge from %s:%s\n", ch_host, ch_port);
                    printf("Type 'yes' to accept or 'no' to decline.\n");
                    /* store pending challenge for user response */
                    strncpy(pending_host, ch_host, sizeof(pending_host));
                    pending_host[sizeof(pending_host)-1] = '\0';
                    strncpy(pending_port, ch_port, sizeof(pending_port));
                    pending_port[sizeof(pending_port)-1] = '\0';
                } else {
                    printf("Malformed incoming challenge: %s\n", buffer);
                }
            } else if (strncmp(buffer, "START_GAME", 10) == 0) {
                printf("%s", buffer);
            } else if (strncmp(buffer, "CHALLENGE_SENT", 14) == 0 ||
                       strncmp(buffer, "CHALLENGE_FAILED", 16) == 0 ||
                       strncmp(buffer, "CHALLENGE_DECLINED", 18) == 0 ||
                       strncmp(buffer, "NO_PENDING_CHALLENGE", 20) == 0) {
                printf("Server: %s", buffer);
            } else {
                printf("Server: %s", buffer);
            }
            fflush(stdout);
        }

        if (FD_ISSET(fileno(stdin), &read_fds)) {
            char command[BUFFER_SIZE];
            if (fgets(command, sizeof(command), stdin) == NULL) {
                printf("End of input, exiting.\n");
                break;
            }
            strip_newline(command);
            if (strncmp(command, "challenge ", 10) == 0) {
                char t_host[64], t_port[16];
                if (sscanf(command + 10, "%63s %15s", t_host, t_port) == 2) {
                    char req[BUFFER_SIZE];
                    snprintf(req, sizeof(req), "CHALLENGE %s %s\n", t_host, t_port);
                    if (send(sock_fd, req, strlen(req), 0) == -1) {
                        die("send");
                    }
                } else {
                    printf("Usage: challenge <host> <port>\n");
                }
            } else if (strcmp(command, "peers") == 0) {
                const char *request = "GET_PEERS\n";
                if (send(sock_fd, request, strlen(request), 0) == -1) {
                    die("send");
                }
            } else if (strcmp(command, "yes") == 0 || strcmp(command, "no") == 0) {
                if (pending_host[0] == '\0') {
                    printf("No pending challenge to respond to.\n");
                } else {
                    if (strcmp(command, "yes") == 0) {
                        const char *req = "ACCEPT_CHALLENGE\n";
                        if (send(sock_fd, req, strlen(req), 0) == -1) die("send");
                    } else {
                        const char *req = "DECLINE_CHALLENGE\n";
                        if (send(sock_fd, req, strlen(req), 0) == -1) die("send");
                    }
                    pending_host[0] = '\0';
                    pending_port[0] = '\0';
                }
            } else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
                printf("Closing connection.\n");
                break;
            } else if (command[0] != '\0') {
                printf("Unknown command '%s'. Use 'peers' or 'quit'.\n", command);
            }
        }
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
