#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>  
#include <ctime>    
#include <SDL2/SDL_ttf.h>

using namespace std;


struct Pipe {
    SDL_Rect topPipe;
    SDL_Rect bottomPipe;
    int gap;
    bool passed;
};

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return SDL_HasIntersection(&a, &b);
}


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Flappy Bird",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
    std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
    return 1;
    }

    TTF_Font* font = TTF_OpenFont("D:/flappybird/fonts/ARLRDBD.TTF", 24); // Use your actual font path
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }

    bool running = true;
    SDL_Event event;
    int score = 0;

    //bird
    SDL_Rect bird;
    bird.x = 100;
    bird.y = 250;
    bird.w = 34;
    bird.h = 24;

    //pipes
    vector<Pipe> pipes;
    const int pipeCount = 3;         // Number of pipe pairs on screen
    const int pipeSpacing = 300; 

   srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < pipeCount; i++) {
        Pipe p;
        p.gap = 150;
        p.topPipe.w = 80;
        p.topPipe.x = 800 + i * pipeSpacing;

        // Randomize height between 100 and 350 (adjust to fit your screen)
        p.topPipe.h = 100 + rand() % 251; 
        p.topPipe.y = 0;

        p.bottomPipe.w = 80;
        p.bottomPipe.x = p.topPipe.x;
        p.bottomPipe.y = p.topPipe.h + p.gap;
        p.bottomPipe.h = 600 - p.bottomPipe.y;

        pipes.push_back(p);
    }

    //movement
    float velocity = 0.0f;
    const float gravity = 0.5f;
    const float jumpStrength = -8.0f;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) running = false;

            if (event.key.keysym.sym == SDLK_SPACE) {
                velocity = jumpStrength;
            }
        }

        //bird jumping
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

        //pipes
        const int pipeSpeed = 5;

        for (Pipe &p : pipes) {
            p.topPipe.x -= pipeSpeed;
            p.bottomPipe.x -= pipeSpeed;

            if (p.topPipe.x + p.topPipe.w < 0) {
                p.topPipe.x = 800 + (pipeCount - 1) * pipeSpacing;
                p.passed = false;

                // Randomize height again
                p.topPipe.h = 100 + rand() % 251;
                p.bottomPipe.x = p.topPipe.x;
                p.bottomPipe.y = p.topPipe.h + p.gap;
                p.bottomPipe.h = 600 - p.bottomPipe.y;
            }
        }  
        //score update
        for (Pipe& p : pipes) {
            if (!p.passed && p.topPipe.x + p.topPipe.w < bird.x) {
                score++;
                p.passed = true;
                cout << "Score: " << score << endl;
            }
        }

                // Check for collision with pipes
        for (const Pipe &p : pipes) {
            if (checkCollision(bird, p.topPipe) || checkCollision(bird, p.bottomPipe)) {
                cout << "Game Over: Bird hit a pipe!" << endl;
                running = false;
            }
        }

        // Check for collision with top or bottom of screen
        if (bird.y < 0 || bird.y + bird.h > 600) {
            cout << "Game Over: Bird went out of bounds!" << endl;
            running = false;
        }

        //rendering
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);  // sky blue
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255, 255};

        //Draw bird
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &bird);
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);

        //draw pipes
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (const Pipe &p : pipes) {
            SDL_RenderFillRect(renderer, &p.topPipe);
            SDL_RenderFillRect(renderer, &p.bottomPipe);
        }
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);

        //score on screen
        string scoreText = "Score: " + to_string(score);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect;
        textRect.x = 10;
        textRect.y = 10;
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        SDL_RenderPresent(renderer);

        if (!running) {
            SDL_Color red = {255, 0, 0, 255};
            string gameOverText = "Game Over! Final Score: " + to_string(score);

            SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, gameOverText.c_str(), red);
            SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);

            SDL_Rect gameOverRect;
            gameOverRect.x = 200;
            gameOverRect.y = 250;
            gameOverRect.w = gameOverSurface->w;
            gameOverRect.h = gameOverSurface->h;

            SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
            SDL_RenderPresent(renderer);

            SDL_Delay(3000); // Wait 3 seconds before exiting

            SDL_FreeSurface(gameOverSurface);
            SDL_DestroyTexture(gameOverTexture);
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
