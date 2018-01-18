src = $(wildcard *.c)
obj = $(src:.c=.o)

# Build type parameter

ifeq ($(BUILD),debug)
    CFLAGS_BUILD = -ggdb -Wall
    GLAD_GEN = c-debug
else
    CFLAGS_BUILD = -O2 -march=native
    GLAD_GEN = c
endif

# Detect OS if not specified (OSX, Linux, BSD are supported)

ifndef INSTALL
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        INSTALL = osx
    else
        INSTALL = unix
    endif
endif

# Install type parameter

ifeq ($(INSTALL),standalone)
    CFLAGS_INSTALL = -DGLAVA_STANDALONE
endif

ifeq ($(INSTALL),unix)
    CFLAGS_INSTALL = -DGLAVA_UNIX
    ifdef XDG_CONFIG_DIRS
        SHADER_DIR = $(firstword $(subst :, ,$XDG_CONFIG_DIR))/glava
    else
        SHADER_DIR = etc/xdg/glava
    endif
endif

ifeq ($(INSTALL),osx)
    CFLAGS_INSTALL = -DGLAVA_OSX
    SHADER_DIR = Library/glava
endif

LDFLAGS = -lpulse -lpulse-simple -pthread -lglfw -ldl -lm -lX11 -lXext

PYTHON = python

GLAD_INSTALL_DIR = glad
GLAD_SRCFILE = ./glad/src/glad.c
GLAD_ARGS = --generator=$(GLAD_GEN) --extensions=GL_EXT_framebuffer_multisample,GL_EXT_texture_filter_anisotropic
CFLAGS_COMMON = -I glad/include
CFLAGS_USE = $(CFLAGS_COMMON) $(CFLAGS_BUILD) $(CFLAGS_INSTALL) $(CFLAGS)

all: glava-dep
glava-dep: glad $(obj)
	$(CC) -o glava $(obj) glad.o $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS_USE) -o $@ -c $<

# GLava target without glad dependency
glava: $(obj)
	$(CC) -o glava $^ glad.o $(LDFLAGS)

.PHONY: glad
glad:
	cd $(GLAD_INSTALL_DIR) && $(PYTHON) -m glad $(GLAD_ARGS) --out-path=.
	$(CC) $(CFLAGS_USE) -o glad.o $(GLAD_SRCFILE) -c

.PHONY: clean
clean:
	rm -f $(obj) glava glad.o

.PHONY: install
install:
	cp glava $(DESTDIR)/usr/bin/glava
	mkdir -p $(DESTDIR)/$(SHADER_DIR)
	cp -Rv shaders/* $(DESTDIR)/$(SHADER_DIR)

.PHONY: uninstall
uninstall:
	rm /usr/bin/glava
	rm -rf $(SHADER_DIR)/glava

