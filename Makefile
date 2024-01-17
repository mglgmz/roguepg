build:	
	gcc -std=c99 ./src/*.c -lraylib -lm -o roguepg

run:
	./roguepg

clean:
	rm ./roguepg