all: renderer-simon renderer-fun

renderer-simon: opc-client.o renderer-simon.c
	gcc -Wall -lm -o renderer-simon opc-client.o renderer-simon.c

renderer-fun: renderer-fun.c
	gcc -Wall -lm -o renderer-fun renderer-fun.c 

opc-client.o: opc-client.c
	gcc -Wall -c -o opc-client.o opc-client.c
