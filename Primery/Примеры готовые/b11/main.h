#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include "sotrudniki.h"
#include "sotrudnik.h"
#include "otdeli.h"

// Структуры данных для базы данных
sqlite3 *db;    // хэндл БД
sqlite3_stmt* stmt; // строка запроса к БД
char *err;
int rc;

// Структуры данных для окон
GtkWidget   *window1, *window2, *window3;
GtkBuilder  *builder1, *builder2, *builder3;

GtkWidget   *w_sotrudnik;
GtkBuilder  *b_sotrudnik;

GtkWidget    *gSotr;
GtkSpinButton *gOklad;
GtkComboBoxText *gOtdel;


char str[1000];
int lenstr;

void StartDB();
void EndDB();
void ShowError();
int ShowSQL();

#endif // MAIN_H_INCLUDED
