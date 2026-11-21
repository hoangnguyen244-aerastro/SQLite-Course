#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

extern sqlite3 *db;    // õýíäë ÁÄ

void ShowTable(GtkWidget *win);
void ShowError ();
void StartDB();
int SaveDB();
int LoadDB();
void EndDB();

#endif // GRID_H_INCLUDED
