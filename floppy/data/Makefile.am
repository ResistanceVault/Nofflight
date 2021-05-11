IMGDIR=../../_res/IFF/

all resistance_final.raw resistance_final.plt: ${IMGDIR}resistance_final.iff
	rm resistance_final.raw resistance_final.plt
	ilbm2raw ${IMGDIR}resistance_final.iff resistance_final.raw -p resistance_final.plt
	chmod 777 resistance_final.raw resistance_final.plt
	cp resistance_final.raw ../../adf/data
	cp resistance_final.plt ../../adf/data

	rm -f piston_logo.raw piston_logo.plt
	ilbm2raw ${IMGDIR}piston-logo-by-fra.iff piston_logo.raw -p piston_logo.plt
	chmod 777 piston_logo.raw piston_logo.plt
	cp piston_logo.raw ../../adf/data
	cp piston_logo.plt ../../adf/data

clean: 
	echo "cancello"