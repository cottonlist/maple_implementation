# Makefile
all: idiom_1 idiom_1_scaled idiom_2 idiom_3 # idiom_4 idiom_5
clean:
	rm idiom_1 idiom_1_scaled idiom_2 idiom_3 idiom_4 idiom_5
run: idiom_1_scaled
	./idiom_1_scaled
idiom_1: idiom_1.c trigger.c instrumenter.c
	gcc -pthread -o idiom_1 idiom_1.c trigger.c instrumenter.c
idiom_1_scaled: idiom_1_scaled.c trigger.c instrumenter.c
	gcc -pthread -o idiom_1_scaled idiom_1_scaled.c trigger.c instrumenter.c
idiom_2: idiom_2.c trigger.c instrumenter.c 
	gcc -pthread -o idiom_2 idiom_2.c trigger.c instrumenter.c
idiom_3: idiom_3.c trigger.c instrumenter.c 
	gcc -pthread -o idiom_3 idiom_3.c trigger.c instrumenter.c
idiom_4: idiom_4.c trigger.c instrumenter.c 
	gcc -pthread -o idiom_4 idiom_4.c trigger.c instrumenter.c
idiom_5: idiom_5.c trigger.c instrumenter.c 
	gcc -pthread -o idiom_5 idiom_5.c trigger.c instrumenter.c
instrumenter.c: trigger.h instrumenter.h
trigger.c: trigger.h instrumenter.h
idiom_1.c: trigger.h instrumenter.h
idiom_2.c: trigger.h instrumenter.h
idiom_3.c: trigger.h instrumenter.h
idiom_4.c: trigger.h instrumenter.h
idiom_5.c: trigger.h instrumenter.h
