#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

char chr=0;

void main() {
	while (chr!=13) {
		if (kbhit()) {
			chr=getch();
			if (chr!=13) {
				printf("%c",chr);
			}
		}
	}
	printf("\nFin\n");
}