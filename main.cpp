#include <iostream>
#include <fftw3.h>
#include <pthread.h>
#include <INIReader.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_syswm.h>
#include "input/pulse.h"
#include "input/pulse.cpp"
#include "main.hpp"
#include "util.cpp"

Display *x11Display;

static int resizingEventWatcher(void *data, SDL_Event *event)
{
    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_RESIZED)
    {
        SDL_Window *win = SDL_GetWindowFromID(event->window.windowID);
        if (win == (SDL_Window *)data)
        {
            int h, w;
            SDL_GetWindowSize(win, &w, &h);

            if (w != 0 && h != 0)
            {
                width = w;
                height = h;
            }
        }
    }
    return 0;
}

SDL_Window *window()
{

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_Window *win = SDL_CreateWindow("Visualino", // creates a window
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       width, 350, SDL_WINDOW_BORDERLESS | SDL_WINDOW_SKIP_TASKBAR);

    if (SDL_GetWindowWMInfo(win, &wmInfo) == SDL_TRUE)
    {
        std::cout << "Got x11 display and window" << std::endl;
        x11Display = wmInfo.info.x11.display;
        Window x11Window = wmInfo.info.x11.window;

        XSetWMProtocols(x11Display, x11Window, NULL, 0);

        x11_window_set_desktop(x11Display, x11Window);
        x11_window_set_borderless(x11Display, x11Window);
        x11_window_set_below(x11Display, x11Window);
        x11_window_set_sticky(x11Display, x11Window);
        x11_window_set_skip_taskbar(x11Display, x11Window);
        x11_window_set_skip_pager(x11Display, x11Window);
    }
    SDL_Surface *icon = IMG_Load("assets/icon.png");
    SDL_SetWindowIcon(win, icon);
    SDL_AddEventWatch(resizingEventWatcher, win);

    delete icon;
    return win;
}

void ctrl_c(int s)
{
    printf("\nQuitting...\n");
    exit(0);
}

void draw(SDL_Renderer *rend)
{
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
    SDL_RenderClear(rend);

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    for (int i = 0; i < 64; i++)
    {
        float bar = bars[i];
        float w = width / 60;
        int h = bar * 256;
        SDL_Rect rect;
        rect.h = h;
        rect.w = w;
        rect.x = i * (w + 5) + 20;
        switch (barposition)
        {
        case BARPOSITION::BAR_BOTTOM:
            rect.y = height - h;
            break;
        case BARPOSITION::BAR_MIDDLE:
            rect.y = (height / 2) - (h / 2);
            break;
        case BARPOSITION::BAR_TOP:
            rect.y = 0;
            break;
        }
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
    SDL_Window *win = window();
    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    // creates a renderer to render our images
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, render_flags);

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

        draw(rend);

        SDL_RenderPresent(rend);

        SDL_Delay(1000 / fps);
    }

    audio.terminate = 1;
    free(audio.source);
    pthread_join(inputThread, NULL);

    // destroy renderer
    SDL_DestroyRenderer(rend);

    XCloseDisplay(x11Display);
    // destroy window
    SDL_DestroyWindow(win);

    // close SDL
    SDL_Quit();

    return 0;
}