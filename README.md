# desktop-visualizer
Linux Desktop Music visualizer made with SFML

![](screenshot.png)

It's not easily customizable yet, though you can dig into the source code to change colors, framerate and transparency, it's pretty well-commented.

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

# TODO

 - Allow customization (color, position, size, etc) via config file
 - Add support for other audio sources
 - More visualizer designs (possibly one similar to [cava](https://github.com/karlstav/cava).
