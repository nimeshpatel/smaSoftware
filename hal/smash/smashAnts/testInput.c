#include <curses.h>
#define MAXHEXVAL 255
main(int argc,char **argv[]) {
WINDOW *mywin;
int value;
char *inputString;
/*
char inputString[100];
*/
printf("argc=%d\n",argc);
printf("hit return to continue.\n");
getchar();
exit(-1);

initscr();
mywin=newwin(10,10,5,10);
/*
do {
*/
erase();
mvprintw(LINES - 1, 0, "Enter value up to %d: ", MAXHEXVAL);
refresh();
/*
scanw("%d",&value);
wscanw(mywin,"%s",inputString);
*/
scanf("%s",inputString);
printf("ENtered value: %s with argc=%d\n",inputString,argc);
/*
} while (value < 0 || value > MAXHEXVAL);
mvprintw (10,10, "Value is %x Hex", value);
*/
refresh();
getch();
endwin();
}
