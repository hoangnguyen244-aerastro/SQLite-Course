/****************************************************************
Урок 3.5
Пример b9 - Изменение записей реализовано для таблицы "Сотрудники"

Задание:
Реализовать правку, добавление и удаление для таблицы "Отделы"
*****************************************************************/
#include "main.h"

/* Общие GTK/SQLite объекты: одно определение. */
sqlite3 *db = NULL;
sqlite3_stmt *stmt = NULL;
char *err = NULL;
int rc = 0;
GtkWidget *window1 = NULL, *window2 = NULL, *window3 = NULL;
GtkBuilder *builder1 = NULL, *builder2 = NULL, *builder3 = NULL;
char str[1000] = {0};
int lenstr = 0;

// Открываем БД
void StartDB()
{
    if( sqlite3_open("b9.db", &db))
    {
        sprintf(str, "Ошибка открытия БД: %s\n", sqlite3_errmsg(db));
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
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", str);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

int ShowSQL()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    "%s", str);

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

	    /* Сигналы подключены явно для Windows. */
        g_signal_connect(gtk_builder_get_object(builder1, "button1"),
                         "clicked", G_CALLBACK(on_button1_clicked), NULL);
        g_signal_connect(gtk_builder_get_object(builder1, "button2"),
                         "clicked", G_CALLBACK(on_button2_clicked), NULL);
        g_signal_connect_swapped(gtk_builder_get_object(builder1, "button3"),
                         "clicked", G_CALLBACK(gtk_widget_destroy), window1);
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
