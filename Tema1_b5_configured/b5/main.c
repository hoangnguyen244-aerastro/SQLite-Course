/********************************************
 Урок 1.5
 Тема 1. Задание 5 — Минимальная программа на SQLite
 Таблица «Сотрудники» из пяти полей
********************************************/
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "sqlite3.h"

static const char *SQL1 =
    "DROP TABLE IF EXISTS Сотрудники;"
    "CREATE TABLE Сотрудники ("
    "N INTEGER, Фамилия TEXT, Имя TEXT, Отдел TEXT, Оклад INTEGER);"
    "INSERT INTO Сотрудники VALUES (1, 'Иванов', 'Иван', 'ИТ', 20000);"
    "INSERT INTO Сотрудники VALUES (2, 'Петров', 'Петр', 'Бухгалтерия', 25000);"
    "INSERT INTO Сотрудники VALUES (3, 'Сидоров', 'Алексей', 'Продажи', 30000);";

static const char *SQL2 =
    "SELECT N, Фамилия, Имя, Отдел, Оклад FROM Сотрудники;";

/* Формируем текст для информационного диалога. */
static int GetDB(char *result, gsize capacity)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char *errmsg = NULL;
    int rc;
    gsize used;

    result[0] = '\0';
    rc = sqlite3_open("b4.db", &db);
    if (rc != SQLITE_OK)
    {
        g_snprintf(result, capacity, "Ошибка открытия БД: %s",
                   db ? sqlite3_errmsg(db) : "unknown");
        if (db) sqlite3_close(db);
        return rc;
    }

    /* Создание таблицы и заполнение всех пяти полей. */
    rc = sqlite3_exec(db, SQL1, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        g_snprintf(result, capacity, "Ошибка SQL: %s",
                   errmsg ? errmsg : sqlite3_errmsg(db));
        sqlite3_free(errmsg);
        sqlite3_close(db);
        return rc;
    }

    rc = sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        g_snprintf(result, capacity, "Ошибка SELECT: %s",
                   sqlite3_errmsg(db));
        sqlite3_close(db);
        return rc;
    }

    g_strlcpy(result, "Сотрудники (5 полей):\n", capacity);
    used = strlen(result);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const unsigned char *surname = sqlite3_column_text(stmt, 1);
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *department = sqlite3_column_text(stmt, 3);

        /* Индексы полей SQLite начинаются с нуля: 0, 1, 2, 3, 4. */
        if (used < capacity)
        {
            used += g_snprintf(result + used, capacity - used,
                       "N=%d, Фамилия=%s, Имя=%s, Отдел=%s, Оклад=%d\n",
                       sqlite3_column_int(stmt, 0),
                       surname ? (const char *)surname : "",
                       name ? (const char *)name : "",
                       department ? (const char *)department : "",
                       sqlite3_column_int(stmt, 4));
            if (used >= capacity) used = capacity;
        }
    }
    if (rc != SQLITE_DONE)
    {
        g_snprintf(result, capacity, "Ошибка чтения: %s",
                   sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

static void ShowDB(GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog;
    char result[2048];

    (void)button;
    GetDB(result, sizeof(result));
    dialog = gtk_message_dialog_new(GTK_WINDOW(user_data),
                                    GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_CLOSE,
                                    "%s", result);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[])
{
    GtkWidget *win;
    GtkWidget *vbox;
    GtkWidget *button;

    gtk_init(&argc, &argv);
    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(win), 8);
    gtk_window_set_default_size(GTK_WINDOW(win), 560, 240);
    gtk_window_set_title(GTK_WINDOW(win), "Задание 5 — SQLite");
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    button = gtk_button_new_with_label("Information");
    g_signal_connect(button, "clicked", G_CALLBACK(ShowDB), win);
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    button = gtk_button_new_with_label("Close");
    g_signal_connect_swapped(button, "clicked",
                             G_CALLBACK(gtk_widget_destroy), win);
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
