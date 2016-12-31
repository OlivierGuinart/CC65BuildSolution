/* ------------------------------------------------------------------------
System       : cc65 cross-development environment
Platform     : Apple IIe
Program      : TextSlides (C) Copyright Olivier Guinart 2015.
               All rights reserved.

Description  : Apple //e ProDOS 8 Text-based Slideshow Program

Written by   : Olivier Guinart
Date Written : April 2015
Revision     : 1.0 First Release (CC65)
Date Revised : xxxxx

Licence      : You may use this program for whatever you wish as long
               as you agree that Olivier Guinart has no warranty or
               liability obligations whatsoever from said use.
------------------------------------------------------------------------ */

#ifndef __APPLE2__
#define __APPLE2__
#endif

#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <errno.h>
#include <apple2enh.h>
#include "_file.h"


#define iMaxStrLen 256
#define ELLIPSIS "...\0" // Used to truncate the slide title if it's longer than screen width

unsigned char XSize, YSize; // Used to store screen size
int iSlidesPosition[iMaxStrLen]; // Used to store slides position within Diapos.txt for fseek purposes
char strTemp[iMaxStrLen]; // Temp string
int ch; // Temp char
int iPos; // Temp var that tracks the current line of the slides txt file
int iCurrentSlide; // Tracks the current slide being displayed

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

void sound(int duration, int pitch)
{
    char *speaker = (char*) 0xC030;
    int i, j;
    char c;

    for (i = 0; i < duration; i++)
    {
      c += *speaker;
      for (j = 0; j < pitch; j++);
    }
}

/* fgets for Apple II text files - read native apple II text files properly!
the cc65 version doesn't! Which means that you need to use unix text files
unless you roll your own fgets... */
/* Created in Windows, text file lines end with CRLF, chr(13) + chr(10) */
char* __fastcall__ fgets(char* s, unsigned size, register FILE* f)
{
	register char* p = s;
	unsigned i;
	int c;

	if (size == 0) {
		/* Invalid size */
		return (char*)_seterrno(EINVAL);
	}

	/* Read input */
	i = 0;
	c = fgetc(f);
	while (c != EOF && ((char)c != (char)13)) {
		//printf("'%i' ",c);
		/* One char more */
		*p = c;
		++p;
		++i;
		c = fgetc(f);
	}

	if (c == EOF) {
		// printf("DEBUG: in fgets with EOF%c",(char)13);
		/* Error or EOF */
		if ((f->f_flags & _FERROR) != 0 || i == 0) {
			/* ERROR or EOF on first char */
			// printf("DEBUG: in fgets returning %i%c",NULL,(char)13);
			*p = '\0';
			return NULL;
		}
	}
	/* On lit le charactere LF, on l'ignore */
	c = fgetc(f);
	/* Terminate the string */
	*p = '\0';

	/* Done */
	return s;
}

