# Makefile
all: idiom_1 idiom_2
clean:
    rm idiom_1 idiom_2
run: idiom_2
    ./idiom_2
idiom_1: idiom_1.c
    gcc -pthread -o idiom_1 idiom_1.c
idiom_2: idiom_2.c
    gcc -pthread -o idiom_2 idiom_2.c
    