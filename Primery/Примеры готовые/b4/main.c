/*******************************************
Урок 1.5
Пример b4 - Минимальная программа на SQLite
********************************************/
#include <stdlib.h>
#include <gtk/gtk.h>
#include "sqlite3.h"

sqlite3 *db = 0;    // хэндл БД
sqlite3_stmt* stmt; // строка запроса к БД
char *err = 0;

char* SQL1 = "DROP TABLE IF EXISTS Сотрудники; \
            CREATE TABLE Сотрудники(N,Фамилия,Оклад); \
            INSERT INTO Сотрудники VALUES(1,'Иванов',20000);  \
            INSERT INTO Сотрудники VALUES(2,'Петров',25000);  \
            INSERT INTO Сотрудники VALUES(3,'Сидоров',30000);  \
            ";
char* SQL2 = "SELECT N, Фамилия, Оклад FROM Сотрудники;";
char str[1000];
int lenstr=0;


int GetDB()
{
    // 1. Открываем БД
    if( sqlite3_open("b4.db", &db))
    {
        sprintf(str, "Ошибка открытия БД: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // 2. Создаем таблицу
    if (sqlite3_exec(db, SQL1, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 2;
    }

    // 3. Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 3;
    }

    // 4. Цикл по SQL-запросу и формируем строку
    int rc = 0;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        lenstr+=sprintf(str+lenstr,"N = %2d,", sqlite3_column_int(stmt, 0));
        lenstr+=sprintf(str+lenstr," Фамилия: %s,", sqlite3_column_text(stmt, 1));
        lenstr+=sprintf(str+lenstr," Оклад = %5d\n", sqlite3_column_int(stmt, 2));
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 4;
    }

    // 5. Освобождаем строку запроса
    sqlite3_finalize(stmt);

    // 6. Закрываем БД
    sqlite3_close(db);
    return 0;
}


void ShowDB (GtkWidget *wid, GtkWidget *win)
{
    GtkWidget *dialog = NULL;

    GetDB();

    dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, str);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

int main (int argc, char *argv[])
{
    GtkWidget *button = NULL;
    GtkWidget *win = NULL;
    GtkWidget *vbox = NULL;

    /* Initialize GTK+ */
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    gtk_init (&argc, &argv);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

    /* Create the main window */
    win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (win), 8);
    gtk_window_set_default_size (GTK_WINDOW (win), 400, 200);
    gtk_window_set_title (GTK_WINDOW (win), "Hello World");
    gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
    gtk_widget_realize (win);
    g_signal_connect (win, "destroy", gtk_main_quit, NULL);

    /* Create a vertical box with buttons */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add (GTK_CONTAINER (win), vbox);

    button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (ShowDB), (gpointer) win);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    g_signal_connect (button, "clicked", gtk_main_quit, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

    /* Enter the main loop */
    gtk_widget_show_all (win);
    gtk_main ();
    return 0;
}
