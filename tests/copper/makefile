
deps = ../init.c
deps += ../debug.c
deps += common.c

files =binary.h ${deps} 

tests_c += CopperCheck.c
tests_c += CopperSplit.c
tests_c += OSCopper.c
tests_c += copperbar.c
tests_c += plasma.c

incdir = -I../ -I./

deps_o = ${deps:.c=.o}
files_o = ${files:.c=.o}


tests = ${tests_c:.c=.elf} 
all_files = $(files_o) ${tests}


all: ${all_files}

%.elf: %.c
		gcc  -D__USE_INLINE__ ${incdir} ${libdir} $< $(deps_o) $(libs) -o $@

%.o:	%.c
		gcc -D__USE_INLINE__ ${incdir} ${libdir} $(opts) -c $<   -o $@


binary.h: gen_binary.elf
		gen_binary.elf > binrary.h

gen_binary.elf: gen_binary.c
		gcc -D__USE_INLINE__  $(opts)  $<   -o $@
		

clean:
	rm ${all_files}
