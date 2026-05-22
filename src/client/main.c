#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <errno.h>
#include <locale.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <poll.h>

#include "app_info.h"
#define move board_move
#include "board.h"
#undef move
#include <ncurses.h>

#define BUFFER_SIZE 256
#define MAX_PEERS 32
#define MAX_LOG_LINES 100

static char self_host[64] = {0};
static char self_port[16] = {0};
static char peers[MAX_PEERS][80];
static int peer_count = 0;
static bool show_incoming_prompt = false;
static char pending_from[64] = {0};
static char pending_port[16] = {0};
static bool game_started = false;
static char game_color[8] = {0};
static char input_buffer[BUFFER_SIZE] = {0};
static int input_pos = 0;
static char log_lines[MAX_LOG_LINES][80];
static int log_count = 0;
static int selected_src_row = -1;
static int selected_src_col = -1;
static int selected_dst_row = -1;
static int selected_dst_col = -1;
static WINDOW *peer_win;
static WINDOW *board_win;
static WINDOW *log_win;
static WINDOW *input_win;
static Board board;
static BoardPosition pending_move_from;
static BoardPosition pending_move_to;
static bool pending_move_valid = false;

static void die(const char *message)
{
    endwin();
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

static void add_log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char line[80];
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    if (log_count < MAX_LOG_LINES) {
        strncpy(log_lines[log_count++], line, sizeof(log_lines[0]) - 1);
        log_lines[log_count - 1][sizeof(log_lines[0]) - 1] = '\0';
    } else {
        memmove(log_lines, log_lines + 1, sizeof(log_lines[0]) * (MAX_LOG_LINES - 1));
        strncpy(log_lines[MAX_LOG_LINES - 1], line, sizeof(log_lines[0]) - 1);
        log_lines[MAX_LOG_LINES - 1][sizeof(log_lines[0]) - 1] = '\0';
    }
}

static char get_piece_letter(Piece piece);

static void draw_peers(void)
{
    werase(peer_win);
    box(peer_win, 0, 0);
    mvwprintw(peer_win, 1, 2, "Self: %s:%s", self_host, self_port);
    mvwprintw(peer_win, 2, 2, "Peers:");
    for (int i = 0; i < peer_count && i < 8; i++) {
        mvwprintw(peer_win, 4 + i, 2, "%s", peers[i]);
    }
    if (peer_count == 0) {
        mvwprintw(peer_win, 4, 2, "(no peers available)");
    }
    if (show_incoming_prompt) {
        mvwprintw(peer_win, 13, 2, "Incoming challenge from %s:%s", pending_from, pending_port);
        mvwprintw(peer_win, 14, 2, "Type yes or no");
    }
    wnoutrefresh(peer_win);
}

static void draw_board(void)
{
    werase(board_win);
    box(board_win, 0, 0);
    mvwprintw(board_win, 1, 2, "Game: %s", game_started ? game_color : "waiting");
    int row_start = 0;
    int row_step = 1;
    if (game_started && strcmp(game_color, "BLACK") == 0) {
        row_start = 7;
        row_step = -1;
    }
    for (int r = 0; r < 8; r++) {
        int board_row = row_start + r * row_step;
        mvwprintw(board_win, 3 + r, 1, "%d", board_row);
        for (int c = 0; c < 8; c++) {
            int x = 3 + c * 3;
            bool dark = ((board_row + c) % 2) == 1;
            bool selected = (board_row == selected_src_row && c == selected_src_col) ||
                            (board_row == selected_dst_row && c == selected_dst_col);
            if (selected) {
                wattron(board_win, A_REVERSE);
            }
            Piece piece = board.squares[board_row][c];
            char display[4] = {' ', ' ', ' ', '\0'};
            char letter = get_piece_letter(piece);
            if (letter != ' ') {
                display[1] = letter;
            } else {
                display[1] = dark ? '#' : '.';
            }
            mvwprintw(board_win, 3 + r, x, "%s", display);
            wattroff(board_win, A_REVERSE);
        }
    }
    mvwprintw(board_win, 12, 3, " 0  1  2  3  4  5  6  7");
    wnoutrefresh(board_win);
}

static void draw_log(void)
{
    werase(log_win);
    box(log_win, 0, 0);
    mvwprintw(log_win, 1, 2, "Messages:");
    int rows = getmaxy(log_win) - 3;
    int start = log_count > rows ? log_count - rows : 0;
    for (int i = 0; i < rows && start + i < log_count; i++) {
        mvwprintw(log_win, 2 + i, 2, "%s", log_lines[start + i]);
    }
    wnoutrefresh(log_win);
}

