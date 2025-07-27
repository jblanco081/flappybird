#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

struct Pipe {
    SDL_Rect topPipe;
    SDL_Rect bottomPipe;
    int gap;
    bool passed;
};

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return SDL_HasIntersection(&a, &b);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }
    if (TTF_Init() == -1) {
        cerr << "TTF_Init Error: " << TTF_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Flappy Bird",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Font* font = TTF_OpenFont("D:/flappybird/fonts/ARLRDBD.TTF", 24);
    if (!font) {
        cerr << "Font load error: " << TTF_GetError() << endl;
        return 1;
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    // --- Game Variables ---
    bool running = true;
    GameState currentState = GameState::MENU;

    SDL_Rect bird = {100, 250, 34, 24};
    float velocity = 0.0f;
    const float gravity = 0.5f;
    const float jumpStrength = -8.0f;

    int score = 0;

    const int pipeCount = 3;
    const int pipeSpacing = 300;
    vector<Pipe> pipes(pipeCount);

    for (int i = 0; i < pipeCount; i++) {
        pipes[i].gap = 150;
        pipes[i].topPipe = {800 + i * pipeSpacing, 0, 80, 200 + rand() % 150};
        pipes[i].bottomPipe = {
            pipes[i].topPipe.x,
            pipes[i].topPipe.h + pipes[i].gap,
            80,
            600 - (pipes[i].topPipe.h + pipes[i].gap)
        };
        pipes[i].passed = false;
    }

    // --- Main Loop ---
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }

            if (currentState == GameState::MENU) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                    currentState = GameState::PLAYING;
                    bird.y = 250;
                    velocity = 0;
                    score = 0;
                }
            }
            else if (currentState == GameState::PLAYING) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                    velocity = jumpStrength;
                    }
                }   else if (currentState == GameState::GAME_OVER) {
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                    currentState = GameState::MENU;
                }
            }
        }

        // --- State Logic ---
        if (currentState == GameState::MENU) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* surface = TTF_RenderText_Solid(font, "Press SPACE to Start", white);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect dest = {200, 250, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &dest);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer);
        }
        else if (currentState == GameState::PLAYING) {
            // --- Bird Physics ---
            velocity += gravity;
            bird.y += static_cast<int>(velocity);

            if (bird.y + bird.h > 600) {
                bird.y = 600 - bird.h;
                velocity = 0;
            }
            if (bird.y < 0) {
                bird.y = 0;
                velocity = 0;
            }

            // --- Pipe Logic ---
            const int pipeSpeed = 5;
            for (auto& p : pipes) {
                p.topPipe.x -= pipeSpeed;
                p.bottomPipe.x -= pipeSpeed;

                if (p.topPipe.x + p.topPipe.w < 0) {
                    p.topPipe.x = 800 + (pipeCount - 1) * pipeSpacing;
                    p.passed = false;
                    //randomize height again
                    p.topPipe.h = 100 + rand() % 251;
                    p.bottomPipe.x = p.topPipe.x;
                    p.bottomPipe.y = p.topPipe.h + p.gap;
                    p.bottomPipe.h = 600 - p.bottomPipe.y;
                }
            }

            // --- Score ---
            for (auto& p : pipes) {
                if (!p.passed && p.topPipe.x + p.topPipe.w < bird.x) {
                    score++;
                    p.passed = true;
                }
            }

            // --- Collision ---
            for (const auto& p : pipes) {
                if (checkCollision(bird, p.topPipe) || checkCollision(bird, p.bottomPipe)) {
                    currentState = GameState::GAME_OVER;
                }
            }
            if (bird.y < 0 || bird.y + bird.h > 600) {
                currentState = GameState::GAME_OVER;
            }

            // --- Render ---
            SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &bird);

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            for (const auto& p : pipes) {
                SDL_RenderFillRect(renderer, &p.topPipe);
                SDL_RenderFillRect(renderer, &p.bottomPipe);
            }

            SDL_Color white = {255, 255, 255, 255};
            string scoreText = "Score: " + to_string(score);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);

            SDL_RenderPresent(renderer);
        }
        else if (currentState == GameState::GAME_OVER) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_Color red = {255, 0, 0, 255};
            string text = "Game Over! Score: " + to_string(score) + " | Press R to Restart";
            SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), red);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect rect = {150, 250, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);

            SDL_RenderPresent(renderer);
        }

        SDL_Delay(16); // ~60 FPS
    }

    // --- Cleanup ---
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
