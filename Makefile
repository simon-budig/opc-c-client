all: renderer-all 

renderer-simon: opc-client.o render-utils.o renderer-simon.c
	gcc -Wall -lm -o renderer-simon opc-client.o render-utils.o renderer-simon.c

renderer-all: renderer-all.c opc-client.o render-utils.o renderer_astern.o renderer_ball.o
	gcc -Wall -lm -o $@  $^

renderer-fun: renderer-fun.c
	gcc -Wall -lm -o renderer-fun renderer-fun.c 

opc-client.o: opc-client.c
	gcc -Wall -c -o opc-client.o opc-client.c

render-utils.o: render-utils.c
	gcc -Wall -c -lm -o render-utils.o render-utils.c

renderer_astern.o: renderer_astern.c renderer_ball.h render-utils.o
	gcc -Wall -c -lm -o $@ $<
renderer_ball.o: renderer_ball.c renderer_ball.h render-utils.o
	gcc -Wall -c -lm -o $@ $<
