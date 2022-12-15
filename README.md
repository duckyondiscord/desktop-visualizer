# desktop-visualizer
Linux Desktop Music visualizer made with SFML

![](screenshot.png)

It's not easily customizable yet, though I've added config options through environment variables, you can find out what those are called by reading `install/run.sh`.

I use some snippets of code from [cava](https://github.com/karlstav/cava) and [conky](https://github.com/brndnmtthws/conky).

# Dependencies

 - PulseAudio
 - fftw3
 - SFML 2.5

# Building

```
$ mkdir build && cd build
$ cmake ..
$ make
```

# Installation

Build the project, then run `./install.sh` in the project root. After that, you can execute it using the desktop icon named "Desktop Visualizer" it just created.

# TODO

 - Allow customization (color, position, size, etc) via config file
 - Add support for other audio sources
 - More visualizer designs (possibly one similar to [cava](https://github.com/karlstav/cava).