static void draw_input(void)
{
    if (has_colors()) {
        wattron(input_win, COLOR_PAIR(1));
        wbkgd(input_win, COLOR_PAIR(1));
    }
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 2, "> %s", input_buffer);
    wmove(input_win, 1, 4 + input_pos);
    curs_set(1);
    if (has_colors()) {
        wattroff(input_win, COLOR_PAIR(1));
    }
    wnoutrefresh(input_win);
}

static void render_screen(void)
{
    draw_peers();
    draw_board();
    draw_log();
    draw_input();
    doupdate();
}

static void update_peer_list(const char *message)
{
    peer_count = 0;
    const char *line = strchr(message, '\n');
    if (line == NULL) {
        return;
    }
    line++;
    while (*line != '\0') {
        if (strncmp(line, "No other connections.", 20) == 0) {
            peer_count = 0;
            break;
        }
        if (*line == '-') {
            char peer_line[80];
            if (sscanf(line, "- %79[^\n]", peer_line) == 1) {
                if (peer_count < MAX_PEERS) {
                    strncpy(peers[peer_count], peer_line, sizeof(peers[peer_count]) - 1);
                    peers[peer_count][sizeof(peers[peer_count]) - 1] = '\0';
                    peer_count++;
                }
            }
        }
        const char *next = strchr(line, '\n');
        if (!next) {
            break;
        }
        line = next + 1;
    }
}

static void clear_input(void)
{
    input_buffer[0] = '\0';
    input_pos = 0;
}

static void show_help(void)
{
    add_log("Commands:");
    add_log("  h | help                 Show this help message");
    add_log("  peers                    Request current peer list from server");
    add_log("  challenge <host> <port>  Send a challenge to another client");
    add_log("  yes | no                 Accept or decline an incoming challenge");
    add_log("  move <r1> <c1> <r2> <c2>  Send a move during a game");
    add_log("  quit | exit              Exit the client");
    add_log("Examples:");
    add_log("  peers");
    add_log("  challenge 192.168.1.10 8080");
    add_log("  move 1 4 3 4");
}

static char get_piece_letter(Piece piece)
{
    if (piece.type == PIECE_NONE || piece.color == COLOR_NONE) {
        return ' ';
    }

    char letter;
    switch (piece.type) {
        case PIECE_PAWN:   letter = 'P'; break;
        case PIECE_ROOK:   letter = 'R'; break;
        case PIECE_KNIGHT: letter = 'N'; break;
        case PIECE_BISHOP: letter = 'B'; break;
        case PIECE_QUEEN:  letter = 'Q'; break;
        case PIECE_KING:   letter = 'K'; break;
        default:           letter = '?'; break;
    }

    if (piece.color == COLOR_BLACK) {
        letter = (char)tolower((unsigned char)letter);
    }
    return letter;
}

static void reset_client_board(void)
{
    initializeClassicGame(&board);
}

static void apply_board_move(Board *board, Movement mv)
{
    Piece movingPiece = board->squares[mv.from.row][mv.from.col];
    board->squares[mv.to.row][mv.to.col] = movingPiece;
    board->squares[mv.from.row][mv.from.col] = (Piece){PIECE_NONE, COLOR_NONE};
}

static void send_server_command(int sock_fd, const char *command)
{
    if (send(sock_fd, command, strlen(command), 0) == -1) {
        die("send");
    }
}

