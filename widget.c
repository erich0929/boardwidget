#include <stdio.h>
#include <glib.h>
#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>

typedef struct _POINT_INFO {

	int origin_x;
	int origin_y;
	int x_from_origin;
	int y_from_origin;

} POINT_INFO;

typedef void (*PRINT_HEADER_FUNC) (WINDOW*, int);
typedef void (*PRINT_DATA_FUNC) (WINDOW*, gpointer, int);

typedef struct _WIDGET {
	WINDOW* mainWnd;
	WINDOW* headerWnd;

	GPtrArray* wndTable;
	GPtrArray* dataTable;
	PRINT_HEADER_FUNC printHeader;
	PRINT_DATA_FUNC printData;
	guint firstrow_index;
	guint lastrow_index;

	int row;
	int col;
	int row_width;
	int col_width;

	POINT_INFO* point_info;
	POINT_INFO* origin_point_info;

	chtype base_color;
	chtype selected_color;
	guint selected_index;

	bool wndFlag;
	bool dataFlag;	
} WIDGET;



WIDGET* new_widget (WIDGET* widget, int row, int col,
		int row_width, int col_width, POINT_INFO* point_info,
	   	GPtrArray* dataTable, PRINT_HEADER_FUNC printHeader,
	   	PRINT_DATA_FUNC printData) {

	int i, j;
	int maxX, maxY;
	getmaxyx (stdscr, maxY, maxX); /* It's used for x_from_origin and y_from_origin
									* in the point_info Structure */

	/* Allocate memory for new widget */
	widget = (WIDGET*) malloc (sizeof (WIDGET));

	/* Initialize fields value */
	widget -> row = row;
	widget -> col = col;
	widget -> row_width = row_width;
	widget -> col_width = col_width;
	widget -> base_color = COLOR_PAIR (0);  
	widget -> selected_color = COLOR_PAIR (2);
	widget -> selected_index = 0;

	if (point_info -> x_from_origin == 0 && point_info -> y_from_origin == 0) {

		widget -> point_info = (POINT_INFO*) malloc (sizeof (POINT_INFO));
		widget -> point_info -> origin_x = point_info -> origin_x;
		widget -> point_info -> origin_y = point_info -> origin_y;
		widget -> point_info -> x_from_origin = maxX - (widget -> point_info -> origin_x);
		widget -> point_info -> y_from_origin = maxY - (widget -> point_info -> origin_y);
		
		widget -> origin_point_info = (POINT_INFO*) malloc (sizeof (POINT_INFO));
		widget -> origin_point_info -> origin_x = widget -> point_info -> origin_x;
		widget -> origin_point_info -> origin_y = widget -> point_info -> origin_y;
		widget -> origin_point_info -> x_from_origin = widget -> point_info -> x_from_origin;
		widget -> origin_point_info -> y_from_origin = widget -> point_info -> y_from_origin;

	}
	else {

		widget -> origin_point_info = (POINT_INFO*) malloc (sizeof (POINT_INFO));

		widget -> origin_point_info -> origin_x = point_info -> origin_x;
		widget -> origin_point_info -> origin_y = point_info -> origin_y;
		widget -> origin_point_info -> x_from_origin = point_info -> x_from_origin;
		widget -> origin_point_info -> y_from_origin = point_info -> y_from_origin;
		
		widget -> point_info = (POINT_INFO*) malloc (sizeof (POINT_INFO));
		widget -> point_info -> origin_x = point_info -> origin_x;
		widget -> point_info -> origin_y = point_info -> origin_y;
		widget -> point_info -> x_from_origin = maxX - (widget -> point_info -> origin_x);
		widget -> point_info -> y_from_origin = maxY - (widget -> point_info -> origin_y);

	}

	int add_row = 
		widget -> point_info -> y_from_origin - widget -> origin_point_info -> y_from_origin;
	
	add_row = ((int) (add_row / row_width) > 0) ? (int) (add_row / row_width) : 0;
	
	int add_col_width = 
		widget -> point_info -> x_from_origin - widget -> origin_point_info -> x_from_origin;
	
	add_col_width = (add_col_width > 0) ? add_col_width : 0;

	widget -> dataTable = dataTable;
	widget -> printHeader = printHeader;
	widget -> printData = printData;

	widget -> wndFlag = true;
	widget -> dataFlag = true;

	/* Create new main window */
	widget -> mainWnd = newwin (
						widget -> row * widget -> row_width + 
						add_row * row_width + 3,
						widget -> col * widget -> col_width + 
						add_col_width + 2,
					   	widget -> point_info -> origin_y,
					   	widget -> point_info -> origin_x);

	wbkgd (widget -> mainWnd, widget -> base_color);
	box (widget -> mainWnd, ACS_VLINE, ACS_HLINE);
	widget -> wndTable = g_ptr_array_new ();

	/* Create new header window */
	widget -> headerWnd = subwin (widget -> mainWnd, 1, widget -> col_width + add_col_width, 2, 2);
	for (j = 0; j < widget -> col; j++) {
		widget -> printHeader (widget -> headerWnd, j);
		wrefresh (widget -> headerWnd);	
	}
	/* Declare container which have windows in a specific row */
	WINDOW** rowContainer;

	/* Create new subwindow for dataTable */
	for (i = 0; i < widget -> row + add_row; i++) {
		rowContainer = (WINDOW**) malloc (col * sizeof (WINDOW *));
		for (j = 0; j < widget -> col; j++) {
			rowContainer [j] = subwin (widget -> mainWnd, widget -> row_width, 
										widget -> col_width + add_col_width,
										i * widget -> row_width + 3, 
										j * widget -> col_width + 2);
		}
		g_ptr_array_add (widget -> wndTable, rowContainer);
	}
	return widget;
}

