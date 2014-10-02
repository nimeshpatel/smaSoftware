#include <stdio.h>
#include <unistd.h>
#include "tek_driv.h"

main() {
	xterm();
	setup();
	clear();
	move(100,100);
	line(924, 100);
	line(924, 700);
	line(100, 700);
	line(100, 100);
	move(512,400);
	center("Hello World");
	cleanup();
	usleep(100000);
	printf("Hello from the text window\n");
	printf("To hide the graphics window, use '^Center Button'\n");
}
