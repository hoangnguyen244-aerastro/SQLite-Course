/*******************************
Урок 1.3
Задание 2 - Работа с таблицами в GTK+
Добавлены поле «Имя» и четвертый сотрудник.
********************************/
#include <gtk/gtk.h>

GtkWidget *window;
GtkListStore *buffer;
GtkWidget *table;
GtkTreeViewColumn *column;
GtkTreeIter iter;
GtkCellRenderer *renderer;

enum
{
    C_NUM, C_NAME, C_FIRST_NAME, C_OKLAD, N_COL
};

void ShowTable(void)
{
    // Четыре столбца: номер, фамилия, имя, оклад.
    buffer = gtk_list_store_new(N_COL,
                                G_TYPE_UINT, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_UINT);

    gtk_list_store_append(buffer, &iter);
    gtk_list_store_set(buffer, &iter,
                       C_NUM, (guint) 1,
                       C_NAME, "Иванов",
                       C_FIRST_NAME, "Иван",
                       C_OKLAD, (guint) 20000, -1);

    gtk_list_store_append(buffer, &iter);
    gtk_list_store_set(buffer, &iter,
                       C_NUM, (guint) 2,
                       C_NAME, "Петров",
                       C_FIRST_NAME, "Петр",
                       C_OKLAD, (guint) 25000, -1);

    gtk_list_store_append(buffer, &iter);
    gtk_list_store_set(buffer, &iter,
                       C_NUM, (guint) 3,
                       C_NAME, "Сидоров",
                       C_FIRST_NAME, "Алексей",
                       C_OKLAD, (guint) 30000, -1);

    // Новая четвертая строка.
    gtk_list_store_append(buffer, &iter);
    gtk_list_store_set(buffer, &iter,
                       C_NUM, (guint) 4,
                       C_NAME, "Смирнов",
                       C_FIRST_NAME, "Дмитрий",
                       C_OKLAD, (guint) 28000, -1);

    table = gtk_tree_view_new_with_model(GTK_TREE_MODEL(buffer));
    renderer = gtk_cell_renderer_text_new();

    column = gtk_tree_view_column_new_with_attributes
                ("№", renderer, "text", C_NUM, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(table), column);

    column = gtk_tree_view_column_new_with_attributes
                ("Фамилия", renderer, "text", C_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(table), column);

    // Новый отображаемый столбец.
    column = gtk_tree_view_column_new_with_attributes
                ("Имя", renderer, "text", C_FIRST_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(table), column);

    column = gtk_tree_view_column_new_with_attributes
                ("Оклад", renderer, "text", C_OKLAD, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(table), column);

    g_object_unref(buffer);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Задание 2 - Таблица сотрудников");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    ShowTable();
    gtk_container_add(GTK_CONTAINER(window), table);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
