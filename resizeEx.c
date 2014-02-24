#include <stdio.h>
#include <ncurses.h>

int main(int argc, const char *argv[])
{
	initscr ();
	start_color ();
	use_default_colors ();
	refresh ();
	WINDOW* mainwnd = newwin (10, 30, 1, 1);
	box (mainwnd, ACS_VLINE, ACS_HLINE);
	WINDOW* subwnd = subwin (mainwnd, 3, 10, 1, 1);
	box (subwnd, ACS_VLINE, ACS_HLINE);
	wrefresh (subwnd);
	wrefresh (mainwnd);


	int ch;

	while ((ch = getch ()) != 'a') {
		switch (ch) {
			case KEY_RESIZE :
				printw ("resize");
				wclear (subwnd);
				wrefresh (subwnd);
				delwin (subwnd);
				wclear (mainwnd);
				wrefresh (mainwnd);
				delwin (mainwnd);
				/*refresh (); */
				mainwnd = newwin (10, 30, 1, 1);
			    box (mainwnd, ACS_VLINE, ACS_HLINE);
				wrefresh (mainwnd);
				subwnd = subwin (mainwnd, 3, 10, 1, 1);
			    box (subwnd, ACS_VLINE, ACS_HLINE);
				wrefresh (subwnd);
		}
	}

	endwin ();

	return 0;
}
