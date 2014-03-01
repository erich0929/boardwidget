#include <stdio.h>
#include <wchar.h>
#include <locale.h>

#include "boardwidget.h"

/* -------------------------- <USER AREA> -------------------------- */
typedef struct _MYDATA {
	int no;
	wchar_t* name;
	int age;
} MYDATA;

static void printHeader (WINDOW* wnd, int colindex) {

	wprintw (wnd, "No.    NAME\t\tAGE");
}	

static void printData (WINDOW* wnd, gpointer data, int colindex) {

	MYDATA* mydata = (MYDATA*) data;
	wattron (wnd, A_BOLD);
	wprintw (wnd, "NO.%-4d%s\t\t%d",  mydata -> no,
			mydata -> name,
			mydata -> age);


}

static gint sorting_by_age (gpointer a, gpointer b) {

	MYDATA* dataA = *(MYDATA**) a;
	MYDATA* dataB = *(MYDATA**) b;

	int tempA = dataA -> age;
	int tempB = dataB -> age;

	if (tempA - tempB > 0) return 1;
	else if (tempA - tempB == 0) return 0;
	else if (tempA - tempB < 0) return -1;
}		

MYDATA mydata [] = {{1, (wchar_t*) "수혜", 25},
					{2, (wchar_t*) "광로", 30},
					{3, (wchar_t*) "사츠코", 56},
					{4, (wchar_t*) "abe", 52},
					{5, (wchar_t*) "법륜", 65},
					{6, (wchar_t*) "재은", 2},
					{7, (wchar_t*) "효주", 2},
					{8, (wchar_t*) "이니에스타", 27},
					{9, (wchar_t*) "messi", 26},
					{10, (wchar_t*) "xabi", 36},
					{11, (wchar_t*) "산체스", 28}};

void init_scr()
{
	initscr();
	start_color(); 
	curs_set(0);
	noecho();
	nodelay (stdscr, TRUE); 
	keypad(stdscr, TRUE);
	use_default_colors (); 
	short r, g, b;
	r=81, g=243, b=22;
	color_content (COLOR_GREEN, &r, &g, &b);
	r=8, g=35, b=77;
	color_content (COLOR_BLUE, &r, &g, &b);
	r=249, g=249, b=41;
	color_content (COLOR_YELLOW, &r, &g, &b);
	init_pair (1, COLOR_YELLOW, COLOR_GREEN);
	init_pair (2, COLOR_YELLOW, COLOR_BLUE);
	refresh ();
}

int main(int argc, const char *argv[])
{
	setlocale(LC_ALL, "ko_KR.utf8");
	init_scr ();
	cbreak ();	
	/* signal(SIGWINCH, resizeHandler); */

	GPtrArray* datatable = g_ptr_array_new ();

	int length = sizeof (mydata) / sizeof (MYDATA);

	int i; 
	for (i = 0; i < length; i++) {
		g_ptr_array_add (datatable, &mydata [i]);
	}
	
	POINT_INFO point_info;
	point_info.origin_x = 1;
	point_info.origin_y = 1;
	point_info.base_color = COLOR_PAIR (0);
	point_info.selected_color = COLOR_PAIR (1);
	point_info.x_from_origin = 0;
	point_info.y_from_origin = 0;

	BOARD_WIDGET* board = new_board (board, 5, 1, 1, 20, &point_info, datatable, printHeader, printData);
	/*	refresh (); */
	
	set_rowIndex (board, 0);
	update_board (board);
	inactivate_board (board);

	int ch ;
	while ((ch = getch ()) != 'a') {		
		switch (ch) {
			case 'b' :
				board_eventhandler (board);
				
				break;

			case 's' :
				g_ptr_array_sort (board -> dataTable, (GCompareFunc) sorting_by_age);
				clear_board (board);
				update_board (board);
				break;

			default :
				break;
		}
		usleep (1000);	
	} 

	g_ptr_array_free (datatable, TRUE);
	del_board (board);
	endwin ();
	return 0;
}
