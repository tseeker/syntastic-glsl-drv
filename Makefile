check-glsl:	check_glsl.c
	$(CC) -g -O0 -lGL -lX11 -o check-glsl check_glsl.c

clean:
	rm -f check-glsl
