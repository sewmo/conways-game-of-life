#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Scroll wheel to change delta time
// Arrow keys to move through updates manually

#define WINDOW_TITLE "Conway's Game of Life"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define CELL_SIZE_PX 10
#define FONT_PATH "assets/fonts/Roboto-Bold.ttf"

typedef struct {
    bool* backBuffer;
    bool* frontBuffer;
    size_t width;
    size_t height;
} Grid;

typedef struct {
    SDL_Rect box;
    SDL_Color color;
    SDL_Surface* surface;
    SDL_Texture* texture;
    char buffer[64];
} Text;

typedef enum { SPACE_BAR, LEFT_CLICK, ARROW_UP, ARROW_DOWN, CLEAR } Actions;

int initSDL(SDL_Window** window, SDL_Renderer** renderer);
void updateText(Text* text, const char* newText, TTF_Font* font, SDL_Renderer* renderer);
void freeText(Text* text);
void cleanupSDL(SDL_Window** window, SDL_Renderer** renderer);
void pollInput(SDL_Event* event, bool* running, bool* actions);
void initGrid(Grid* grid);
void renderGrid(SDL_Renderer* renderer, Grid* grid);
void updateGrid(Grid* grid);
void clearGrid(Grid* grid);
void swapGridBuffers(Grid* grid);
void drawGridLines(SDL_Renderer* renderer, Grid* grid);
int getLiveNeighbors(Grid* grid, int cell);

