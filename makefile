MAKE=mingw32-make

all:

tests:
	cd "./test" && $(MAKE) test

clean:
	del obj\*.o test\*.exe