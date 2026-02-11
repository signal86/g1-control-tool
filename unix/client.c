#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT 10555
#define SERVERIP "192.168.0.66"
#define DEBUG false

void add_text(SDL_Texture ***text_buffer, int *text_buf_size, char *newtext,
              TTF_Font **font, SDL_Renderer **renderer, SDL_Color c) {
    SDL_Surface *t = TTF_RenderText_Blended(*font, newtext, c);
    if (!t) {
        printf("text surface error: %s\n", TTF_GetError());
        TTF_CloseFont(*font);
        SDL_DestroyRenderer(*renderer);
        // just gotta pray cause no window passed in and i dont really wanna
        // bother w it
        TTF_Quit();
        SDL_Quit();
        exit(1);
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(*renderer, t);
    SDL_FreeSurface(t);

    *text_buffer =
        realloc(*text_buffer, sizeof(SDL_Texture *) * ++(*text_buf_size));

    (*text_buffer)[(*text_buf_size) - 1] = texture;
}

long long clock_now() {
    struct timespec s;
    clock_gettime(CLOCK_MONOTONIC, &s);
    return (long long)s.tv_sec * 1000LL + s.tv_nsec / 1000000LL;
}

// QWEASD
void movement(bool *control_keys, int *client_fd) {
    // !!!
    //
    // change MAX if more movement commands are added
    // formula is:
    // (amount of commands - 1) spaces + length of all commands individually +
    // the null terminator
    // for the current six commands: 5 spaces + 35 individual
    // chars in the motion_set + null terminator = 41
    //
    // or, you could just overallocate it... null terminator automatically
    // concatenates the socket message anyway, so if you don't want to do the
    // math and add a new command or are confused, just increase MAX by however
    // many bytes
    //
    // In any case, the memory allocation in this entire function isn't truly
    // safe so make sure your machine has memory available for the requests,
    // which it should, since the allocations arent even remotely close to a
    // kilobyte
    //
    // !!!
    const int MAX = 41;

    static bool init_flag = false;
    static long long last_send;
    static bool msgwait = false; // false = 0.1s cooldown, true = 1s cooldown
    // ^ true if a motion occurs, false is no motion occurs

    // printf("%llu\n", last_send);

    if (!init_flag)
        init_flag = true;
    else {
        // exit out if:
        // msgwait is true and 1000 ms havent passed
        // msgwait is false and 100 ms havent passed
        if ((clock_now() - last_send < 1000 && msgwait) ||
            (clock_now() - last_send < 100 && !msgwait)) {
            return;
        }
    }

    int s = 0;
    char **motion_set = malloc(s * sizeof(char *));
    // W + !S = forward
    if (control_keys[1] && !control_keys[4]) {
        motion_set = realloc(motion_set, sizeof(char *) * ++s);
        motion_set[s - 1] = "forward";
    }
    // S + !W = back
    if (control_keys[4] && !control_keys[1]) {
        motion_set = realloc(motion_set, sizeof(char *) * ++s);
        motion_set[s - 1] = "back";
    }
    // Q + !E = rotleft
    if (control_keys[0] && !control_keys[2]) {
        motion_set = realloc(motion_set, sizeof(char *) * ++s);
        motion_set[s - 1] = "rotleft";
    }
    // E + !Q = rotright
    if (control_keys[2] && !control_keys[0]) {
        motion_set = realloc(motion_set, sizeof(char *) * ++s);
        motion_set[s - 1] = "rotright";
    }
    // A + !D = left
    if (control_keys[3] && !control_keys[5]) {
        motion_set = realloc(motion_set, sizeof(char *) * ++s);
        motion_set[s - 1] = "left";
    }
    // D + !A = right
    if (control_keys[5] && !control_keys[3]) {
        motion_set = realloc(motion_set, sizeof(char *) * ++s);
        motion_set[s - 1] = "right";
    }

    char *msg_raw =
        malloc(MAX); // !! Not dynamically calculated for readability
    int indice = 0;
    for (int i = 0; i < s; i++) {
        for (int v = 0; v < strlen(motion_set[i]); v++) {
            msg_raw[indice++] = motion_set[i][v];
        }
        msg_raw[indice++] = ' ';
    }
    msg_raw[indice] = '\0'; // null terminator
    free(motion_set);

    if (strlen(msg_raw) > 0) {
        send(*client_fd, msg_raw, strlen(msg_raw), 0);
        printf("Movement command sent: %s\n", msg_raw);
        msgwait = true;
    } else {
        msgwait = false;
    }
    free(msg_raw);

    last_send = clock_now();
}

bool between(int i, int min, int max) {
    return (i >= min && i <= max ? true : false);
}

int main(int argc, char *argv[]) {
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char sock_buffer[1024] = {0};
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket failed!!\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, DEBUG ? "127.0.0.1" : SERVERIP,
                  &serv_addr.sin_addr) <= 0) {
        printf("Invalid/unsupported ip address\n");
        return 1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0) {
        printf("conn failed\n");
        return 1;
    }

    int text_buf_size = 0;
    SDL_Texture **text = malloc(0);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("sdl error: %s\n", SDL_GetError());
        free(text);
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("ttl error: %s\n", TTF_GetError());
        SDL_Quit();
        free(text);
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("MST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         1000, 1000, SDL_WINDOW_FULLSCREEN);

    if (!window) {
        printf("window not created: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        free(text);
        return 1;
    }

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("renderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        free(text);
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("./fonts/SpaceMono-Regular.ttf", 48);
    if (!font) {
        printf("font error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        free(text);
        return 1;
    }

    TTF_Font *font_small = TTF_OpenFont("./fonts/SpaceMono-Regular.ttf", 20);
    if (!font_small) {
        printf("smaller font error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        free(text);
        return 1;
    }

    bool running = true;
    SDL_Event event;

    SDL_Color c0 = {255, 255, 255};
    SDL_Color c1 = {0, 255, 255};

    // up
    add_text(&text, &text_buf_size, "Q", &font, &renderer, c0);
    add_text(&text, &text_buf_size, "W", &font, &renderer, c0);
    add_text(&text, &text_buf_size, "E", &font, &renderer, c0);
    add_text(&text, &text_buf_size, "A", &font, &renderer, c0);
    add_text(&text, &text_buf_size, "S", &font, &renderer, c0);
    add_text(&text, &text_buf_size, "D", &font, &renderer, c0);

    // down
    add_text(&text, &text_buf_size, "Q", &font, &renderer, c1);
    add_text(&text, &text_buf_size, "W", &font, &renderer, c1);
    add_text(&text, &text_buf_size, "E", &font, &renderer, c1);
    add_text(&text, &text_buf_size, "A", &font, &renderer, c1);
    add_text(&text, &text_buf_size, "S", &font, &renderer, c1);
    add_text(&text, &text_buf_size, "D", &font, &renderer, c1);

    int smaller_text_buf_size = 0;
    SDL_Texture **smaller_text = malloc(0);

    char *arm_list[] = {
        "high wave",     "face wave", "shake hand",    "high five",
        "hug",           "clap",      "left kiss",     "right kiss",
        "two-hand kiss", "reject",    "right hand up", "x-ray",
        "hands up",      "heart",     "right heart",   "release arm"};

    int arm_list_len = sizeof(arm_list) / sizeof(arm_list[0]);
    for (int i = 0; i < arm_list_len; i++) {
        add_text(&smaller_text, &smaller_text_buf_size, arm_list[i],
                 &font_small, &renderer, c0);
    }

    int r = 0;
    int g = 0;
    int b = 0;

    // keypresses
    bool control_keys[6] = {false, false, false, false, false, false};

    while (running) {
        while (SDL_PollEvent(&event)) {
            const char *k;
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                k = SDL_GetKeyName(event.key.keysym.sym);
                printf("Key down: %s\n", k);
                if (!strcmp(k, "Q"))
                    control_keys[0] = true;
                else if (!strcmp(k, "W"))
                    control_keys[1] = true;
                else if (!strcmp(k, "E"))
                    control_keys[2] = true;
                else if (!strcmp(k, "A"))
                    control_keys[3] = true;
                else if (!strcmp(k, "S"))
                    control_keys[4] = true;
                else if (!strcmp(k, "D"))
                    control_keys[5] = true;
                else if (!strcmp(k, "\\"))
                    running = false;
                break;
            case SDL_KEYUP:
                k = SDL_GetKeyName(event.key.keysym.sym);
                printf("Key up: %s\n", k);
                if (!strcmp(k, "Q"))
                    control_keys[0] = false;
                else if (!strcmp(k, "W"))
                    control_keys[1] = false;
                else if (!strcmp(k, "E"))
                    control_keys[2] = false;
                else if (!strcmp(k, "A"))
                    control_keys[3] = false;
                else if (!strcmp(k, "S"))
                    control_keys[4] = false;
                else if (!strcmp(k, "D"))
                    control_keys[5] = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_RIGHT)
                    break;
                printf("%d, %d\n", event.button.x, event.button.y);
                // commands
                for (int i = 0; i < arm_list_len; i++) {
                    if (between(event.button.x, 960, 1760) &&
                        between(event.button.y, 360 + (i * 40),
                                394 + (i * 40))) {
                        printf("command: %s\n", arm_list[i]);
                        char tmp[100] = "command ";
                        char *msg_raw = strcat(tmp, arm_list[i]);
                        send(client_fd, msg_raw, strlen(msg_raw), 0);
                    }
                }
                // rgb
                if (between(event.button.x, 80, 590)) {
                    for (int i = 0; i < 3; i++) {
                        if (between(event.button.y, 210 + (90 * i),
                                    (210 + 90 * (i + 1)))) {
                            int color = (event.button.x - 80) / 2;
                            switch (i) {
                            case 0:
                                r = 255 - color;
                                break;
                            case 1:
                                g = 255 - color;
                                break;
                            case 2:
                                b = 255 - color;
                                break;
                            }
                            printf("updated rgb: (%d, %d, %d)\n", r, g, b);
                            char cmd[20];
                            sprintf(cmd, "led %d %d %d", r, g, b);
                            send(client_fd, cmd, strlen(cmd), 0);
                        }
                    }
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // boxes for WASDQE
        for (int y = 0; y < 2; y++) {
            for (int x = 0; x < 3; x++) {
                SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
                SDL_Rect a = {80 * (x + 1), 720 + (y * 80), 80, 80};
                SDL_RenderFillRect(renderer, &a);
                if (!control_keys[x + (y * 2) + y])
                    SDL_RenderCopy(renderer, text[x + (y * 2) + y], NULL, &a);
                else
                    SDL_RenderCopy(renderer, text[x + (y * 2) + y + 6], NULL,
                                   &a);
            }
        }

        // walls: WASDQE
        for (int y = 0; y < 3; y++) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(
                renderer, &(SDL_Rect){75, 720 + (y * 80) - 5, 80 * 3 + 10, 10});
        }
        for (int x = 0; x < 4; x++) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(
                renderer, &(SDL_Rect){80 * (x + 1) - 5, 715, 10, 80 * 2 + 10});
        }

        // commands
        for (int i = 0; i < arm_list_len; i++) {
            SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
            SDL_Rect a = {960, 360 + (40 * i), 800, 40};
            SDL_RenderFillRect(renderer, &a);

            SDL_Rect wall1 = {955, 355 + (40 * i), 810, 5};
            SDL_Rect wall2 = {955, 355 + (40 * (i + 1)), 810, 5};
            SDL_Rect wall3 = {955, 355 + (40 * i), 5, 45};
            SDL_Rect wall4 = {955 + 805, 355 + (40 * i), 5, 45};
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(renderer, &wall1);
            SDL_RenderFillRect(renderer, &wall2);
            SDL_RenderFillRect(renderer, &wall3);
            SDL_RenderFillRect(renderer, &wall4);

            SDL_Rect b = {960, 360 + (40 * i) + 5, strlen(arm_list[i]) * 20,
                          30};
            SDL_RenderCopy(renderer, smaller_text[i], NULL, &b);
        }

        // rgb

        // Initial box
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_Rect initial_rgb_box = {80, 120, 510, 80};
        SDL_RenderFillRect(renderer, &initial_rgb_box);
        // -- wall
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_Rect rgb_box_wall1 = {75, 115, 520, 10};
        SDL_Rect rgb_box_wall2 = {75, 115, 10, 90};
        SDL_Rect rgb_box_wall3 = {75, 195, 520, 10};
        SDL_Rect rgb_box_wall4 = {585, 115, 10, 90};
        SDL_RenderFillRect(renderer, &rgb_box_wall1);
        SDL_RenderFillRect(renderer, &rgb_box_wall2);
        SDL_RenderFillRect(renderer, &rgb_box_wall3);
        SDL_RenderFillRect(renderer, &rgb_box_wall4);
        // sliders
        int cd = 255;
        for (int x = 80; x < 590; x += 2) {
            SDL_SetRenderDrawColor(renderer, cd, 0, 0, 255);
            cd--;
            SDL_Rect red = {x, 210, 2, 90};
            SDL_RenderFillRect(renderer, &red);
        }
        cd = 255;
        for (int x = 80; x < 590; x += 2) {
            SDL_SetRenderDrawColor(renderer, 0, cd, 0, 255);
            cd--;
            SDL_Rect green = {x, 300, 2, 90};
            SDL_RenderFillRect(renderer, &green);
        }
        cd = 255;
        for (int x = 80; x < 590; x += 2) {
            SDL_SetRenderDrawColor(renderer, 0, 0, cd, 255);
            cd--;
            SDL_Rect blue = {x, 390, 2, 90};
            SDL_RenderFillRect(renderer, &blue);
        }

        SDL_RenderPresent(renderer);

        movement(control_keys, &client_fd);
    }

    close(client_fd);

    for (int i = 0; i < text_buf_size; i++) {
        SDL_DestroyTexture(text[i]);
    }
    TTF_CloseFont(font);
    TTF_CloseFont(font_small);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    free(smaller_text);
    free(text);
    return 0;
}
