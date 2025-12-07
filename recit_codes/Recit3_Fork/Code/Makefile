
# all: f1 f1_extra f2 f3 f4 f5 f6

all: f1 f2 f3 f4 f5 f6  f45  pipe1 pipe2 pipe3

clean:
	rm -f f1 f2 f3 f4 f5 f6 f45  pipe1 pipe2 pipe3

f1: fork_example1.c
	gcc -o f1 fork_example1.c -Wall
	
# f1_extra: fork_example1_extra.c
# 	gcc -o f1_extra fork_example1_extra.c -Wall	

f2: fork_example2.c
	gcc -o f2 fork_example2.c -Wall

f3: fork_example3.c
	gcc -o f3 fork_example3.c -Wall

f4: fork_example4.c
	gcc -o f4 fork_example4.c -Wall

f5: fork_example5.c
	gcc -o f5 fork_example5.c -Wall

f6: fork_example6.c
	gcc -o f6 fork_example6.c -Wall

f45: fork_example4_5.c
	gcc -o f45 fork_example4_5.c -Wall

f7: fork_example7.c
	gcc -o f7 fork_example7.c -Wall

# fn1: fork_example_new1.c
# 	gcc -o fn1 fork_example_new1.c -Wall

# fn2: fork_example_new2.c
# 	gcc -o fn2 fork_example_new2.c -Wall


pipe1: pipe_example1.c
	gcc -o pipe1 pipe_example1.c -Wall

pipe2: pipe_example2.c
	gcc -o pipe2 pipe_example2.c -Wall

pipe3: pipe_example3.c
	gcc -o pipe3 pipe_example3.c -Wall