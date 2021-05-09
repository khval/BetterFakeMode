
deps = ../init.c

files = ${deps}

tests_c = OpenScreen.c 
tests_c += OpenWindow.c
tests_c += dbuf.c

incdir = -I../ -I./

deps_o = ${deps:.c=.o}
files_o = ${files:.c=.o}


tests = ${tests_c:.c=.elf} 
all_files = $(files_o) ${tests}


all: ${all_files}



%.elf: %.c
		gcc -D__USE_INLINE__ ${incdir} ${libdir} $< $(deps_o) $(libs) -o $@

%.o:	%.c
		gcc -D__USE_INLINE__ ${incdir} ${libdir} $(opts) -c $<   -o $@



clean:
	rm ${all_files}