void set_base_color (WIDGET* widget, chtype color) {

	widget -> base_color = color;

}

void set_selected_color (WIDGET* widget, chtype color) {

	widget -> selected_color = color;

}

void set_rowIndex (WIDGET* widget, int index) {

	/* ------------- <SET selected_index> ----------------- */
	guint length = widget -> wndTable -> len;
	if (index <= 0) {
		widget -> selected_index = 0;
	}
	else if (index > (int) length - 1) {
		widget -> selected_index = length - 1;
	}
	else widget -> selected_index = index;
	/* ------------- </SET selected_index> ------------------ */

	/* ------------- <SET firstrow_index and lastrow_index> -------------- */
	if (index <= 0) {
		widget -> firstrow_index = ((int) (widget -> firstrow_index) + index < 0) ? 0 : (int) widget -> firstrow_index + index;
		widget -> lastrow_index = (widget -> dataTable -> len < widget -> wndTable -> len) ?
			widget -> firstrow_index + widget -> dataTable -> len -1 :
			widget -> firstrow_index + widget -> wndTable -> len -1;
	}
	else if (index > (int) length - 1) {
		widget -> lastrow_index = ((int) (widget -> lastrow_index) + (index - ((int) length - 1)) > widget -> dataTable -> len - 1) ?
			widget -> dataTable -> len - 1 : (int) (widget -> lastrow_index) + (index - ((int) length - 1));
		widget -> firstrow_index = (widget -> dataTable -> len < widget -> wndTable -> len) ?
			widget -> lastrow_index - (widget -> dataTable -> len - 1) :
			widget -> lastrow_index - (widget -> wndTable -> len - 1);	
	}

	/* ------------- </SET firstrow_index and lastrow_index> -------------- */
}

