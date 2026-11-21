#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED
extern sqlite3 *db;
void ShowTable(GtkWidget *win);
void ShowError(void);
void StartDB(void);
int SaveDB(void);
void EndDB(void);
#endif