int main(int argc, char** argv) {

    srand(time(NULL));

    SDL_Window* window;
    SDL_Renderer* renderer;

    if (initSDL(&window, &renderer) == EXIT_FAILURE) {
        fprintf(stderr, "ERROR: Failed in initializing SDL2 resources!\n");
        return EXIT_FAILURE;
    }

    SDL_Event event;
    bool running = true;
    bool started = false;
    bool actions[5] = {false};

    Grid grid;

    int mouseX, mouseY;
    int iterationCount = 0;
    int deltaTime = 100;

    initGrid(&grid);

    TTF_Font* textFont = TTF_OpenFont(FONT_PATH, 24);
    Text timeText, iterationText, statusText;

    snprintf(timeText.buffer, sizeof(timeText.buffer), "Time in between updates: %dms", deltaTime);
    timeText.color = (SDL_Color){255, 255, 255, 255};
    timeText.surface = TTF_RenderText_Solid(textFont, timeText.buffer, timeText.color);
    timeText.texture = SDL_CreateTextureFromSurface(renderer, timeText.surface);
    timeText.box = (SDL_Rect){10, 10, timeText.surface->w, timeText.surface->h};

    snprintf(iterationText.buffer, sizeof(iterationText.buffer), "Iteration count: 0");
    iterationText.color = (SDL_Color){255, 255, 255, 255};
    iterationText.surface = TTF_RenderText_Solid(textFont, iterationText.buffer, iterationText.color);
    iterationText.texture = SDL_CreateTextureFromSurface(renderer, iterationText.surface);
    iterationText.box = (SDL_Rect){10, 40, iterationText.surface->w, iterationText.surface->h};

    snprintf(statusText.buffer, sizeof(statusText.buffer), "Status: Simulation Off");
    statusText.color = (SDL_Color){255, 255, 255, 255};
    statusText.surface = TTF_RenderText_Solid(textFont, statusText.buffer, statusText.color);
    statusText.texture = SDL_CreateTextureFromSurface(renderer, statusText.surface);
    statusText.box = (SDL_Rect){10, 70, statusText.surface->w, statusText.surface->h};

    while (running) {
        pollInput(&event, &running, actions);

        if (actions[LEFT_CLICK] && !started) {
            printf("INFO: Left click pressed!\n");
            SDL_GetMouseState(&mouseX, &mouseY);
            int pos = mouseX / CELL_SIZE_PX + (mouseY / CELL_SIZE_PX) * grid.width;
            grid.backBuffer[pos] = !grid.backBuffer[pos];
            grid.frontBuffer[pos] = !grid.frontBuffer[pos];
            actions[LEFT_CLICK] = false;
        }
        if (actions[SPACE_BAR]) {
            if (started) {
                started = false;
                printf("INFO: Game of life stopped!\n");
                updateText(&statusText, "Simulation: Off", textFont, renderer);
            } else {
                started = true;
                printf("INFO: Game of life started!\n");
                updateText(&statusText, "Simulation: On", textFont, renderer);
            }
            actions[SPACE_BAR] = false;
        }
        if (actions[CLEAR]) { 
            printf("INFO: Clearing grid buffers!\n"); 
            iterationCount = 0;
            char newText[64];
            snprintf(newText, sizeof(newText), "Iteration count: %d", iterationCount);
            updateText(&iterationText, newText, textFont, renderer);
            clearGrid(&grid);  
            actions[CLEAR] = false; 
        }
        if (actions[ARROW_UP]) { 
            deltaTime += 5; 
            actions[ARROW_UP] = false; 
            char newText[64];
            snprintf(newText, sizeof(newText), "Time in between updates: %dms", deltaTime);
            updateText(&timeText, newText, textFont, renderer);
        }
        if (actions[ARROW_DOWN]) { 
            if (deltaTime >= 5) deltaTime -= 5; 
            actions[ARROW_DOWN] = false; 
            char newText[64];
            snprintf(newText, sizeof(newText), "Time in between updates: %dms", deltaTime);
            updateText(&timeText, newText, textFont, renderer);
        }

        if (started) {
            char newText[64];
            updateGrid(&grid); 
            iterationCount++;
            snprintf(newText, sizeof(newText), "Iteration count: %d", iterationCount);
            updateText(&iterationText, newText, textFont, renderer);   
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderGrid(renderer, &grid);
        SDL_RenderCopy(renderer, timeText.texture, NULL, &timeText.box);
        SDL_RenderCopy(renderer, iterationText.texture, NULL, &iterationText.box);
        SDL_RenderCopy(renderer, statusText.texture, NULL, &statusText.box);
        SDL_RenderPresent(renderer);

        if (started) { swapGridBuffers(&grid); SDL_Delay(deltaTime); }
    }

    free(grid.backBuffer);
    free(grid.frontBuffer);

    freeText(&timeText);
    freeText(&iterationText);
    freeText(&statusText);

    TTF_CloseFont(textFont);

    cleanupSDL(&window, &renderer);

    return EXIT_SUCCESS;
    
}

void clearGrid(Grid* grid) {
    for (int i = 0; i < grid->width * grid->height; i++) {
        grid->frontBuffer[i] = false;
        grid->backBuffer[i] = false;
    }
}

void updateText(Text* text, const char* newText, TTF_Font* font, SDL_Renderer* renderer) {
    memset(text->buffer, '\0', sizeof(text->buffer));
    snprintf(text->buffer, sizeof(text->buffer), newText);
    freeText(text);
    text->surface = TTF_RenderText_Solid(font, text->buffer, text->color);
    text->texture = SDL_CreateTextureFromSurface(renderer, text->surface);
    text->box.w = text->surface->w;
    text->box.h = text->surface->h;
}

void freeText(Text* text) {
    SDL_FreeSurface(text->surface);
    SDL_DestroyTexture(text->texture);
}

void swapGridBuffers(Grid* grid) {
    for (int i = 0; i < grid->width * grid->height; i++) {
        grid->backBuffer[i] = grid->frontBuffer[i];
    }
}

int getLiveNeighbors(Grid* grid, int cell) {
    int aliveNeighbors = 0;
    const int neighborOffsets[] = {-grid->width - 1, -grid->width, -grid->width + 1, -1, 1, grid->width - 1, grid->width, grid->width + 1};
    for (int i = 0; i < 8; i++) {
        int neighboringCell = cell + neighborOffsets[i];
        if (neighboringCell < 0 || neighboringCell >= grid->width * grid->height) continue;
        if (grid->backBuffer[neighboringCell] == true) aliveNeighbors++;
    }
    return aliveNeighbors;
}

void updateGrid(Grid* grid) {
    for (int i = 0; i < grid->width * grid->height; i++) {
        int neighbors = getLiveNeighbors(grid, i);

        if (grid->backBuffer[i] == true) {
            if (neighbors < 2) grid->frontBuffer[i] = false;
            else if (neighbors > 3) grid->frontBuffer[i] = false;
        } else {
            if (neighbors == 3) grid->frontBuffer[i] = true;
        }
    }
}

void renderGrid(SDL_Renderer* renderer, Grid* grid) {
    drawGridLines(renderer, grid);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < grid->width * grid->height; i++) {
        if (grid->frontBuffer[i] == true) {
            if (((i / grid->width) * CELL_SIZE_PX) < 100) continue;
            SDL_Rect cellRect = {(i % grid->width) * CELL_SIZE_PX, (i / grid->width) * CELL_SIZE_PX, CELL_SIZE_PX, CELL_SIZE_PX};
            SDL_RenderFillRect(renderer, &cellRect);   
        }
    }
}