static bool process_command(int sock_fd, const char *command)
{
    if (strncmp(command, "challenge ", 10) == 0) {
        char t_host[64], t_port[16];
        if (sscanf(command + 10, "%63s %15s", t_host, t_port) == 2) {
            char req[BUFFER_SIZE];
            snprintf(req, sizeof(req), "CHALLENGE %s %s\n", t_host, t_port);
            send_server_command(sock_fd, req);
            add_log("Sent challenge to %s:%s", t_host, t_port);
        } else {
            add_log("Usage: challenge <host> <port>");
        }
    } else if (strncmp(command, "move ", 5) == 0) {
        int r1, c1, r2, c2;
        if (sscanf(command + 5, "%d %d %d %d", &r1, &c1, &r2, &c2) == 4) {
            char req[BUFFER_SIZE];
            snprintf(req, sizeof(req), "MOVE %d %d %d %d\n", r1, c1, r2, c2);
            send_server_command(sock_fd, req);
            pending_move_from.row = r1;
            pending_move_from.col = c1;
            pending_move_to.row = r2;
            pending_move_to.col = c2;
            pending_move_valid = true;
            add_log("Sent move %d %d -> %d %d", r1, c1, r2, c2);
        } else {
            add_log("Usage: move <r1> <c1> <r2> <c2>");
            pending_move_valid = false;
        }
    } else if (strcmp(command, "peers") == 0) {
        send_server_command(sock_fd, "GET_PEERS\n");
        add_log("Requested peers");
    } else if (strcmp(command, "yes") == 0 || strcmp(command, "no") == 0) {
        if (!show_incoming_prompt) {
            add_log("No pending challenge");
        } else {
            if (strcmp(command, "yes") == 0) {
                send_server_command(sock_fd, "ACCEPT_CHALLENGE\n");
                add_log("Accepted challenge");
            } else {
                send_server_command(sock_fd, "DECLINE_CHALLENGE\n");
                add_log("Declined challenge");
            }
            show_incoming_prompt = false;
            pending_from[0] = '\0';
            pending_port[0] = '\0';
        }
    } else if (strcmp(command, "h") == 0 || strcmp(command, "help") == 0) {
        show_help();
    } else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        add_log("Quitting...");
        return true;
    } else if (command[0] != '\0') {
        add_log("Unknown command: %s", command);
    }
    return false;
}

static void handle_click(MEVENT *event)
{
    int win_y, win_x;
    int row;
    int col;

    getbegyx(peer_win, win_y, win_x);
    if (event->y >= win_y + 4 && event->y < win_y + 4 + peer_count && event->x >= win_x + 2) {
        int idx = event->y - (win_y + 4);
        if (idx >= 0 && idx < peer_count) {
            snprintf(input_buffer, sizeof(input_buffer), "challenge %s", peers[idx]);
            input_pos = strlen(input_buffer);
            add_log("Peer clicked: %s", peers[idx]);
            return;
        }
    }

    getbegyx(board_win, win_y, win_x);
    if (event->y >= win_y + 3 && event->y < win_y + 11 && event->x >= win_x + 4) {
        row = event->y - (win_y + 3);
        col = (event->x - (win_x + 4)) / 2;
        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            int board_row = row;
            if (game_started && strcmp(game_color, "BLACK") == 0) {
                board_row = 7 - row;
            }
            if (selected_src_row < 0) {
                selected_src_row = board_row;
                selected_src_col = col;
                add_log("Selected source %d,%d", board_row, col);
            } else {
                selected_dst_row = board_row;
                selected_dst_col = col;
                add_log("Selected target %d,%d", board_row, col);
                snprintf(input_buffer, sizeof(input_buffer), "move %d %d %d %d",
                         selected_src_row, selected_src_col, selected_dst_row, selected_dst_col);
                input_pos = strlen(input_buffer);
                add_log("Prepared move %s", input_buffer);
                selected_src_row = -1;
                selected_src_col = -1;
                selected_dst_row = -1;
                selected_dst_col = -1;
            }
            return;
        }
    }
}

