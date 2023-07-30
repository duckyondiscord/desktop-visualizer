#include <iostream>
#include <fftw3.h>
#include <pthread.h>
#include <INIReader.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "input/pulse.h"
#include "input/pulse.cpp"

float bars[64];

void ctrl_c(int s)
{
    printf("Quitting...\n");
    exit(0);
}

void draw(SDL_Renderer *rend, SDL_Window *win)
{
    // For the window
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(rend);

    // For the fucking rectangles
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_SetWindowOpacity(win, 0.0f);
    for (int i = 0; i < 64; i++)
    {
        float bar = bars[i];
        SDL_Rect rect;
        rect.h = bar * 256;
        rect.w = 10;
        rect.x = i * 15 + 20;
        rect.y = 100;
        SDL_RenderFillRect(rend, &rect);
    }
}

int main()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = ctrl_c;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window *win = SDL_CreateWindow("Visualino", // creates a window
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       800, 600, SDL_WINDOW_BORDERLESS);
    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    // creates a renderer to render our images
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, render_flags);
    SDL_SetHint("_NET_WM_WINDOW_OPACITY", "0.5");
    // controls animation loop
    int close = 0;

    pthread_t inputThread;
    int i, thr_id, silence, sleep;
    struct timespec req = {.tv_sec = 0, .tv_nsec = 0};
    struct audio_data audio;
    double in[2050];
    fftw_complex out[1025][2];
    fftw_plan p = fftw_plan_dft_r2c_1d(2048, in, *out, FFTW_MEASURE);
    int *freq;
    int fps = 60;

    for (i = 0; i < 42; i++)
    {
        bars[i] = 0;
    }

    audio.source = (char *)"auto";
    audio.format = -1;
    audio.terminate = 0;
    for (i = 0; i < 2048; i++)
    {
        audio.audio_out[i] = 0;
    }
    getPulseDefaultSink((void *)&audio);
    thr_id = pthread_create(&inputThread, NULL, input_pulse, (void *)&audio);
    audio.rate = 48000;

    // animation loop
    while (!close)
    {

        silence = 1;
        for (i = 0; i < 2050; i++)
        {
            if (i < 2048)
            {
                in[i] = audio.audio_out[i];
                if (in[i])
                    silence = 0;
            }
            else
            {
                in[i] = 0;
            }
        }

        // Check if there was silence
        if (silence == 1)
            sleep++;
        else
            sleep = 0;

        if (sleep > fps * 5)
        {
            // i sleep
            req.tv_sec = 1;
            req.tv_nsec = 0;
            nanosleep(&req, NULL);
            continue;
        }

        fftw_execute(p);

        for (i = 0; i < 42; i++)
        {
            int ab = pow(pow(*out[i * 2][0], 2) + pow(*out[i * 2][1], 2), 0.5);
            ab = ab + (ab * i / 30);
            float bar = (float)ab / (float)3500000;
            if (bar > 1)
                bar = 1;
            if (bar > bars[i])
            {
                bars[i] = bar;
            }
            else
            {
                bars[i] -= 1.05 / fps;
            }
            if (bars[i] < 0)
                bars[i] = 0;
        }

        SDL_Event event;

        // Events management
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                close = 1;
                break;
            }
        }

        draw(rend, win);

        SDL_RenderPresent(rend);

        SDL_Delay(1000 / fps);
    }

    audio.terminate = 1;
    free(audio.source);
    pthread_join(inputThread, NULL);

    // destroy renderer
    SDL_DestroyRenderer(rend);

    // destroy window
    SDL_DestroyWindow(win);

    // close SDL
    SDL_Quit();

    return 0;
}
