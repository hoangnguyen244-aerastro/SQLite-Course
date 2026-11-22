#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include "sotrudniki.h"
#include "sotrudnik.h"
#include "otdeli.h"
#include "otdel.h"

/* Shared application state: definitions live in main.c. */
extern sqlite3 *db;
extern sqlite3_stmt *stmt;
extern char *err;
extern int rc;
extern GtkWidget *window1, *window2, *window3;
extern GtkBuilder *builder1, *builder2, *builder3;
extern GtkWidget *w_sotrudnik;
extern GtkBuilder *b_sotrudnik;
extern GtkWidget *gSotr;
extern GtkSpinButton *gOklad;
extern char str[1000];
extern int lenstr;
/* Tema 4, task 2: selector for manager surname. */
extern GtkComboBoxText *gBossCombo;

void StartDB(void);
void EndDB(void);
void ShowError(void);
int ShowSQL(void);

#endif
