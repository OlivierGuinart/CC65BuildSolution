/*
 * "Performance" "comparison". BASIC and C versions on DSK to be launched at the same time...
 */



#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <apple2.h>
// stdio.h est necessaire si appel a printf
// #include <stdio.h>



/*****************************************************************************/
/*     	      	    	  	     Data	     								*/
/*****************************************************************************/

static const char Barres [] = "\\|/-";


/*****************************************************************************/
/*     	      	    	  	     Code	     							     */
/*****************************************************************************/

int main (void)
{
 	int i, j;

    /* Clear the screen, put cursor in upper left corner */
    clrscr ();

    /* Write the animated star in the mid of the screen */
	for (i = 0; i < 1000; i++)
	{
		// for (j = 0; j < iLen; j++)
		for (j = 0; j < 4; j++)
		{
			// gotoxy(iMidX, iMidY);
			gotoxy(20, 12);
			cprintf("%c", Barres[j]);
		}
	}
	// printf("C'est fini !");
    /* Clear the screen again */
    // clrscr ();

    /* Done */
    return EXIT_SUCCESS;
}



