
files = init.c \
	OpenScreen.c \
	CloseScreen.c \
	OpenWindow.c \
	CloseWindow.c \
	renderWindow.c \
	helper/screen.c \
	spawn.c

files_o = ${files:.c=.o}

opts += -Duse_fake_bitmap=1 
opts += -Dmax_screens=100

opts_inline = ${opts} -D__USE_INLINE__  -Wall

incdir = -I./
incdir += -I../

all_files = BetterFakeModes ${files_o}

all: ${files_o}
	gcc  $(opts) BetterFakeModes.c ${files_o} -o BetterFakeModes

%.o:	%.c
		gcc -c ${incdir} ${libdir} $(opts_inline) $< $(libs) -o $@

clean:
	delete $(all_files)