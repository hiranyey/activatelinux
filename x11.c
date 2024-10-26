#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can not connect to X server");
    return 1;
  }
  int screen = DefaultScreen(display);
  int screen_width = DisplayWidth(display, screen);
  int screen_height = DisplayHeight(display, screen);
  unsigned long white_pixel = WhitePixel(display, screen);
  unsigned long black_pixel = BlackPixel(display, screen);
  unsigned int window_x = screen_width - 320;
  unsigned int window_y = screen_height - 100;
  unsigned int window_width = 300;
  unsigned int window_height = 50;
  unsigned int window_border = 0;

  XVisualInfo vinfo;
  XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &vinfo);

  XSetWindowAttributes attr;
  attr.colormap = XCreateColormap(display, DefaultRootWindow(display),
                                  vinfo.visual, AllocNone);
  attr.border_pixel = 0;
  attr.background_pixel = 0;

  Window win = XCreateWindow(
      display, RootWindow(display, screen), window_x, window_y, window_width,
      window_height, window_border, vinfo.depth, InputOutput, vinfo.visual,
      CWColormap | CWBorderPixel | CWBackPixel, &attr);
  XStoreName(display, win, "ActivateLinux");
  XMapWindow(display, win);
  XFlush(display);
  XCharStruct text_info;

  XSelectInput(display, win, ExposureMask | KeyPressMask);
  GC gc = XCreateGC(display, win, 0, NULL);
  char *font_name = "-urw-urw bookman l-light-r-normal--0-0-0-0-p-0-iso8859-15";
  XFontStruct *font = XLoadQueryFont(display, font_name);
  if (font) {
    XSetFont(display, gc, font->fid);
  } else {
    fprintf(stderr, "XLoadQueryFont: failed loading font '%s'\n", font_name);
  }
  XSetForeground(display, gc, white_pixel);
  char *activateLinux = "Activate your linux";
  char *settingLine = "Go to Settings to activate Linux.";
  XEvent event;

  while (True) {
    XNextEvent(display, &event);
    switch (event.type) {
    case Expose:
      if (event.xexpose.count)
        break;
      XDrawString(display, win, gc, 20, 20, activateLinux,
                  strlen(activateLinux));
      XDrawString(display, win, gc, 20, 40, settingLine, strlen(settingLine));
      break;

    default:
      break;
    }
  }

  XUnloadFont(display, font->fid);
  XFreeGC(display, gc);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
  return 0;
}
