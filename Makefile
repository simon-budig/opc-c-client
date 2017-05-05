all: renderer-all 

renderer-simon: opc-client.o render-utils.o renderer-simon.c
	gcc -Wall -g -lm `pkg-config --libs --cflags libpng` -o renderer-simon opc-client.o render-utils.o renderer-simon.c

renderer-all: renderer-all.c opc-client.o render-utils.o renderer_astern.o renderer_ball.o renderer_pong.o
	gcc -Wall -g -lm `pkg-config --libs --cflags libpng` -o $@  $^

renderer-fun: renderer-fun.c
	gcc -Wall -g -lm -o renderer-fun renderer-fun.c 

opc-client.o: opc-client.c
	gcc -Wall -g -c -o opc-client.o opc-client.c

render-utils.o: render-utils.c
	gcc -Wall -g -c -lm -o render-utils.o render-utils.c

renderer_astern.o: renderer_astern.c renderer_ball.h render-utils.o
	gcc -Wall -g -c -lm -o $@ $<
renderer_ball.o: renderer_ball.c renderer_ball.h render-utils.o
	gcc -Wall -g -c -lm -o $@ $<
renderer_pong.o: renderer_pong.c
	gcc -Wall -g -c -lm -o $@ $<
