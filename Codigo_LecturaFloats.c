#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

float acum=0.0;
float multiplicador=1.0;
char chr;
int flag=0;

void main() {
	do {
		chr=getch();
		if (chr!=13) {
			if (chr=='.') {
				if (flag==0) {
					flag=1;
					printf("%c",chr);
				}
			} else if ((chr>='0')&&(chr<='9')) {
				printf("%c",chr);
				if (flag==0) {
					acum*=10;
					acum+=(chr-'0');
				} else {
					multiplicador*=(float)0.1;
					acum+=((chr-'0')*multiplicador);
				}
			}
		}
	} while(chr!=13);
	printf("\nnumero=%f\n",acum);
}