int main(void)
{
    setlocale(LC_ALL, "");
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

    struct sockaddr_storage local_addr;
    socklen_t local_len = sizeof(local_addr);
    if (getsockname(sock_fd, (struct sockaddr *)&local_addr, &local_len) == 0) {
        if (getnameinfo((struct sockaddr *)&local_addr, local_len,
                        self_host, sizeof(self_host), self_port, sizeof(self_port),
                        NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
            strcpy(self_host, "unknown");
            strcpy(self_port, "unknown");
        }
    } else {
        strcpy(self_host, "unknown");
        strcpy(self_port, "unknown");
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(100);
    mousemask(BUTTON1_CLICKED, NULL);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        wbkgd(stdscr, COLOR_PAIR(1));
    }

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);
    (void)term_h;
    peer_win = newwin(18, term_w / 2, 0, 0);
    board_win = newwin(15, term_w - term_w / 2, 0, term_w / 2);
    log_win = newwin(8, term_w, 18, 0);
    input_win = newwin(3, term_w, 26, 0);
    keypad(input_win, TRUE);
    nodelay(input_win, TRUE);

    reset_client_board();
    clear_input();
    add_log("Connected to server %s:%s", host, port_str);

    char buffer[BUFFER_SIZE];
    ssize_t received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (received == -1) {
        die("recv");
    }
    if (received > 0) {
        buffer[received] = '\0';
        add_log("Server: %s", buffer);
    }

    const char *message = "LANChess client: connection established\n";
    if (send(sock_fd, message, strlen(message), 0) == -1) {
        die("send");
    }

    render_screen();

    struct pollfd pfd = {.fd = sock_fd, .events = POLLIN};
    bool should_quit = false;
    MEVENT event;

    while (!should_quit) {
        int ch = wgetch(input_win);
        if (ch != ERR) {
            if (ch == KEY_MOUSE) {
                if (getmouse(&event) == OK) {
                    handle_click(&event);
                }
            } else if (ch == '\n' || ch == KEY_ENTER) {
                if (input_buffer[0] != '\0') {
                    should_quit = process_command(sock_fd, input_buffer);
                    clear_input();
                }
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                if (input_pos > 0) {
                    input_pos--;
                    input_buffer[input_pos] = '\0';
                }
            } else if (isprint(ch) && input_pos < BUFFER_SIZE - 2) {
                input_buffer[input_pos++] = (char)ch;
                input_buffer[input_pos] = '\0';
            }
        }

        if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
            received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) {
                if (received == 0) {
                    add_log("Server closed the connection.");
                } else {
                    add_log("Server recv error: %s", strerror(errno));
                }
                break;
            }
            buffer[received] = '\0';

            if (strncmp(buffer, "PEER_UPDATE", 11) == 0) {
                update_peer_list(buffer);
                show_incoming_prompt = false;
                add_log("Peer list updated");
            } else if (strncmp(buffer, "INCOMING_CHALLENGE ", 19) == 0) {
                char ch_host[64], ch_port[16];
                if (sscanf(buffer + 19, "%63s %15s", ch_host, ch_port) == 2) {
                    strncpy(pending_from, ch_host, sizeof(pending_from));
                    pending_from[sizeof(pending_from) - 1] = '\0';
                    strncpy(pending_port, ch_port, sizeof(pending_port));
                    pending_port[sizeof(pending_port) - 1] = '\0';
                    show_incoming_prompt = true;
                    add_log("Incoming challenge from %s:%s", ch_host, ch_port);
                } else {
                    add_log("Malformed incoming challenge: %s", buffer);
                }
            } else if (strncmp(buffer, "START_GAME", 10) == 0) {
                if (strncmp(buffer, "START_GAME WHITE", 16) == 0) {
                    strcpy(game_color, "WHITE");
                } else if (strncmp(buffer, "START_GAME BLACK", 16) == 0) {
                    strcpy(game_color, "BLACK");
                }
                reset_client_board();
                game_started = true;
                show_incoming_prompt = false;
                add_log("Game started as %s", game_color);
            } else if (strncmp(buffer, "MOVE_OK", 7) == 0) {
                if (pending_move_valid) {
                    Movement mv = {.from = pending_move_from, .to = pending_move_to};
                    apply_board_move(&board, mv);
                    pending_move_valid = false;
                    add_log("Move accepted");
                } else {
                    add_log("Server: %s", buffer);
                }
            } else if (strncmp(buffer, "ILLEGAL_MOVE", 12) == 0 ||
                       strncmp(buffer, "NOT_YOUR_TURN", 13) == 0 ||
                       strncmp(buffer, "INVALID_MOVE_FORMAT", 19) == 0 ||
                       strncmp(buffer, "NOT_IN_GAME", 11) == 0) {
                pending_move_valid = false;
                add_log("Server: %s", buffer);
            } else if (strncmp(buffer, "OPPONENT_MOVE ", 14) == 0) {
                int r1, c1, r2, c2;
                if (sscanf(buffer + 14, "%d %d %d %d", &r1, &c1, &r2, &c2) == 4) {
                    Movement mv = {.from = {.row = r1, .col = c1}, .to = {.row = r2, .col = c2}};
                    apply_board_move(&board, mv);
                    add_log("Opponent moved %d %d -> %d %d", r1, c1, r2, c2);
                } else {
                    add_log("Server: %s", buffer);
                }
            } else if (strncmp(buffer, "GAME_END", 8) == 0 ||
                       strncmp(buffer, "WINNER:", 7) == 0) {
                add_log("%s", buffer);
            } else {
                add_log("Server: %s", buffer);
            }
        }

        render_screen();
    }

    endwin();
    close(sock_fd);
    return EXIT_SUCCESS;
}
