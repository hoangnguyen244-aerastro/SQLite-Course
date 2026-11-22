/****************************************************************
Урок 4.5
Пример b10 - форма ввода реализована для таблицы "Сотрудники"

Задание:
Реализовать форму ввода для таблицы "Отделы"
*****************************************************************/
#include "main.h"

/* Definitions are centralized to work with GCC -fno-common too. */
sqlite3 *db = NULL;
sqlite3_stmt *stmt = NULL;
char *err = NULL;
int rc = SQLITE_OK;
GtkWidget *window1 = NULL, *window2 = NULL, *window3 = NULL;
GtkBuilder *builder1 = NULL, *builder2 = NULL, *builder3 = NULL;
GtkWidget *w_sotrudnik = NULL;
GtkBuilder *b_sotrudnik = NULL;
GtkWidget *gSotr = NULL;
GtkSpinButton *gOklad = NULL;
char str[1000] = {0};
int lenstr = 0;

// Открываем БД
void StartDB()
{
    sqlite3_stmt *q = NULL;
    int has_photo = 0;
    int result;
    if (sqlite3_open("b13.db", &db) != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка открытия БД: %s", sqlite3_errmsg(db));
        ShowError();
        return;
    }
    result = sqlite3_prepare_v2(db, "PRAGMA table_info(Отделы)", -1, &q, NULL);
    if (result != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка проверки схемы: %s", sqlite3_errmsg(db));
        ShowError();
        return;
    }
    while ((result = sqlite3_step(q)) == SQLITE_ROW) {
        const unsigned char *column = sqlite3_column_text(q, 1);
        if (column && strcmp((const char *)column, "Фото") == 0)
            has_photo = 1;
    }
    sqlite3_finalize(q);
    if (result != SQLITE_DONE) {
        g_snprintf(str, sizeof(str), "Ошибка чтения схемы: %s", sqlite3_errmsg(db));
        ShowError();
        return;
    }
    if (!has_photo && sqlite3_exec(db, "ALTER TABLE Отделы ADD COLUMN Фото TEXT", NULL, NULL, NULL) != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка добавления поля Фото: %s", sqlite3_errmsg(db));
        ShowError();
    }
    /* Add a founding date in ISO 8601 format (YYYY-MM-DD). */
    q = NULL;
    int has_date = 0;
    result = sqlite3_prepare_v2(db, "PRAGMA table_info(Отделы)", -1, &q, NULL);
    if (result != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка проверки даты: %s", sqlite3_errmsg(db));
        ShowError();
        return;
    }
    while ((result = sqlite3_step(q)) == SQLITE_ROW) {
        const unsigned char *column = sqlite3_column_text(q, 1);
        if (column && strcmp((const char *)column, "Дата основания") == 0) has_date = 1;
    }
    sqlite3_finalize(q);
    if (result != SQLITE_DONE) {
        g_snprintf(str, sizeof(str), "Ошибка чтения даты: %s", sqlite3_errmsg(db));
        ShowError();
        return;
    }
    if (!has_date && sqlite3_exec(db, "ALTER TABLE Отделы ADD COLUMN [Дата основания] TEXT", NULL, NULL, NULL) != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка добавления даты: %s", sqlite3_errmsg(db));
        ShowError();
    }
}

// Закрываем БД
void EndDB()
{
    sqlite3_close(db);
}

void ShowError ()
{
    GtkWidget *dialog = NULL;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window1), GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, str);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

int ShowSQL()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    str);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return 0;
}


int ShowGladeError()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                 "Не найден файл\n main.glade");

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return 0;
}

G_MODULE_EXPORT void on_button1_clicked (GtkButton *button, gpointer label)
{
    ShowOtdeli(window1);
}

G_MODULE_EXPORT void on_button2_clicked (GtkButton *button, gpointer label)
{
    ShowSotrudniki(window1);
}


int ShowMainWindow()
{
	builder1 = gtk_builder_new ();
	if (gtk_builder_add_from_file (builder1, "main.glade", NULL))
	{
        window1 = GTK_WIDGET(gtk_builder_get_object(builder1, "window1"));

        gtk_window_set_default_size (GTK_WINDOW (window1), 500, 300);
        gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

	    gtk_builder_connect_signals (builder1, NULL);
        g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);

        gtk_widget_show (window1);

        return TRUE;
	}
    else
	{
	    return FALSE;
	}
}


int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    StartDB();
    if (ShowMainWindow ())
    {
       gtk_main ();
       EndDB();
       return 0;
    }
    else
    {
       ShowGladeError();
       EndDB();
       return 1;
    }
}
