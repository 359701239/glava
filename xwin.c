/* X11 specific code and features */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XShm.h>

#define GLFW_EXPOSE_NATIVE_X11

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/* Hack to make GLFW 3.1 headers work with GLava. We don't use the context APIs from GLFW, but
   the old headers require one of them to be selected for exposure in glfw3native.h. */
#if GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR <= 1
#define GLFW_EXPOSE_NATIVE_GLX
#error "GLX defined"
#endif
#include <GLFW/glfw3native.h>

#include "render.h"
#include "xwin.h"

bool xwin_should_render(void) {
    bool ret = true, should_close = false;
    Display* d = glfwGetX11Display();
    if (!d) {
        d = XOpenDisplay(0);
        should_close = true;
    }

    Atom prop       = XInternAtom(d, "_NET_ACTIVE_WINDOW", true);
    Atom fullscreen = XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", true);
    
    Atom actual_type;
    int actual_format, t;
    unsigned long nitems, bytes_after;
    unsigned char* data;

    int handler(Display* d, XErrorEvent* e) { return 0; }
    
    XSetErrorHandler(handler); /* dummy error handler */
          
    if (Success != XGetWindowProperty(d, RootWindow(d, 0), prop, 0, 1, false, AnyPropertyType,
                                     &actual_type, &actual_format, &nitems, &bytes_after, &data)) {
        goto close; /* if an error occurs here, the WM probably isn't EWMH compliant */
    }

    Window active = ((Window*) data)[0];

    prop = XInternAtom(d, "_NET_WM_STATE", true);

    if (Success != XGetWindowProperty(d, active, prop, 0, LONG_MAX, false, AnyPropertyType,
                                      &actual_type, &actual_format, &nitems, &bytes_after, &data)) {
        goto close; /* some WMs are a little slow on creating _NET_WM_STATE, so errors may occur here */
    }
    for (t = 0; t < nitems; ++t) {
        if (fullscreen == ((Atom*) data)[t]) {
            ret = false;
        }
    }
 close:
    if (should_close)
        XCloseDisplay(d);
    return ret;
}

/* Set window types defined by the EWMH standard, possible values:
   -> "desktop", "dock", "toolbar", "menu", "utility", "splash", "dialog", "normal" */
static void xwin_changeatom(struct renderer* rd, const char* type, const char* atom, const char* fmt, int mode) {
    Window w = glfwGetX11Window((GLFWwindow*) rd_get_impl_window(rd));
    Display* d = glfwGetX11Display();
    Atom wtype = XInternAtom(d, atom, false);
    size_t len = strlen(type), t;
    char formatted[len + 1];
    for (t = 0; t < len + 1; ++t) {
        char c = type[t];
        switch (c) {
        case 'a' ... 'z': c -= 'a' - 'A';
        default:          formatted[t] = c;
        }
    }
    char buf[256];
    snprintf(buf, sizeof(buf), fmt, formatted);
    Atom desk = XInternAtom(d, buf, false);
    XChangeProperty(d, w, wtype, XA_ATOM, 32, mode, (unsigned char*) &desk, 1);
}

void xwin_settype(struct renderer* rd, const char* type) {
    xwin_changeatom(rd, type, "_NET_WM_WINDOW_TYPE", "_NET_WM_WINDOW_TYPE_%s", PropModeReplace); 
}

void xwin_addstate(struct renderer* rd, const char* state) {
    xwin_changeatom(rd, state, "_NET_WM_STATE", "_NET_WM_STATE_%s", PropModeAppend); 
}

static Pixmap get_pixmap(Display* d, Window w) {
    Pixmap p;
    Atom act_type;
    int act_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    Atom id;

    id = XInternAtom(d, "_XROOTPMAP_ID", False);

    if (XGetWindowProperty(d, w, id, 0, 1, False, XA_PIXMAP,
                           &act_type, &act_format, &nitems, &bytes_after,
                           &data) == Success) {
        if (data) {
            p = *((Pixmap *) data);
            XFree(data);
        }
    }

    return p;
}

