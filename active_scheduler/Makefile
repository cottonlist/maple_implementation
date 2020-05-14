# Makefile
all: idiom_1 idiom_2 idiom_3 idiom_4
clean:
	rm idiom_1 idiom_2 idiom_3 idiom_4
run: idiom_4
	./idiom_4
idiom_1: idiom_1.c trigger.c
	gcc -pthread -o idiom_1 idiom_1.c trigger.c
idiom_2: idiom_2.c trigger.c
	gcc -pthread -o idiom_2 idiom_2.c trigger.c
idiom_3: idiom_3.c trigger.c
	gcc -pthread -o idiom_3 idiom_3.c trigger.c
idiom_4: idiom_4.c trigger.c
	gcc -pthread -o idiom_4 idiom_4.c trigger.c
trigger.c: trigger.h
idiom_1.c: trigger.h	
idiom_2.c: trigger.h
idiom_3.c: trigger.h
idiom_4.c: trigger.h