void drawGridLines(SDL_Renderer* renderer, Grid* grid) {
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    for (int i = 100; i < WINDOW_HEIGHT; i += 10) SDL_RenderDrawLine(renderer, 0, i, WINDOW_WIDTH, i);

    for (int i = 0; i < WINDOW_WIDTH; i += 10) SDL_RenderDrawLine(renderer, i, 100, i, WINDOW_HEIGHT);
    
}

void initGrid(Grid* grid) {
    grid->width = WINDOW_WIDTH / CELL_SIZE_PX;
    grid->height = WINDOW_HEIGHT / CELL_SIZE_PX;
    grid->backBuffer = malloc(grid->width * grid->height * sizeof(bool));
    grid->frontBuffer = malloc(grid->width * grid->height * sizeof(bool));

    for (int i = 0; i < grid->width * grid->height; i++) {
        grid->backBuffer[i] = false;
        grid->frontBuffer[i] = false;
    }
}

int initSDL(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "ERROR: SDL_Init - %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() != 0) {
        SDL_Quit();
        fprintf(stderr, "ERROR: TTF_Init - %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if (*window == NULL) {
        fprintf(stderr, "ERROR: SDL_CreateWindow - %s\n", SDL_GetError());
        SDL_Quit(); TTF_Quit();
        return EXIT_FAILURE;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (*renderer == NULL) {
        fprintf(stderr, "ERROR: SDL_CreateRenderer - %s\n", SDL_GetError());
        SDL_Quit(); TTF_Quit(); SDL_DestroyWindow(*window);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void cleanupSDL(SDL_Window** window, SDL_Renderer** renderer) {
    SDL_Quit();
    TTF_Quit();
    SDL_DestroyWindow(*window);
    SDL_DestroyRenderer(*renderer);
}

void pollInput(SDL_Event* event, bool* running, bool* actions) {
    while (SDL_PollEvent(event)) {
        switch ((*event).type) {
            case SDL_QUIT: { 
                *running = false; 
                break; 
            }
            case SDL_KEYDOWN: {
                switch ((*event).key.keysym.sym) {
                    case SDLK_q: { 
                        *running = false; 
                        break; 
                    }
                    case SDLK_SPACE: {
                        actions[SPACE_BAR] = true;
                        break;
                    }
                    case SDLK_UP: {
                        actions[ARROW_UP] = true;
                        break;
                    }
                    case SDLK_DOWN: {
                        actions[ARROW_DOWN] = true;
                        break;
                    }
                }
                break;
            }
            case SDL_KEYUP: { 
                switch ((*event).key.keysym.sym) {
                    case SDLK_SPACE: {
                        actions[SPACE_BAR] = false;
                        break;
                    }
                    case SDLK_UP: {
                        actions[ARROW_UP] = false;
                        break;
                    }
                    case SDLK_DOWN: {
                        actions[ARROW_DOWN] = false;
                        break;
                    }
                }    
                break; 
            }
            case SDL_MOUSEBUTTONDOWN: {
                switch ((*event).button.button) {
                    case SDL_BUTTON_LEFT: {
                        actions[LEFT_CLICK] = true;
                        break;
                    }
                    case SDL_BUTTON_MIDDLE: {
                        actions[CLEAR] = true;
                        break;
                    }
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                switch ((*event).button.button) {
                    case SDL_BUTTON_LEFT: {
                        actions[LEFT_CLICK] = false;
                        break;
                    }
                    case SDL_BUTTON_MIDDLE: {
                        actions[CLEAR] = false;
                        break;
                    }
                }
                break;
            }
        }
    }
}