/* clear_widget must be called before new setting widget !! */
void clear_widget (WIDGET* widget) {

	WINDOW** rowContainer;
	int i, j;
	/* ------------- <CLEAR VIEW> ------------- */
	if (widget -> wndFlag == true) {
		rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable , widget -> selected_index);
		for (j = 0; j < widget -> col; j++) {
			wbkgd (rowContainer [j], widget -> base_color);
			wrefresh (rowContainer [j]);
		}
	}


	/* ------------- </CLEAR VIEW> ------------- */


	/* ------------- <CLEAR DATA> ---------------- */
	if (widget -> dataFlag == true) {
		for (i = 0; i < widget -> row; i++) {
			rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, i);
			for (j = 0; j < widget -> col; j++) {
				wdeleteln (rowContainer [j]); /* clear data */
				wmove (rowContainer [j], 0 , 0);
				wrefresh (rowContainer [j]); 
			}
		}
	}


	/* ------------- </CLEAR DATA> ---------------- */
}
void update_widget (WIDGET* widget) {

	int i, j, k;
	WINDOW** rowContainer;	

	/* ------------- <VIEW UPDATE> --------------- */

	if (widget -> wndFlag == true) {
		rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, widget -> selected_index);

		for (j = 0; j < widget -> col; j++) {
			wbkgd (rowContainer [j], widget -> selected_color);
			wrefresh (rowContainer [j]);
		}
		wrefresh (widget -> mainWnd);
	}

	/* ------------- </VIEW UPDATE> -------------- */

	/* ------------- <DATA UPDATE> -------------- */

	if (widget -> dataFlag == true) {
		gpointer recordset;
		int firstrow_index = (int) widget -> firstrow_index;
		int lastrow_index = (int) widget -> lastrow_index;

		for (i = widget -> firstrow_index, k = 0; i < widget -> lastrow_index + 1; i++, k++) {
			rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, k);
			recordset = g_ptr_array_index (widget -> dataTable, i);
			for (j = 0; j < widget -> col; j++) {
				widget -> printData (rowContainer [j], (gpointer) recordset, j);
				wrefresh (rowContainer [j]);
			}
		}
	}

	/* ------------- </DATA UPDATE> -------------- */

	/* ------------- <SET FLAG MEMBERS> ------------ */

	widget -> wndFlag = true;
	widget -> dataFlag = true;

	/* ------------- </SET FLAG MEMBERS> ------------ */

}			

void inc_rowIndex (WIDGET* widget) {

	/* ------- <SET FLAG MEMBERS> --------  */		
	if (widget -> selected_index == widget -> wndTable -> len - 1)
		widget -> wndFlag = false;
	else widget -> wndFlag = true;

	if (widget -> lastrow_index == widget -> dataTable -> len - 1)
		widget -> dataFlag = false;
	else widget -> dataFlag = true;

	/* -------- </SET FLAG MEMBERS> --------- */

	/* ------ <INCREASE VIEW AND DATA> ------ */

	int index = widget -> selected_index;
	index ++;
	clear_widget (widget);
	set_rowIndex (widget, index);
	update_widget (widget);

	/* ------ </INCREASE VIEW AND DATA> ------ */

}

void dec_rowIndex (WIDGET* widget) {

	/* ------- <SET FLAG MEMBERS> -------- */	

	if (widget -> selected_index == 0)
		widget -> wndFlag = false;
	else widget -> wndFlag = true;

	if (widget -> firstrow_index == 0)
		widget -> dataFlag = false;
	else widget -> dataFlag = true;

	/* ------- </SET FLAG MEMBERS> -------- */

	/* ------- <Decrease selected_index in the view> ------ */

	int index = widget -> selected_index;
	index --;
	clear_widget (widget);
	set_rowIndex (widget, index);
	update_widget (widget);

	/* ------- </Decrease selected_index in the view> ------ */

	/* ------ <Increase fistrow_index in the data> ------ */

	/* ------ </Increase firstrow_index in the data> ----- */

}

void pageup_handler (WIDGET* widget) {

	/* ------- <SET FLAG MEMBERS> -------- */	

	if (widget -> selected_index == 0)
		widget -> wndFlag = false;
	else widget -> wndFlag = true;

	if (widget -> firstrow_index == 0)
		widget -> dataFlag = false;
	else widget -> dataFlag = true;

	/* ------- </SET FLAG MEMBERS> -------- */

	int index = -1 * (int) (widget -> wndTable -> len);
	clear_widget (widget);
	set_rowIndex (widget, index);
	update_widget (widget);

}

void pagedown_handler (WIDGET* widget) {

	/* ------- <SET FLAG MEMBERS> --------  */		
	if (widget -> selected_index == widget -> wndTable -> len - 1)
		widget -> wndFlag = false;
	else widget -> wndFlag = true;

	if (widget -> lastrow_index == widget -> dataTable -> len - 1)
		widget -> dataFlag = false;
	else widget -> dataFlag = true;

	/* -------- </SET FLAG MEMBERS> --------- */

	int index = (int) (widget -> wndTable -> len) * 2 - 1;
	clear_widget (widget);
	set_rowIndex (widget, index);
	update_widget (widget);

}

