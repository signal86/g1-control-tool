#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char *argv[]) {
    int text_buf_size = 0;
    SDL_Texture **text =
        (SDL_Texture **)malloc(text_buf_size * sizeof(SDL_Texture *));

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
        SDL_CreateWindow("SDL C Example", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);

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

    // keypresses
    bool controlKeys[6] = {false, false, false, false, false, false};

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
                    controlKeys[0] = true;
                else if (!strcmp(k, "W"))
                    controlKeys[1] = true;
                else if (!strcmp(k, "E"))
                    controlKeys[2] = true;
                else if (!strcmp(k, "A"))
                    controlKeys[3] = true;
                else if (!strcmp(k, "S"))
                    controlKeys[4] = true;
                else if (!strcmp(k, "D"))
                    controlKeys[5] = true;
                break;
            case SDL_KEYUP:
                k = SDL_GetKeyName(event.key.keysym.sym);
                printf("Key up: %s\n", k);
                if (!strcmp(k, "Q"))
                    controlKeys[0] = false;
                else if (!strcmp(k, "W"))
                    controlKeys[1] = false;
                else if (!strcmp(k, "E"))
                    controlKeys[2] = false;
                else if (!strcmp(k, "A"))
                    controlKeys[3] = false;
                else if (!strcmp(k, "S"))
                    controlKeys[4] = false;
                else if (!strcmp(k, "D"))
                    controlKeys[5] = false;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // boxes for WASDQE
        for (int y = 0; y < 2; y++) {
            for (int x = 0; x < 3; x++) {
                SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
                SDL_Rect a = {80 * (x + 1), 480 + (y * 80), 80, 80};
                SDL_RenderFillRect(renderer, &a);
                if (!controlKeys[x + (y * 2) + y])
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
                renderer, &(SDL_Rect){75, 480 + (y * 80) - 5, 80 * 3 + 10, 10});
        }
        for (int x = 0; x < 4; x++) {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(
                renderer, &(SDL_Rect){80 * (x + 1) - 5, 475, 10, 80 * 2 + 10});
        }

        SDL_RenderPresent(renderer);
    }

    for (int i = 0; i < text_buf_size; i++) {
        SDL_DestroyTexture(text[i]);
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    free(text);
    return 0;
}
