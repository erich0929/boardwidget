#include <stdio.h>
#include <glib.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

typedef struct _STOCK {
	char date[50];
	float open;
	float high;
	float low;
	float close;
	float volume;
	float adj_close;
}STOCK;

typedef struct _BufferStruct {
	char* buffer;
	size_t size;
}BufferStruct;

static size_t WriteMemoryCallback
(void *ptr, size_t size, size_t nmemb, void *data);

int str_to_array (char* line, GPtrArray* data);

long get_stock_from_yahoo (char* code, 
		char* from_mm, char* from_dd, char* from_yy, 
		char* to_mm, char* to_dd, char* to_yy, GPtrArray* data);