void resize_handler (WIDGET* widget) {

	WINDOW* mainWnd = widget -> mainWnd;
	WINDOW* headerWnd = widget -> headerWnd;

	GPtrArray* wndTable = widget -> wndTable;
	GPtrArray* dataTable = widget -> dataTable;
	PRINT_HEADER_FUNC printHeader = widget -> printHeader;
	PRINT_DATA_FUNC printData = widget -> printData;
	guint firstrow_index = widget -> firstrow_index;
	guint lastrow_index = widget -> lastrow_index;

	int row = widget -> row;
	int col = widget -> col;
	int row_width = widget -> row_width;
	int col_width = widget -> col_width;
	chtype base_color = widget -> base_color;
	chtype selected_color = widget -> selected_color;
	guint selected_index = widget -> selected_index;

	bool wndFlag;
	bool dataFlag;	

	refresh (); /* It's very essential!! */

	POINT_INFO origin_point_info;
	origin_point_info.origin_x = widget -> origin_point_info -> origin_x;
	origin_point_info.origin_y = widget -> origin_point_info -> origin_y;
	origin_point_info.x_from_origin = widget -> origin_point_info -> x_from_origin;
	origin_point_info.y_from_origin = widget -> origin_point_info -> y_from_origin;

	del_widget (widget);
	widget = new_widget (widget, row, col, row_width, col_width, &origin_point_info
						,dataTable, printHeader, printData); 
	set_rowIndex (widget, selected_index);
	set_selected_color (widget, selected_color);
	set_base_color (widget, base_color);
	update_widget (widget);

	refresh ();


}

void del_widget (WIDGET* widget) {
	WINDOW** rowContainer;
	int i, j;
	for (i = 0; i < widget -> wndTable -> len; i++) {
		rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, i);
		for (j = 0; j < widget -> col; j++) {
	/*		wclear (rowContainer [j]);
			wrefresh (rowContainer [j]); */
			delwin (rowContainer [j]);
		}
	}
	
	wclear (widget -> headerWnd);
	wrefresh (widget -> headerWnd);
	delwin (widget -> headerWnd);
	wclear (widget -> mainWnd);
	wrefresh (widget -> mainWnd);
	delwin (widget -> mainWnd);
	g_ptr_array_free (widget -> wndTable, TRUE);
	free (widget -> point_info);
	free (widget -> origin_point_info);
	free (widget);
}


/* -------------------------- <USER AREA> -------------------------- */
typedef struct _MYDATA {
	int no;
	wchar_t* name;
	int age;
} MYDATA;

static void printHeader (WINDOW* wnd, int colindex) {

	wprintw (wnd, "No.  NAME\tAGE");
}	

static void printData (WINDOW* wnd, gpointer data, int colindex) {

	MYDATA* mydata = (MYDATA*) data;
	wprintw (wnd, "NO.%d %s\t%d", (MYDATA*) mydata -> no,
			(MYDATA*) mydata -> name,
			(MYDATA*) mydata -> age);


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

MYDATA mydata [] = {{1, "수혜", 25},
					{2, "광로", 30},
					{3, "사츠코", 56},
					{4, "아베", 52},
					{5, "법륜", 65},
					{6, "재은", 2},
					{7, "효주", 2}};

void init_scr()
{
	initscr();
	start_color(); 
	curs_set(0);
	noecho();
	nodelay (stdscr, TRUE); 
	keypad(stdscr, TRUE);
	use_default_colors (); 
	init_pair (2, COLOR_YELLOW, COLOR_GREEN);
	refresh ();
}

void resizeHandler () {

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
	point_info.x_from_origin = 0;
	point_info.y_from_origin = 0;

	WIDGET* widget = new_widget (widget, 5, 1, 1, 20, &point_info, datatable, printHeader, printData);
	/*	refresh (); */

	clear_widget (widget);
	set_rowIndex (widget, 0);
	update_widget (widget);

	int ch ;
	while ((ch = getch ()) != 'a') {		
		switch (ch) {
			case KEY_UP :
				dec_rowIndex (widget);
				break;
			case KEY_DOWN :
				inc_rowIndex (widget);
				break;
			case KEY_PPAGE :
				pageup_handler (widget);
				break;
			case KEY_NPAGE :
				pagedown_handler (widget);
				break;
		
			case KEY_RESIZE :
	
				/* clear_widget (widget); */
				resize_handler (widget);
				break;
	
			case 's' :
				g_ptr_array_sort (widget -> dataTable, sorting_by_age);
				clear_widget (widget);
				update_widget (widget);
				break;
			default :
				break;
		}
		usleep (1000);	
	} 

	g_ptr_array_free (datatable, TRUE);
	del_widget (widget);
	endwin ();
	return 0;
}
