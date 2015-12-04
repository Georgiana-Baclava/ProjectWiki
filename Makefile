CC = gcc -c
LD = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -o3
INCLUDES = -pthread

main: main.o strgraph.o
	$(LD) $(INCLUDES) $^ $(CFLAGS) -o $@
	
testgen: testgen.o
	$(LD) $(INCLUDES) $^ $(CFLAGS) -o $@
	
main.o: main.c strgraph.h
	$(CC) $(INCLUDES) $< $(CFLAGS) -o $@
	
strgraph.o: strgraph.c strgraph.h
	$(CC) $(INCLUDES) $< $(CFLAGS) -o $@

testgen.o: testgen.c strgraph.h
	$(CC) $(INCLUDES) $< $(CFLAGS) -o $@
 	
clean:
	rm -f *.o 
	rm -f main testgen
	rm -f *~
	
