

files_intuition = \
	OpenScreen.c \
	CloseScreen.c \
	OpenWindow.c \
	CloseWindow.c \
	MoveWindow.c \
	SizeWindow.c \
	SetWindowTitles.c \
	ActivateWindow.c \
	CreateGadgetA.c

files_graphics = \
	GetBitMapAttr.c \
	LockBitMapTagList.c 

files = \
	${files_intuition} \
	${files_graphics} \
	init.c \
	hooks/gadtools.c \
	hooks/intuition.c \
	hooks/graphics.c \
	helper/screen.c \
	helper/debug.c \
	AllocScreenBuffer.c \
	renderWindow.c \
	renderGadgets.c \
	spawn.c \
	engine.c \
	EngineTimer.c \

files_o = ${files:.c=.o}

opts += -Duse_fake_bitmap=1 
opts += -Dmax_screens=100
opts += -Dpatch_gadtools=0

opts_inline = ${opts} -D__USE_INLINE__  -Wall

incdir = -I./
incdir += -I../

all_files = BetterFakeModes ${incdir} ${files_o}

all: ${files_o}
	gcc  $(opts) BetterFakeModes.c ${files_o} -o BetterFakeModes

%.o:	%.c
		gcc -c ${incdir} ${libdir} $(opts_inline) $< $(libs) -o $@

clean:
	delete $(all_files)

