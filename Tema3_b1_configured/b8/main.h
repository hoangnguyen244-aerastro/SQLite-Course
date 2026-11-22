#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include "sotrudniki.h"
#include "otdeli.h"

// Ñòðóêòóðû äàííûõ äëÿ áàçû äàííûõ
extern sqlite3 *db;    // õýíäë ÁÄ
extern sqlite3_stmt* stmt; // ñòðîêà çàïðîñà ê ÁÄ
extern char *err;
extern int rc;

// Ñòðóêòóðû äàííûõ äëÿ îêîí
extern GtkWidget   *window1, *window2, *window3;
extern GtkBuilder  *builder1, *builder2, *builder3;

extern char str[1000];
extern int lenstr;

void StartDB();
void EndDB();
void ShowError();
int ShowSQL();

#endif // MAIN_H_INCLUDED
