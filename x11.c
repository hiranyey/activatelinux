#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <fontconfig/fontconfig.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Unable to open X display\n");
    return 1;
  }

  int screen = DefaultScreen(display);

  XVisualInfo vinfo;
  if (!XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo)) {
    fprintf(stderr, "No ARGB visual found\n");
    return 1;
  }

  XSetWindowAttributes attr;
  attr.colormap = XCreateColormap(display, DefaultRootWindow(display),
                                  vinfo.visual, AllocNone);
  attr.border_pixel = 0;
  attr.background_pixel = 0;

  int screen_width = DisplayWidth(display, screen);
  int screen_height = DisplayHeight(display, screen);
  unsigned long white_pixel = WhitePixel(display, screen);
  unsigned long black_pixel = BlackPixel(display, screen);
  unsigned int window_x = screen_width - 320;
  unsigned int window_y = screen_height - 100;
  unsigned int window_width = 300;
  unsigned int window_height = 50;
  unsigned int window_border = 0;

  Window window = XCreateWindow(
      display, DefaultRootWindow(display), window_x, window_y, window_width,
      window_height, window_border, vinfo.depth, InputOutput, vinfo.visual,
      CWColormap | CWBorderPixel | CWBackPixel, &attr);

  XStoreName(display, window, "ActivateLinux");
  XSelectInput(display, window, ExposureMask);
  XMapWindow(display, window);

  XftDraw *xft_draw = XftDrawCreate(display, window, vinfo.visual,
                                    DefaultColormap(display, screen));
  FcPattern *pattern;
  if (argc > 1) {
    pattern = FcNameParse((const FcChar8 *)argv[1]);
  } else {
    pattern = FcNameParse((const FcChar8 *)"Iosevka NF-16");
  }
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  XftFont *font = XftFontOpenPattern(
      display, XftFontMatch(display, screen, pattern, &result));
  FcPatternDestroy(pattern);

  if (!font) {
    fprintf(stderr, "Unable to load font\n");
    return 1;
  }

  XftColor color;
  XRenderColor render_color = {0xffff, 0xffff, 0xffff, 0xffff};
  XftColorAllocValue(display, vinfo.visual, DefaultColormap(display, screen),
                     &render_color, &color);

  char *activateLinux = "Activate your Linux";
  char *settingLine = "Go to Settings to activate Linux.";
  XEvent event;
  while (1) {

    XNextEvent(display, &event);

    if (event.type == Expose) {
      XftDrawString8(xft_draw, &color, font, 20, 20, (XftChar8 *)activateLinux,
                     strlen(activateLinux));
      XftDrawString8(xft_draw, &color, font, 20, 40, (XftChar8 *)settingLine,
                     strlen(settingLine));
    } else if (event.type == KeyPress) {
      break;
    }
  }

  XftColorFree(display, vinfo.visual, DefaultColormap(display, screen), &color);
  XftFontClose(display, font);
  XftDrawDestroy(xft_draw);
  XCloseDisplay(display);

  return 0;
}