unsigned int xwin_copyglbg(struct renderer* rd, unsigned int tex) {
    GLuint texture = (GLuint) tex;
    if (!texture)
        glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    GLFWwindow* gwin = (GLFWwindow*) rd_get_impl_window(rd);
    int x, y, w, h;
    glfwGetFramebufferSize(gwin, &w, &h);
    glfwGetWindowPos(gwin, &x, &y);
    XColor c;
    Display* d = glfwGetX11Display();
    Pixmap p = get_pixmap(d, RootWindow(d, DefaultScreen(d)));

    /* Obtain section of root pixmap using XShm */
    
    XShmSegmentInfo shminfo;
    Visual* visual = DefaultVisual(d, DefaultScreen(d));
    XVisualInfo match = { .visualid = XVisualIDFromVisual(visual) };
    int nret;
    XVisualInfo* info = XGetVisualInfo(d, VisualIDMask, &match, &nret);
    XImage* image = XShmCreateImage(d, visual, info->depth, ZPixmap, NULL,
                                    &shminfo, (unsigned int) w, (unsigned int) h);
    if ((shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777)) == -1) {
        fprintf(stderr, "shmget() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    shminfo.shmaddr = image->data = shmat(shminfo.shmid, 0, 0);
    shminfo.readOnly = false;
    XShmAttach(d, &shminfo);
    XShmGetImage(d, p, image, x, y, AllPlanes);

    /* Try to convert pixel bit depth to OpenGL storage format. The following formats\
       will need intermediate conversion before OpenGL can accept the data:
       
       - 8-bit pixel formats (retro displays, low-bandwidth virtual displays)
       - 36-bit pixel formats (rare deep color displays) */
    
    bool invalid = false, aligned = false;
    GLenum type;
    switch (image->bits_per_pixel) {
    case 16:
        switch (image->depth) {
        case 12: type = GL_UNSIGNED_SHORT_4_4_4_4; break; /* 12-bit (rare)    */
        case 15: type = GL_UNSIGNED_SHORT_5_5_5_1; break; /* 15-bit, hi-color */
        case 16:                                          /* 16-bit, hi-color */
            type    = GL_UNSIGNED_SHORT_5_6_5;
            aligned = true;
            break;
        }
        break;
    case 32:
        switch (image->depth) {
        case 24: type = GL_UNSIGNED_BYTE;           break; /* 24-bit, true color */
        case 30: type = GL_UNSIGNED_INT_10_10_10_2; break; /* 30-bit, deep color */
        }
        break;
    case 64: 
        if (image->depth == 48) /* 48-bit deep color */
            type = GL_UNSIGNED_SHORT;
        else goto invalid;
        break;
        /* >64-bit formats */
    case 128:
        if (image->depth == 96)
            type = GL_UNSIGNED_INT;
        else goto invalid;
        break;
    default:
    invalid: invalid = true;
    }
    
    uint8_t* buf;
    if (invalid) {
        abort();
        /* Manual reformat (slow) */
        buf = malloc(4 * w * h);
        int xi, yi;
        Colormap map = DefaultColormap(d, DefaultScreen(d));
        for (yi = 0; yi < h; ++yi) {
            for (xi = 0; xi < w; ++xi) {
                c.pixel = XGetPixel(image, xi, yi);
                XQueryColor(d, map, &c);
                size_t base = (xi + (yi * w)) * 4;
                buf[base + 0] = c.red   / 256;
                buf[base + 1] = c.green / 256;
                buf[base + 2] = c.blue  / 256;
                buf[base + 3] = 255;
            }
        }
    
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
        free(buf);
    } else {
        /* Use image data directly. The alpha value is garbage/unassigned data, but
           we need to read it because X11 keeps pixel data aligned */
        buf = (uint8_t*) image->data;
        /* Data could be 2, 4, or 8 byte aligned, the RGBA format and type (depth)
           already ensures reads will be properly aligned across scanlines */
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format = image->bitmap_bit_order == LSBFirst ?
            (!aligned ? GL_BGRA : GL_BGR) :
            (!aligned ? GL_RGBA : GL_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, type, buf);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); /* restore default */
    }

    XShmDetach(d, &shminfo);
    shmdt(shminfo.shmaddr);
    shmctl(shminfo.shmid, IPC_RMID, NULL);

    XDestroyImage(image);
    
    return texture;
}
