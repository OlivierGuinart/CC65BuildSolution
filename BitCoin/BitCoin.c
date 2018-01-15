#include <stdio.h>
#include <conio.h>


/* 1MHZ timing loop settings - increase for faster machines */
#define YTIME 60L
#define XTIME 2L
char wait(unsigned duration)
{
	char c = 0;
	long y, x;

	while (duration > 0) {
		for (y = 0; y < YTIME; y++) {
			for (x = 0; x < XTIME; x++) {
				if (kbhit()) {
					c = cgetc();
					/* clear keyboard buffer */
					while (kbhit())cgetc();
					return c;
				}
			}
		}
		duration--;
	}
	return c;
}

void main(void)
{
	int i, j;
	for (i = 21; ; i++)
	{
		for (j = 0; j < 60; j++)
		{
			printf("m 09:%d:%02d Speed 0.01 h/s [A8+0:R0+0:F0]", i, j);
			wait(9);
		}
	}
}
