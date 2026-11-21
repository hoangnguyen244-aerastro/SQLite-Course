/**************************************************
 Тема 2, Задание 1 — Правка таблицы и запись в БД
 На основе примера b6
**************************************************/
#include "main.h"

GtkWidget *window1 = NULL, *window2 = NULL;
GtkBuilder *builder1 = NULL, *builder2 = NULL;

static int ShowGladeError(void)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE, "%s", "Не найден файл main.glade");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return 0;
}

static void on_button1_clicked(GtkButton *button, gpointer userdata)
{
    (void)button; (void)userdata;
    if (SaveDB()) ShowError();
}

static void on_button2_clicked(GtkButton *button, gpointer userdata)
{
    (void)button; (void)userdata;
    ShowTable(window1);
}

static gboolean ShowMainWindow(void)
{
    GtkWidget *save, *staff, *exit_button;
    GError *error = NULL;

    builder1 = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder1, "main.glade", &error))
    {
        if (error) {
            g_warning("Glade: %s", error->message);
            g_error_free(error);
        }
        g_object_unref(builder1);
        return FALSE;
    }
    window1 = GTK_WIDGET(gtk_builder_get_object(builder1, "window1"));
    save = GTK_WIDGET(gtk_builder_get_object(builder1, "button1"));
    staff = GTK_WIDGET(gtk_builder_get_object(builder1, "button2"));
    exit_button = GTK_WIDGET(gtk_builder_get_object(builder1, "button3"));
    if (!window1 || !save || !staff || !exit_button)
        return FALSE;

    gtk_window_set_default_size(GTK_WINDOW(window1), 500, 300);
    gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);
    /* Сигналы соединяются явно для надежной работы в Windows/MinGW. */
    g_signal_connect(save, "clicked", G_CALLBACK(on_button1_clicked), NULL);
    g_signal_connect(staff, "clicked", G_CALLBACK(on_button2_clicked), NULL);
    g_signal_connect_swapped(exit_button, "clicked",
                             G_CALLBACK(gtk_widget_destroy), window1);
    g_signal_connect(window1, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window1);
    return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    StartDB();
    if (!ShowMainWindow()) {
        ShowGladeError();
        EndDB();
        return 1;
    }
    gtk_main();
    EndDB();
    return 0;
}
