/********************************
Урок 1.4
Пример b3 - Вывод таблицы в Glade
*********************************/
#include "main.h"

void ShowVersion ()
{
  GtkWidget *dialog = NULL;

  dialog = gtk_message_dialog_new (GTK_WINDOW (window1), GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, sqlite3_libversion());
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
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
    ShowVersion();
}

G_MODULE_EXPORT void on_button2_clicked (GtkButton *button, gpointer label)
{
    ShowTable(window1);
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
    if (ShowMainWindow ())
    {
       gtk_main ();
       return 0;
    }
    else
    {
       ShowGladeError();
       return 1;
    }
}
