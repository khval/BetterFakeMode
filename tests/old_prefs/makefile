
inc =-I../inc -I./


files += blitz.c
files += init.c 
files += ../debug.c
files_o = ${files:.c=.o}

exe = wb96setup

opts = -D__USE_INLINE__ 
opts += -Duse_custom_screen=1
opts += -Duse_gadtools=1

all: ${files_o}
	gcc  $(inc) $(opts) ${files_o} wb96setup_0.c  -o ${exe}

%.o:	%.c
		gcc -c $(incdir) $(libdir) $(opts) $< $(libs) -o $@

clean:
	delete ${files_o} ${exe}

