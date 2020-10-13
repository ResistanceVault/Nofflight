bin_PROGRAMS = flash2021
flash2021_LDADD = music_ptr_linkable2.o
flash2021_SOURCES = main.c simplebuffertest.c sprites.c physics.c vectors.c goatlight.c resistancelogo.c metaballschunky.c slidingtext.c

install: flash2021
	cp flash2021 ../adf/
	mv flash2021 ../floppy/
	chmod 777 ../floppy/flash2021

music_ptr_linkable2.o: music_ptr_linkable2.s
	vasmm68k_mot -devpac -unnamed-sections -chklabels -Fhunk -quiet -esc  -DINTERLACE=0 ./music_ptr_linkable2.s  -o ./music_ptr_linkable2.o -I/opt/amiga/m68k-amigaos/ndk-include/