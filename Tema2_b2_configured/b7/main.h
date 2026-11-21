#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include "sotrudniki.h"
#include "otdeli.h"
extern sqlite3 *db;
extern sqlite3_stmt *stmt;
extern char *err;
extern int rc;
extern GtkWidget *window1, *window2, *window3;
extern GtkBuilder *builder1, *builder2, *builder3;
extern char str[1000];
extern int lenstr;
void StartDB(void);
void EndDB(void);
void ShowError(void);
int ShowSQL(void);
#endif
