#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

sqlite3 *db;    // хэндл БД

void ShowTable();
void ShowError ();
void StartDB();
int SaveDB();
int LoadDB();
void EndDB();

#endif // GRID_H_INCLUDED
