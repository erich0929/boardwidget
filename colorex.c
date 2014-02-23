#include <stdio.h>
#include <ncurses.h>

int main(int argc, const char *argv[])
{
	initscr ();
	cbreak ();
	start_color ();
	use_default_colors ();
	refresh ();

	init_pair (1, COLOR_RED, COLOR_GREEN);
	WINDOW* mainwnd = newwin (3,3,1,1);
	wbkgd (mainwnd, COLOR_PAIR(0));
	wprintw (mainwnd, "see anything?");
	wrefresh (mainwnd);

	int ch;
	while ((ch = getch ()) != 'a') {
			/* do nothing */
	}

	delwin (mainwnd);
	endwin ();
	return 0;
}
