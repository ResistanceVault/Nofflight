#CC 			= incbin.sh
#CLEANFILES = Dirty_Tricks.h
#noinst_SCRIPTS = Dirty_Tricks.h

MODDIR=../_res/MOD/
IFFDIR=../_res/IFF/

all Dirty_Tricks.h goatblock32X32.h goatblockplt.h: ${MODDIR}amazed_by_the_pokey.mod ${IFFDIR}goatblock.iff
	incbin.sh ${MODDIR}amazed_by_the_pokey.mod amazed_by_the_pokey.h amazed_by_the_pokey
	incbin.sh ${MODDIR}amazed_by_the_pokey2.mod amazed_by_the_pokey2.h amazed_by_the_pokey2
	rm -f ${IFFDIR}goatblockplt.plt ${IFFDIR}goatblock32X32.raw
	ilbm2raw ${IFFDIR}goatblock_32x32.iff ${IFFDIR}goatblock32X32.raw -p ${IFFDIR}goatblockplt.plt
	incbin.sh ${IFFDIR}goatblock32X32.raw goatblock32X32.h goatblock32X32
	incbin.sh ${IFFDIR}goatblockplt.plt goatblockplt.h goatblockplt



clean: 
	rm -f goatblock32X32.h goatblockplt.h

        
#    incbin.sh Dirty_Tricks.mod Dirty_Tricks.h Dirty_Tricks
