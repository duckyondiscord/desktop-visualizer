#include <iostream>
#include <fftw3.h>
#include <pthread.h>
#include <INIReader.h>
#include <math.h>
#include <time.h>
#include <SFML/Graphics.hpp>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "util.cpp"
#include "input/pulse.h"
#include "input/pulse.cpp"

float bars[64];
Display *display = XOpenDisplay(NULL);

Window TransparentWindow(int MAX_HEIGHT, int WINDOW_WIDTH)
{
    XVisualInfo visualinfo;
    XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &visualinfo);

    // Create window
    Window wnd;
    GC gc;
    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(display, DefaultRootWindow(display), visualinfo.visual, AllocNone);
    attr.event_mask = ExposureMask | KeyPressMask;
    attr.background_pixmap = None;
    attr.border_pixel = 0;
    attr.override_redirect = true;
    wnd = XCreateWindow(
        display, DefaultRootWindow(display),
        (sf::VideoMode::getDesktopMode().width / 2) - (WINDOW_WIDTH / 2),
        sf::VideoMode::getDesktopMode().height - (MAX_HEIGHT * 2),
        WINDOW_WIDTH,
        MAX_HEIGHT * 2,
        0,
        visualinfo.depth,
        InputOutput,
        visualinfo.visual,
        CWColormap | CWEventMask | CWBackPixmap | CWBorderPixel,
        &attr);
    gc = XCreateGC(display, wnd, 0, 0);
    XStoreName(display, wnd, "Visualizer");

    XSizeHints sizehints;
    sizehints.flags = PPosition | PSize;
    sizehints.x = (sf::VideoMode::getDesktopMode().width / 2) - (WINDOW_WIDTH);
    sizehints.y = sf::VideoMode::getDesktopMode().height - (MAX_HEIGHT);
    sizehints.width = WINDOW_WIDTH;
    sizehints.height = MAX_HEIGHT * 2;
    XSetWMNormalHints(display, wnd, &sizehints);

    XSetWMProtocols(display, wnd, NULL, 0);

    x11_window_set_desktop(display, wnd);
    x11_window_set_borderless(display, wnd);
    x11_window_set_below(display, wnd);
    x11_window_set_sticky(display, wnd);
    x11_window_set_skip_taskbar(display, wnd);
    x11_window_set_skip_pager(display, wnd);

    return wnd;
}
#undef None

void *handleXEvents(void *idk)
{
    XEvent event;
    while (1)
    {
        XNextEvent(display, &event);
        switch (event.type)
        {
        case Expose:
            std::cout << "Received Expose event" << std::endl;
            break;
        case ButtonPress:
            std::cout << "Received ButtonPress event" << std::endl;
            break;
        case KeyPress:
            std::cout << "Received KeyPress event" << std::endl;
            break;
        }
    }
    return NULL;
}

void draw(sf::RenderWindow *window, int MAX_HEIGHT, int red, int green, int blue, int alpha)
{ // render stuff :D
    int i;

    sf::Vector2u s = window->getSize();
    window->clear(sf::Color::Transparent);

    for (i = 0; i < 64; i++)
    {
        float bar = bars[i];
        float width = (float)s.x / (float)42;
        float height = bar * MAX_HEIGHT;
        float posY = (s.y / 2) - (height / 2);

        // Create rectangles(bars) and set properties
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(sf::Vector2f(width * i, posY));
        rect.setFillColor(sf::Color(red, green, blue, alpha)); // Set RGB color and opacity values
        window->draw(rect);
    }

    window->display();
}

int main()
{

    std::string confFile = std::getenv("HOME");
    confFile = confFile + "/.config/deskvis.ini";
    INIReader reader(confFile); // Open config file

    if (reader.ParseError() < 0) // Check if we can't load/parse the config file
    {
        std::cerr << "Can't load config file, exiting!\n";
        return 1;
    }

    // Set necessary variables from the config file
    float fps = reader.GetInteger("window", "fps", 144);
    int MAX_HEIGHT = reader.GetInteger("window", "height", 256);
    int WINDOW_WIDTH = reader.GetInteger("window", "width", 1024);

    Window win = TransparentWindow(MAX_HEIGHT, WINDOW_WIDTH);
    sf::RenderWindow window(win);
    window.setFramerateLimit(fps);

    sf::Clock clock;
    pthread_t inputThread;
    pthread_t eventThread;
    int i, thr_id, silence, sleep, eventThread_id;
    struct timespec req = {.tv_sec = 0, .tv_nsec = 0};
    struct audio_data audio;
    double in[2050];
    fftw_complex out[1025][2];
    fftw_plan p = fftw_plan_dft_r2c_1d(2048, in, *out, FFTW_MEASURE);
    int *freq;

    // Initialization
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
    eventThread_id = pthread_create(&eventThread, NULL, handleXEvents, NULL);
    audio.rate = 48000;

    // Main Loop
    while (window.isOpen())
    {
        // Copy Audio Data
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

        // real shit??
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
                bars[i] -= 1 / fps;
            }
            if (bars[i] < 0)
                bars[i] = 0;
        }

        // Render
        INIReader reader(confFile);
        draw(&window, MAX_HEIGHT, reader.GetInteger("color", "red", 4), reader.GetInteger("color", "green", 248), reader.GetInteger("color", "blue", 0), reader.GetInteger("color", "alpha", 153));
        fps = 1 / clock.restart().asSeconds();
        // Handle Events
        sf::Event event;
        window.pollEvent(event);
        if (event.type == sf::Event::Closed)
            window.close();
    }

    // Free resources
    audio.terminate = 1;
    pthread_join(inputThread, NULL);
    pthread_join(eventThread, NULL);
    free(audio.source);

    return 0;
}
