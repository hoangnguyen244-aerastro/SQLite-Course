#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <math.h>
#include "tovar.h"
#include "tovari.h"

// Структуры данных для базы данных
sqlite3 *db;    // хэндл БД
sqlite3_stmt* stmt; // строка запроса к БД
char *err;
int rc;

// Структуры данных для окон
GtkWidget   *wMain, *wTovari, *window3;
GtkBuilder  *builder1, *builder2, *builder3;

GtkWidget   *wTovar;
GtkBuilder  *bTovar;


char str[1000];
int lenstr;

void StartDB();
void EndDB();
void ShowError();
int ShowSQL();

#endif // MAIN_H_INCLUDED
