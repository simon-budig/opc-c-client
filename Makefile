all: renderer-simon renderer-fun

renderer-simon: renderer-simon.c
	gcc -Wall -lm -o renderer-simon renderer-simon.c

renderer-fun: renderer-fun.c
	gcc -Wall -lm -o renderer-fun renderer-fun.c 
