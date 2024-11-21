# The final executable file built from the object files.
lab5: main.o hashmap.o word_count.o bounded_buffer.o
	gcc -pthread -Wall -g main.o hashmap.o word_count.o bounded_buffer.o -o lab5

# Compile main.c into main.o.
main.o: main.c
	gcc -pthread -Wall -g -c main.c

# Compile word_count.c into word_count.o.
word_count.o: word_count.c word_count.h hashmap.h
	gcc -pthread -Wall -g -c word_count.c

# Compile hashmap.c into hashmap.o.
hashmap.o: hashmap.c hashmap.h
	gcc -pthread -Wall -g -c hashmap.c	

# Compile bounded_buffer.c into bounded_buffer.o.
bounded_buffer.o: bounded_buffer.c bounded_buffer.h
	gcc -pthread -Wall -g -c bounded_buffer.c	

# Run the program with arguments.
run: lab5
	./lab5 6 2 AA B C D AA B

# Remove all generated object files and the executable.
clean:
	rm -f *.o lab5
