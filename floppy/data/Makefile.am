IMGDIR=../../_res/IFF/

all resistance_final.raw resistance_final.plt: ${IMGDIR}resistance_final.iff
	rm resistance_final.raw resistance_final.plt
	ilbm2raw ${IMGDIR}resistance_final.iff resistance_final.raw -p resistance_final.plt
	chmod 777 resistance_final.raw resistance_final.plt
	cp resistance_final.raw ../../adf/data
	cp resistance_final.plt ../../adf/data

clean: rm *