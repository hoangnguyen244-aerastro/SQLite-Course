#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <math.h>
#include "tovar.h"
#include "tovari.h"
#include "prih.h"
#include "prih_stroki.h"
#include "prih_stroka.h"
#include "rash.h"
#include "settings.h"


// Структуры данных для базы данных
sqlite3 *db;    // хэндл БД
sqlite3_stmt* stmt; // строка запроса к БД
char *err;
int rc;

// Структуры данных для окон
GtkWidget   *wMain, *wTovari, *wPrih, *wRash, *wPrihStroki;
GtkWidget   *wPrihStroka, *wSettings;
GtkBuilder  *builder1, *builder2, *builder_prih, *builder_rash;
GtkBuilder  *bPrihStroki, *bPrihStroka, *bSettings;

GtkWidget   *wTovar;
GtkBuilder  *bTovar;

char str[1000];
int lenstr;

void StartDB();
void EndDB();
void ShowError();
int ShowSQL();

#endif // MAIN_H_INCLUDED
