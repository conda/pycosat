pycosat.so: libpicosat.a pycosat.c
	python setup.py build_ext --inplace

picosat.o: picosat.c picosat.h
	$(CC) $(CFLAGS) -c $<

libpicosat.a: picosat.o
	ar rc $@ picosat.o 


test: pycosat.so
	python test_pycosat.py


clean:
	rm -f picosat.so picosat.o libpicosat.a
