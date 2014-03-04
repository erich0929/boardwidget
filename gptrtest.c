#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef struct _STOCK {
	char date[50];
	float open;
	float high;
	float low;
	float close;
	float volume;
	float adj_close;
}STOCK;

void parse_csv (char* file, GPtrArray* data);
void printdata (GPtrArray* data);
void printint (GPtrArray* data);
void func (GPtrArray* data);


STOCK  mydata [] = {
	{"1986-06-13", 52.75f, 54.13f, 52.75f, 53.88f, 602400, 2.81f},
	{"1986-06-12", 53.5f, 53.5f, 51.75f, 52.38f, 348800, 2.73f}
};


int main(void)
{

	GPtrArray* data;
	data = g_ptr_array_new ();
	/* func (data); */
	 parse_csv ("/home/erich0929/문서/table.csv", data); 
	/* printint (data);	*/
	/* g_ptr_array_add (data, &mydata [0]);
	g_ptr_array_add (data, &mydata [1]);*/
	printdata (data);
	return 0;
}

void func (GPtrArray* data) {

	int i;
	for (i = 0; i < 100; i++) {
		g_ptr_array_add (data, &i);
	}

}

void printdata (GPtrArray* data) {

	STOCK* e;
	int i = 0;
	for (i = 0; i < (int) data -> len; i++) {
		e = (STOCK*) g_ptr_array_index (data, i);
		printf ("%s, %.2f, %.2f, %.2f, %.0f, %.2f\n", e -> date, e -> open, e -> high, e -> low, e -> close, e -> volume, e -> adj_close);
	}
}

void printint (GPtrArray* data) {
	int i;
	int* k;

	for (i = 0; i < data -> len; i ++) {
		k = (int*) g_ptr_array_index (data, i);
		printf ("%d\n", *k);
	}
}

void parse_csv (char* filename, GPtrArray* data) {

	char buffer [200];
	char* token;
/*	STOCK recordset[70000]; */

	FILE* file = fopen (filename, "r");
	float f;
	int i=0;

/*	while (fgets (buffer, 200, file)) {

		token = strtok (buffer, ",");

		strcpy (recordset [i].date, token);
		recordset [i].open = atof (strtok (NULL, ","));
		recordset [i].high = atof (strtok (NULL, ","));
		recordset [i].low = atof (strtok (NULL, ","));
		recordset [i].close = atof (strtok (NULL, ","));
		recordset [i].volume = atof (strtok (NULL, ","));
		recordset [i].adj_close = atof (strtok (NULL, ","));

		g_ptr_array_add (data, &recordset [i]);
		i++;

	} */
	STOCK* recordset = (STOCK*) malloc (sizeof (STOCK) * 10000);
	while (fgets (buffer, 200, file)) {
	
		token = strtok (buffer, ",");

		strcpy (recordset[i].date, token);
		recordset[i].open = atof (strtok (NULL, ","));
		recordset[i].high = atof (strtok (NULL, ","));
		recordset[i].low = atof (strtok (NULL, ","));
		recordset[i].close = atof (strtok (NULL, ","));
		recordset[i].volume = atof (strtok (NULL, ","));
		recordset[i].adj_close = atof (strtok (NULL, ","));

		g_ptr_array_add (data, &recordset[i]);
		free (recordset);
		i++;
	}

	fclose (file);
}
