#include "main.h"

// Кнопка Закрыть
G_MODULE_EXPORT void on_button7_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (window2);
}

// Вывод таблицы
void ShowTable(GtkWidget *win)
{
    builder2 = gtk_builder_new ();
    gtk_builder_add_from_file (builder2, "main.glade", NULL);
    window2 = GTK_WIDGET(gtk_builder_get_object(builder2, "window2"));

    gtk_window_set_default_size (GTK_WINDOW (window2), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (builder2, NULL);
    gtk_widget_show_all(window2);
}
