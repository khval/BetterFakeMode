
deps = ../init.c
deps += debug.c

files = ${deps}

tests_c = OpenScreen.c 
tests_c += OpenScreen_0021000.c
tests_c += OpenScreen_HAM.c
tests_c += OpenWindow.c
tests_c += dbuf.c
tests_c += Gadtools.c
tests_c += ResizeWindow.c
tests_c += SetWindowTitles.c 
tests_c += ActiveWindow.c
tests_c += paint.c
tests_c += RawKey.c
tests_c += ScreenToFront.c

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
