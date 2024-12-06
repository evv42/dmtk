# dmtk - da minimalist toolkit

A dumb GUI toolkit.
It should work on most X11 systems.

Some features:

- UTF-8 support, thanks to unifont.h
- Draw images from a buffer, or from a PNG/JPG/QOI/BMP/GIF file
- Optional support for libjpeg-turbo (arithmetic-coded JPEGs)

## Changelog

17.12.2022 :
Put everything into a header instead of a old .a file.
dmtksio is removed.

## Todo

Make a Windows variant in the same header.

## Usage

dmtk depends only on Xlib.

Copy the four headers files to your project, and include dmtk like this:

```
#define DMTK_IMPLEMENTATION
#include "dmtk.h"
```

Then, compile using these flags:

```
$ cc -o program program.c -lm -lX11 -lXrender

(optionally, append -lturbojpeg -DDMTK_TURBOJPEG for libjpeg-turbo support)
```

See the examples folder for basic usage.