////////////////////////////////////////////////////////////////////////////////////
/// Repositions the file pointer to line iLine
/// iLine line number were the next read will happen
/// fp file from which to read the iBullets points
////////////////////////////////////////////////////////////////////////////////////
int PositionFilePointer(int iLine, FILE *fp)
{
	int i;

	// If we are here, it means we need to close the file, re-open it and reposition the pointer to the line #iLine...
	fclose(fp);
	/* We test/open the files of slides */
	if ((fp = fopen("Diapos.txt", "r")) == NULL) 
	{
		printf("Files 'Diapos.txt' couldn't be opened. Press a key to exit.");
		cgetc();
		return EXIT_FAILURE;
	}
	for (i = 1; i < iLine; i++)
	{
		fgets(strTemp, iMaxStrLen, fp);
	}
	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
/// Print slide frame
///////////////////////////////////////////////////////////////////////////////
void SlideFrame()
{
	/* Set screen colors, hide the cursor */
	//textcolor(COLOR_WHITE);
	//bordercolor(COLOR_BLACK);
	//bgcolor(COLOR_BLACK);
	//cursor(0);

	/* Clear the screen, put cursor in upper left corner */
	clrscr();

	/* Draw a border around the screen */

	/* Top line */
	cputc(CH_ULCORNER);
	chline(XSize - 2);
	cputc(CH_URCORNER);

	/* Vertical line, left side */
	cvlinexy(0, 1, YSize - 2);

	/* Bottom line */
	cputc(CH_LLCORNER);
	chline(XSize - 2);
	cputc(CH_LRCORNER);

	/* Vertical line, right side */
	cvlinexy(XSize - 1, 1, YSize - 2);
}

///////////////////////////////////////////////////////////////////////////////
/// Slide #0 with title and subtitle of the presentation
/// strTitles, 2 strings arrays for title and subtitle
///////////////////////////////////////////////////////////////////////////////
void ShowOpeningSlide(char strTitles[2][iMaxStrLen])
{
	int ilenTitle, ilenSubTitle, iMidX, iMidY;

	ilenTitle = strlen(strTitles[0]);
	ilenSubTitle = strlen(strTitles[1]);

	// If Title and subtitle are empty, no need to even display an opening slides...
	if ((ilenTitle + ilenSubTitle) > 0)
	{
		SlideFrame();

		// printf("DEBUG: ilen '%i'%c", ilen, (char)13); 
		// printf("DEBUG: XSize '%i'%c", XSize, (char)13);
		iMidX = XSize / 2;
		iMidY = YSize / 2;

		// printf("DEBUG: iMidX '%i'%c", iMidX, (char)13);
		// Printing first the frame of the title
		/* Vertical line, left side (x, y, longueur) */
		// cvlinexy(iMidX - 2, 1, 1);
		/* Ligne horizontale sous le texte */
		// chlinexy(iMidX - 1, 2, ilen + 2);
		/* Vertical line, right side */
		// cvlinexy(iMidX + ilen + 1, 1, 1);
		// Handling the title now, it will print from both right and left sides, animation effect...
		gotoxy(iMidX - (ilenTitle / 2), iMidY - 4); // -4 to print a little bit above the middle
		printf("%s", strTitles[0]);
		gotoxy(iMidX - (ilenSubTitle / 2), iMidY);
		printf("%s", strTitles[1]);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Reads the slides file and call the ShowOpeningSlide procedure
/// strTitles, 2 strings arrays for title and subtitle
/// fp the slides text file
///////////////////////////////////////////////////////////////////////////////
int OpenAndShowOpeningSlide(char strTitles[2][iMaxStrLen], FILE *fp)
{
	iPos = 0; // iPos will record the current line of the slides txt file
	iCurrentSlide = 0;
	PositionFilePointer(iPos, fp);
	fgets(strTitles[0], iMaxStrLen, fp);
	iPos++;
	iSlidesPosition[iCurrentSlide] = iPos; // the opening slide is always on first (and second) line of the slides text file
	fgets(strTitles[1], iMaxStrLen, fp);
	iPos++;
	ShowOpeningSlide(strTitles);
	return cgetc();
}

///////////////////////////////////////////////////////////////////////////////
/// Print slide titles, below top frame line, centered and boxed
/// strTitle, the title of the slide read in Main() loop
///////////////////////////////////////////////////////////////////////////////
void SlideTitle(char strTitle[iMaxStrLen])
{
	int i, j, ilen, iMidX;
	int iMargin = 5;
	register char* p;

	ilen = strlen(strTitle);
	// printf("DEBUG: ilen '%i'%c", ilen, (char)13); 
	// printf("DEBUG: XSize '%i'%c", XSize, (char)13);
	// We need to test if the title is longer than the screen width + some buffer = 5
	// If yes, we truncate with ellipsis...
	if (ilen >= (XSize - iMargin))
	{
		p = strTitle + XSize - iMargin - strlen(ELLIPSIS);
		memcpy(p, ELLIPSIS, strlen(ELLIPSIS));
		ilen = strlen(strTitle);
	}
	iMidX = (XSize - ilen) / 2;
	// printf("DEBUG: iMidX '%i'%c", iMidX, (char)13);
	// printf("DEBUG: ilen = %i%c",ilen,(char)13);
	// Printing first the frame of the title
	/* Vertical line, left side (x, y, longueur) */
	cvlinexy(iMidX - 2, 1, 1);
	/* Ligne horizontale sous le texte */
	chlinexy(iMidX - 1 , 2, ilen + 2);
	/* Vertical line, right side */
	cvlinexy(iMidX + ilen + 1, 1, 1);
	// Handling the title now, it will print from both right and left sides, animation effect...
	i = 0;
	j = ilen - 1; // zero-index string array...
	while (i <= j) {
		gotoxy(iMidX + i, 1);
		printf("%c", strTitle[i]);
		gotoxy(iMidX + j, 1);
		printf("%c", strTitle[j]);
		sound(5, 30);
		wait(1);
		i++;
		j--;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
/// Reads bullets from text files and prints them on screen, with margins x = 5, start y = 7
/// iBullets number of bullets/points of the slide
/// fp file from which to read the iBullets points
/////////////////////////////////////////////////////////////////////////////////////
void SlideBullets(int iBullets, FILE *fp)
{
	const int iHmarge = 5;
	const int iVmarge = 5;
	int i;

	for (i = 0; i < iBullets; i++) {
		gotoxy(iHmarge, iVmarge + i);
		fgets(strTemp, iMaxStrLen, fp);
		printf("%s", strTemp);
	}
}

////////////////////////////////////////////////////////////////////////////////////
/// Draws the whole slide
/// strTitle, the title of the slide read in Main() loop
/// iBullets number of bullets/points of the slide
/// fp file from which to read the iBullets points
////////////////////////////////////////////////////////////////////////////////////
void TitleAndContentSlide(char strTitle[iMaxStrLen], int iBullets, FILE *fp)
{
	// We draw the frame of the slide
	SlideFrame();
	// We print the slide title
	SlideTitle(strTitle);
	// We load and print iBullets bullets from text file fp
	SlideBullets(iBullets, fp);
}

////////////////////////////////////////////////////////////////////////////////////
/// Main :)
////////////////////////////////////////////////////////////////////////////////////
int main(void)
{
	char strTitles[2][iMaxStrLen];
	char strSlideTitle[iMaxStrLen];
	int iBullets;
	FILE *fp;

	/* We test/open the files of slides */
	if ((fp = fopen("Diapos.txt", "r")) == NULL) {
		printf("Files 'Diapos.txt' couldn't be opened. Press a key to exit.");
		cgetc();
		return EXIT_FAILURE;
	}

	videomode(VIDEOMODE_80COL);
	
	/* Ask for the screen size */
	screensize(&XSize, &YSize);

	// Slides are read from file
	// File structure:
	// 1st line = Presentation title (to be printed in Opening Slide)
	// 2nd line = Presentation subtitle (to be printed in Opening Slide)
	// 3rd line = Title of the first slide
	// 4th line = n number of bullets for that slide
	// n following lines = bullets of the slide
	// Repeat with 3rd, 4th and n lines structure for next slides

	ch = OpenAndShowOpeningSlide(strTitles, fp);

	// First slide, First line = title...
	while (fgets(strSlideTitle, iMaxStrLen, fp) != NULL) {
		// We record the slide position in the slides text file
		iCurrentSlide++;
		iPos++; // +1 because of the title
		// printf("DEBUG: iCurrentSlide='%i' iPos='%i'%c", iCurrentSlide, iPos, (char)13);
		// cgetc();
		iSlidesPosition[iCurrentSlide] = iPos; // We save the line position of the slide title
		// Second line = Number of bullets in current slide...
		fgets(strTemp, iMaxStrLen, fp);
		iPos++; // +1 because of the count of bullets
		// printf("DEBUG: strTemp '%s'%c", strTemp, (char)13);
		iBullets = atoi(strTemp);
		iPos+=iBullets; // And adding the bullets themselves
		TitleAndContentSlide(strSlideTitle, iBullets, fp);
		// We pause before going to next slide
		ch = cgetc();
		if (ch == CH_ESC) // If escape key pressed, exit loop
		{
			break;
		}
		if (ch == CH_CURS_LEFT) // Back to previous slide with left arrow letter (for now)
		{
			iCurrentSlide--;
			if (iCurrentSlide > 0)
			{
				iPos = iSlidesPosition[iCurrentSlide];
				PositionFilePointer(iPos, fp);
				// When we entered the loop the first time, we weren't yet on the line, now we are, so 
				// to avoid counting it twice, we decrease it here by one...
				iPos--; 
				// And same for the slide #...
				iCurrentSlide--;
			}
			else
			{
				// we are back to the opening slide
				// and we keep looping if users keeps pressing left arrow
				ch = OpenAndShowOpeningSlide(strTitles, fp);
				while (ch == CH_CURS_LEFT)
				{
					ch = cgetc();
				};
				// but we exit loop if ESC
				if (ch == CH_ESC)
				{
					break;
				}
			}
		}
	}

	// Close the file
	fclose(fp);
	clrscr();
	return EXIT_SUCCESS;
}
