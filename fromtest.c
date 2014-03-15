#include <stdio.h>
#include <ncurses.h>
#include "boardwidget.h"
typedef struct _STOCK {
	char date[50];
	float open;
	float high;
	float low;
	float close;
	float volume;
	float adj_close;
}STOCK;


static void printHeader_date (WINDOW* wnd, int colindex) {
	wprintw (wnd, "Select Month");
	wrefresh (wnd);
}

static void printData_date (WINDOW* wnd, gpointer data, int colindex) {
	int i = *((int*) data);

	wprintw (wnd, "%-2dth Month", i);
}

int main(void)
{
	GPtrArray* date = g_ptr_array_new ();
	BOARD_WIDGET* dateInput;

	init_scr ();
	cbreak ();
	refresh ();

	int i;
	for (i = 1; i < 13; i++) {
		g_ptr_array_add (date, &i);
	}

	POINT_INFO dataInput_pinfo;

	dataInput_pinfo.origin_x = 1;
	dataInput_pinfo.origin_y = 1;
	dataInput_pinfo.base_color = COLOR_PAIR (0);
	dataInput_pinfo.selected_color = COLOR_PAIR (1);
	dataInput_pinfo.x_from_origin = 0;
	dataInput_pinfo.y_from_origin = 0;

	dateInput = new_board (dateInput, 5, 1, 1, 10, &dataInput_pinfo, date, printHeader_date, printData_date);
	set_rowIndex (dateInput, 0);
	update_board (dateInput);
	
	board_eventhandler (dateInput);
	
	del_board (dateInput);
	
	endwin();
	return 0;
}
