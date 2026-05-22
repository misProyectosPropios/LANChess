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
#include <time.h>
#include <unistd.h>

#include "app_info.h"
#include "board.h"

#define BACKLOG 10
#define BUFFER_SIZE 512
#define MAX_CLIENTS 32
#define PEER_UPDATE_SECONDS 15

typedef struct {
    int fd;
    char host[64];
    char service[16];
} ClientConnection;

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

static void remove_client(ClientConnection clients[], int *count, int index)
{
    if (index < 0 || index >= *count) {
        return;
    }
    close(clients[index].fd);
    for (int i = index; i < *count - 1; i++) {
        clients[i] = clients[i + 1];
    }
    (*count)--;
}

static int find_client_index_by_fd(const ClientConnection clients[], int count, int fd)
{
    for (int i = 0; i < count; i++) {
        if (clients[i].fd == fd) return i;
    }
    return -1;
}

static int find_client_index_by_hostport(const ClientConnection clients[], int count,
                                         const char *host, const char *service)
{
    struct addrinfo hints;
    struct addrinfo *target_res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, service, &hints, &target_res) != 0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        struct addrinfo *client_res = NULL;
        if (getaddrinfo(clients[i].host, clients[i].service, &hints, &client_res) != 0) {
            continue;
        }

        bool matched = false;
        for (struct addrinfo *ta = target_res; ta != NULL && !matched; ta = ta->ai_next) {
            for (struct addrinfo *ca = client_res; ca != NULL && !matched; ca = ca->ai_next) {
                if (ta->ai_family == ca->ai_family && ta->ai_addrlen == ca->ai_addrlen &&
                    memcmp(ta->ai_addr, ca->ai_addr, ta->ai_addrlen) == 0) {
                    matched = true;
                }
            }
        }

        freeaddrinfo(client_res);

        if (matched) {
            freeaddrinfo(target_res);
            return i;
        }
    }

    freeaddrinfo(target_res);
    return -1;
}

/* Game session management */
#define MAX_GAMES (MAX_CLIENTS / 2)

typedef struct {
    bool active;
    int white_fd;
    int black_fd;
    Board board;
    PieceColor turn;
} GameSession;

static GameSession games[MAX_GAMES];

static int find_game_index_by_fd(int fd)
{
    for (int i = 0; i < MAX_GAMES; i++) {
        if (!games[i].active) continue;
        if (games[i].white_fd == fd || games[i].black_fd == fd) return i;
    }
    return -1;
}

static int create_game(int white_fd, int black_fd)
{
    for (int i = 0; i < MAX_GAMES; i++) {
        if (!games[i].active) {
            games[i].active = true;
            games[i].white_fd = white_fd;
            games[i].black_fd = black_fd;
            initializeClassicGame(&games[i].board);
            games[i].turn = COLOR_WHITE;
            return i;
        }
    }
    return -1;
}

static void end_game(int idx)
{
    if (idx < 0 || idx >= MAX_GAMES) return;
    games[idx].active = false;
}

static void build_peer_list(const ClientConnection clients[], int count, int self_fd,
                            char *buffer, size_t size)
{
    size_t used = 0;
    used += snprintf(buffer + used, size - used, "PEER_UPDATE\n");
    int found_other = 0;
    for (int i = 0; i < count; i++) {
        if (clients[i].fd == self_fd) {
            continue;
        }
        used += snprintf(buffer + used, size - used, "- %s:%s\n",
                         clients[i].host, clients[i].service);
        found_other = 1;
        if (used + 64 >= size) {
            break;
        }
    }
    if (!found_other) {
        snprintf(buffer + used, size - used, "No other connections.\n");
    }
}

static void send_peer_list(int fd, const ClientConnection clients[], int count, int self_fd)
{
    char buffer[BUFFER_SIZE];
    build_peer_list(clients, count, self_fd, buffer, sizeof(buffer));
    if (send(fd, buffer, strlen(buffer), 0) == -1) {
        perror("send peer list");
    }
}

static void broadcast_peer_list(const ClientConnection clients[], int count)
{
    for (int i = 0; i < count; i++) {
        send_peer_list(clients[i].fd, clients, count, clients[i].fd);
    }
}

