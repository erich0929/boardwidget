#include <stdio.h>
#include <glib.h>
#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>

typedef void (*PRINT_HEADER_FUNC) (WINDOW*, int);
typedef void (*PRINT_DATA_FUNC) (WINDOW*, gpointer, int);

typedef struct _WIDGET {
		GPtrArray* wndTable;
		WINDOW* mainWnd;
		WINDOW* headerWnd;

		GPtrArray* dataTable;
		PRINT_HEADER_FUNC printHeader;
		PRINT_DATA_FUNC printData;
		guint firstrow_index;
		guint lastrow_index;
		
		int row;
		int col;
		int row_width;
		int col_width;
		chtype base_color;
		chtype selected_color;
		guint selected_index;
} WIDGET;

typedef struct _dataContainer {
	
			
} dataContainer;

WIDGET* new_widget (WIDGET* widget, int row, int col,
			   	int row_width, int col_width, GPtrArray* dataTable, PRINT_HEADER_FUNC printHeader, PRINT_DATA_FUNC printData) {
		
		int i, j;

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

		widget -> dataTable = dataTable;
		widget -> printHeader = printHeader;
		widget -> printData = printData;
		
		/* Create new main window */
		widget -> mainWnd = newwin (widget -> row * widget -> row_width + 3,
					   	widget -> col * widget -> col_width + 2, 1, 1);
		wbkgd (widget -> mainWnd, widget -> base_color);
		box (widget -> mainWnd, ACS_VLINE, ACS_HLINE);
		widget -> wndTable = g_ptr_array_new ();
	
		/* Create new header window */
		widget -> headerWnd = subwin (widget -> mainWnd, 1, widget -> col_width, 2, 2);
		for (j = 0; j < widget -> col; j++) {
				widget -> printHeader (widget -> headerWnd, j);
				wrefresh (widget -> headerWnd);	
		}
		/* Declare container which have windows in a specific row */
		WINDOW** rowContainer;
		
		/* Create new subwindow for dataTable */
		for (i = 0; i < widget -> row; i++) {
			rowContainer = (WINDOW**) malloc (col * sizeof (WINDOW *));
			for (j = 0; j < widget -> col; j++) {
				rowContainer [j] = subwin (widget -> mainWnd, 1, 
								widget -> col_width,
							   	i * widget -> row_width + 3, 
								j * widget -> col_width + 2);
			}
			g_ptr_array_add (widget -> wndTable, rowContainer);
		}
		return widget;
}

void show_widget (WIDGET* widget, int row, int col) {
	
		WINDOW** rowContainer;
		int i, j;
		for (i = 0; i < row; i++) {
			rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, i);
			for (j = 0; j < col; j++) {
					wrefresh (rowContainer [j]);
			}
		}
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

		/* ------------- <CLEAR VIEW AND DATA> ---------------- */
		
		WINDOW** rowContainer;
	
		int i, j;
		for (i = 0; i < widget -> row; i++) {
			rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, i);
			for (j = 0; j < widget -> col; j++) {
					/* wbkgd (rowContainer [j], widget -> base_color); */
					if (i == widget -> selected_index)
							wbkgd (rowContainer [j], widget -> base_color);
					wdeleteln (rowContainer [j]); /* clear data */
					wmove (rowContainer [j], 0 , 0);
					wrefresh (rowContainer [j]); 
			}
		}
		wbkgd (widget -> mainWnd, widget -> base_color);
		wrefresh (widget -> mainWnd);

		/* ------------- </CLEAR VIEW AND DATA> ---------------- */
}
void update_widget (WIDGET* widget) {
	
		int i, j, k;
		WINDOW** rowContainer;	

		/* ------------- <VIEW UPDATE> --------------- */
		rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, widget -> selected_index);
	
		for (j = 0; j < widget -> col; j++) {
				wbkgd (rowContainer [j], widget -> selected_color);
				wrefresh (rowContainer [j]);
		}
		wrefresh (widget -> mainWnd);

		/* ------------- </VIEW UPDATE> -------------- */

		/* ------------- <DATA UPDATE> -------------- */
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
		/* ------------- </DATA UPDATE> -------------- */

		
}			

void inc_rowIndex (WIDGET* widget) {
		
		/* ------ <Increase selected_index in the view> ------ */
		
		int index = widget -> selected_index;
		index ++;
		clear_widget (widget);
		set_rowIndex (widget, index);
		update_widget (widget);

		/* ------ </Increase selected_index in the view> ------ */

		/* ------ <Increase fistrow_index in the data> ------ */

		/* ------ </Increase firstrow_index in the data> ----- */
}

void dec_rowIndex (WIDGET* widget) {
	
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
void del_widget (WIDGET* widget) {
		WINDOW** rowContainer;
		int i, j;
		for (i = 0; i < widget -> row; i++) {
			rowContainer = (WINDOW**) g_ptr_array_index (widget -> wndTable, i);
			for (j = 0; j < widget -> col; j++) {
					delwin (rowContainer [j]);
			}
		}
		delwin (widget -> mainWnd);
		g_ptr_array_free (widget -> wndTable, TRUE);
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

static gint sorting_by_no (gpointer a, gpointer b) {
	   	
		MYDATA* dataA = *(MYDATA**) a;
		MYDATA* dataB = *(MYDATA**) b;
		
		int tempA = dataA -> no;
		int tempB = dataB -> no;

		if (tempA - tempB > 0) return 1;
		else if (tempA - tempB == 0) return 0;
		else if (tempA - tempB < 0) return -1;
}		

MYDATA mydata [] = {{5, "수혜", 25},
					{2, "광로", 30},
					{7, "사츠코", 56},
					{6, "아베", 52}};
void init_scr()
{
		initscr();
		start_color(); 
		curs_set(0); 
		noecho();
		nodelay (stdscr, TRUE);
		/* halfdelay (1);	*/
		keypad(stdscr, TRUE);
		use_default_colors (); 
		init_pair (2, COLOR_YELLOW, COLOR_GREEN);
		refresh ();
}

int main(int argc, const char *argv[])
{
	setlocale(LC_ALL, "ko_KR.utf8");
	init_scr ();
	cbreak ();	

	GPtrArray* datatable = g_ptr_array_new ();
	g_ptr_array_add (datatable, &mydata [0]);
	g_ptr_array_add (datatable, &mydata [1]);
	g_ptr_array_add (datatable, &mydata [2]);
	g_ptr_array_add (datatable, &mydata [3]);

	WIDGET* widget = new_widget (widget, 5, 1, 1, 20, datatable, printHeader, printData);
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
			case 's' :
				g_ptr_array_sort (widget -> dataTable, sorting_by_no);
				clear_widget (widget);
				update_widget (widget);

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
