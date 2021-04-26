
files = init.c \
	OpenScreen.c \
	CloseScreen.c \
	spawn.c

files_o = ${files:.c=.o}

opts = -D__USE_INLINE__ 
opts += -Duse_fake_bitmap=1

all_files = BetterFakeModes ${files_o}

all: ${files_o}
	gcc  BetterFakeModes.c ${files_o} -o BetterFakeModes

%.o:	%.c
		gcc -c ${incdir} ${libdir} $(opts) $< $(libs) -o $@

clean:
	delete $(all_files)