int main(void)
{
    printf("%s server %s\n", lanchess_name(), lanchess_version());

    char port_str[32];
    printf("Enter port to listen on: ");
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
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, port_str, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    int listen_fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listen_fd == -1) {
            continue;
        }

        int option = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }

        close(listen_fd);
        listen_fd = -1;
    }

    if (listen_fd == -1) {
        fprintf(stderr, "Could not bind to port %s\n", port_str);
        freeaddrinfo(result);
        return EXIT_FAILURE;
    }

    freeaddrinfo(result);

    if (listen(listen_fd, BACKLOG) == -1) {
        die("listen");
    }

    printf("Server listening on port %s...\n", port_str);
    printf("Press Ctrl+C to stop the server.\n");

    ClientConnection clients[MAX_CLIENTS];
    int client_count = 0;
    int pending_challenger[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) pending_challenger[i] = -1;
    time_t last_broadcast = time(NULL);

    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);
        int max_fd = listen_fd;

        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i].fd, &read_fds);
            if (clients[i].fd > max_fd) {
                max_fd = clients[i].fd;
            }
        }

        struct timeval timeout = {1, 0};
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready == -1) {
            if (errno == EINTR) {
                continue;
            }
            die("select");
        }

        time_t now = time(NULL);
        if (now - last_broadcast >= PEER_UPDATE_SECONDS) {
            broadcast_peer_list(clients, client_count);
            last_broadcast = now;
        }

        if (FD_ISSET(listen_fd, &read_fds)) {
            struct sockaddr_storage client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_fd == -1) {
                perror("accept");
            } else if (client_count >= MAX_CLIENTS) {
                const char *busy = "SERVER_BUSY\n";
                send(client_fd, busy, strlen(busy), 0);
                close(client_fd);
                fprintf(stderr, "Rejected connection: max clients reached.\n");
            } else {
                if (getnameinfo((struct sockaddr *)&client_addr, client_addr_len,
                                clients[client_count].host, sizeof(clients[client_count].host),
                                clients[client_count].service, sizeof(clients[client_count].service),
                                NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
                    strcpy(clients[client_count].host, "unknown");
                    strcpy(clients[client_count].service, "unknown");
                }
                clients[client_count].fd = client_fd;
                client_count++;

                printf("Accepted connection from %s:%s\n",
                       clients[client_count - 1].host,
                       clients[client_count - 1].service);

                const char *welcome = "LANChess server: connection established\n";
                if (send(client_fd, welcome, strlen(welcome), 0) == -1) {
                    perror("send");
                }
                const char *usage = "Commands:\n - GET_PEERS\n - CHALLENGE <host> <port>\n - ACCEPT_CHALLENGE\n - DECLINE_CHALLENGE\n - MOVE <r1> <c1> <r2> <c2>\n - quit\n";
                if (send(client_fd, usage, strlen(usage), 0) == -1) {
                    perror("send usage");
                }
                send_peer_list(client_fd, clients, client_count, client_fd);
            }
        }

        for (int i = 0; i < client_count; i++) {
            int fd = clients[i].fd;
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }

            char buffer[BUFFER_SIZE];
            ssize_t received = recv(fd, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) {
                if (received == 0) {
                    printf("Client %s:%s disconnected.\n",
                           clients[i].host, clients[i].service);
                } else {
                    perror("recv");
                }
                remove_client(clients, &client_count, i);
                i--;
                continue;
            }

            buffer[received] = '\0';
            strip_newline(buffer);

            if (strcmp(buffer, "GET_PEERS") == 0) {
                send_peer_list(fd, clients, client_count, fd);
                continue;
            }

            if (strncmp(buffer, "CHALLENGE ", 10) == 0) {
                /* CHALLENGE <host> <port> */
                char target_host[64], target_port[16];
                if (sscanf(buffer + 10, "%63s %15s", target_host, target_port) == 2) {
                    int target_idx = find_client_index_by_hostport(clients, client_count, target_host, target_port);
                    if (target_idx == -1 || clients[target_idx].fd == fd) {
                        const char *fail = "CHALLENGE_FAILED\n";
                        send(fd, fail, strlen(fail), 0);
                    } else {
                        /* forward challenge to target */
                        char msg[BUFFER_SIZE];
                        int src_idx = find_client_index_by_fd(clients, client_count, fd);
                        snprintf(msg, sizeof(msg), "INCOMING_CHALLENGE %s %s\n",
                                 clients[src_idx].host, clients[src_idx].service);
                        if (send(clients[target_idx].fd, msg, strlen(msg), 0) == -1) {
                            perror("send challenge");
                            const char *fail = "CHALLENGE_FAILED\n";
                            send(fd, fail, strlen(fail), 0);
                        } else {
                            pending_challenger[target_idx] = fd;
                            const char *ok = "CHALLENGE_SENT\n";
                            send(fd, ok, strlen(ok), 0);
                        }
                    }
                } else {
                    const char *bad = "INVALID_CHALLENGE\n";
                    send(fd, bad, strlen(bad), 0);
                }
                continue;
            }

            if (strcmp(buffer, "ACCEPT_CHALLENGE") == 0) {
                int target_idx = i;
                int challenger_fd = pending_challenger[target_idx];
                if (challenger_fd == -1) {
                    const char *nope = "NO_PENDING_CHALLENGE\n";
                    send(fd, nope, strlen(nope), 0);
                } else {
                    int challenger_idx = find_client_index_by_fd(clients, client_count, challenger_fd);
                    if (challenger_idx != -1) {
                        char challeng_hostport[128];
                        snprintf(challeng_hostport, sizeof(challeng_hostport), "%s:%s",
                                 clients[challenger_idx].host, clients[challenger_idx].service);
                        char target_hostport[128];
                        snprintf(target_hostport, sizeof(target_hostport), "%s:%s",
                                 clients[target_idx].host, clients[target_idx].service);

                        char msg1[BUFFER_SIZE];
                        char msg2[BUFFER_SIZE];
                        snprintf(msg1, sizeof(msg1), "START_GAME WHITE\nOPPONENT %s\n", target_hostport);
                        snprintf(msg2, sizeof(msg2), "START_GAME BLACK\nOPPONENT %s\n", challeng_hostport);

                        send(challenger_fd, msg1, strlen(msg1), 0);
                        send(fd, msg2, strlen(msg2), 0);
                        /* create game session: challenger is white, target is black */
                        int game_idx = create_game(challenger_fd, fd);
                        if (game_idx == -1) {
                            const char *err = "START_GAME_FAILED\n";
                            send(challenger_fd, err, strlen(err), 0);
                            send(fd, err, strlen(err), 0);
                        }
                    }
                }
                pending_challenger[target_idx] = -1;
                continue;
            }

            if (strncmp(buffer, "MOVE ", 5) == 0) {
                /* MOVE r1 c1 r2 c2 */
                int r1, c1, r2, c2;
                if (sscanf(buffer + 5, "%d %d %d %d", &r1, &c1, &r2, &c2) != 4) {
                    const char *bad = "INVALID_MOVE_FORMAT\n";
                    send(fd, bad, strlen(bad), 0);
                    continue;
                }

                int game_idx = find_game_index_by_fd(fd);
                if (game_idx == -1 || !games[game_idx].active) {
                    const char *notin = "NOT_IN_GAME\n";
                    send(fd, notin, strlen(notin), 0);
                    continue;
                }

                GameSession *g = &games[game_idx];
                PieceColor player_color = (g->white_fd == fd) ? COLOR_WHITE : COLOR_BLACK;
                if (g->turn != player_color) {
                    const char *notturn = "NOT_YOUR_TURN\n";
                    send(fd, notturn, strlen(notturn), 0);
                    continue;
                }

                BoardPosition from = {.row = r1, .col = c1};
                BoardPosition to = {.row = r2, .col = c2};
                Piece source_piece = g->board.squares[from.row][from.col];
                if (source_piece.type == PIECE_NONE || source_piece.color != player_color) {
                    const char *illegal = "ILLEGAL_MOVE\n";
                    send(fd, illegal, strlen(illegal), 0);
                    continue;
                }

                PossiblePositions poss = movements(&g->board, from);
                bool legal = false;
                for (size_t pi = 0; pi < size_BoardPosition(poss.possiblePositions); pi++) {
                    BoardPosition p = get_BoardPosition(poss.possiblePositions, pi);
                    if (p.row == to.row && p.col == to.col) {
                        legal = true;
                        break;
                    }
                }
                destroy_BoardPosition(poss.possiblePositions);

                if (!legal) {
                    const char *illegal = "ILLEGAL_MOVE\n";
                    send(fd, illegal, strlen(illegal), 0);
                    continue;
                }

                Movement mv = {.from = from, .to = to};
                movePiece(&g->board, mv);

                /* flip turn */
                g->turn = (g->turn == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;

                /* notify opponent */
                int opp_fd = (g->white_fd == fd) ? g->black_fd : g->white_fd;
                char okmsg[BUFFER_SIZE];
                snprintf(okmsg, sizeof(okmsg), "OPPONENT_MOVE %d %d %d %d\n", r1, c1, r2, c2);
                send(opp_fd, okmsg, strlen(okmsg), 0);

                const char *ok = "MOVE_OK\n";
                send(fd, ok, strlen(ok), 0);

                PieceColor opponent_color = (player_color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
                if (is_checkmate(&g->board, opponent_color)) {
                    char end_msg[BUFFER_SIZE];
                    const char *winner = player_color == COLOR_WHITE ? "WHITE" : "BLACK";
                    snprintf(end_msg, sizeof(end_msg), "GAME_END\nWINNER: %s\n", winner);
                    send(fd, end_msg, strlen(end_msg), 0);
                    send(opp_fd, end_msg, strlen(end_msg), 0);
                    end_game(game_idx);
                }
                continue;
            }

            if (strcmp(buffer, "DECLINE_CHALLENGE") == 0) {
                int target_idx = i;
                int challenger_fd = pending_challenger[target_idx];
                if (challenger_fd != -1) {
                    const char *decl = "CHALLENGE_DECLINED\n";
                    send(challenger_fd, decl, strlen(decl), 0);
                }
                pending_challenger[target_idx] = -1;
                continue;
            }

            /* default: regular chat/command */
            printf("Client %s:%s says: %s\n",
                   clients[i].host, clients[i].service, buffer);
        }
    }

    close(listen_fd);
    return EXIT_SUCCESS;
}
