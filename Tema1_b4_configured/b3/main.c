/*******************************************
 Тема 1. Задание 4 — Обработка таблиц в GTK+
 На основе проекта b3 и формы main1.glade
********************************************/
#include "main.h"

/* Кнопка "Закрыть" из main1.glade. */
G_MODULE_EXPORT void on_button1_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;
    gtk_widget_destroy(window1);
}

/* Показываем сообщение, если файл Glade отсутствует. */
static void ShowGladeError(void)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE, "%s",
        "Не удалось открыть файл main1.glade");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static gboolean ShowMainWindow(void)
{
    GError *error = NULL;
    GtkWidget *close_button;

    builder1 = gtk_builder_new();
    /* Задание 4: вместо main.glade подключаем main1.glade. */
    if (!gtk_builder_add_from_file(builder1, "main1.glade", &error))
    {
        if (error != NULL)
        {
            g_warning("%s", error->message);
            g_error_free(error);
        }
        g_object_unref(builder1);
        return FALSE;
    }

    window1 = GTK_WIDGET(gtk_builder_get_object(builder1, "window1"));
    close_button = GTK_WIDGET(gtk_builder_get_object(builder1, "button1"));

    if (window1 == NULL || close_button == NULL)
    {
        g_warning("В main1.glade отсутствует window1 или button1");
        g_object_unref(builder1);
        return FALSE;
    }

    gtk_window_set_default_size(GTK_WINDOW(window1), 600, 320);
    gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

    /* Явно связываем сигнал, не полагаясь на экспорт символов Windows DLL. */
    g_signal_connect(close_button, "clicked",
                     G_CALLBACK(on_button1_clicked), NULL);
    g_signal_connect(window1, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window1);
    return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    if (!ShowMainWindow())
    {
        ShowGladeError();
        return 1;
    }
    gtk_main();
    return 0;
}
