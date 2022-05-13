target: compile

compile:
	gcc -Wall main.c additional.c -o hw4

run:
	gcc -Wall *.c -o hw4
	./hw4 -C 6 -N 2 -F files/file.txt

run2:
	gcc -Wall *.c -o hw4
	./hw4 -C 10 -N 5 -F files/file2.txt

debug:
	gcc -Wall -g *.c -o hw4
	gdb -q ./hw4
# r -C 6 -N 2 -F files/file.txt

debug2:
	gcc -Wall -g *.c -o hw4
	gdb -q ./hw4
# r -C 10 -N 5 -F files/file2.txt

run_warning:
	gcc -Wall -Wextra *.c -o hw4
	./hw4 -C 10 -N 5 -F files/file.txt

clean:
	rm -f hw4

run_valgrind:
	gcc -g -Wall *.c -o hw4
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes  ./hw4 -C 10 -N 5 -F files/file2.txt

# Suleyman Golbol 1801042656