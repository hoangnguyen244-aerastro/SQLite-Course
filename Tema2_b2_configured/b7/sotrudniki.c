#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer;    // Буфер таблицы
GtkTreeView         *table;     // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначения полей
enum
{
    C_ID, C_NAME, C_OKLAD
};

// Сохранение таблицы Сотрудники
void SaveSotrudniki()
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    gint row_count = 0;

    char* SQLstart = "DROP TABLE IF EXISTS Сотрудники; \
            CREATE TABLE Сотрудники([Код сотрудника] INTEGER PRIMARY KEY,Фамилия,Оклад);";

    // Очищаем таблицу Сотрудники
    if (sqlite3_exec(db, SQLstart, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }

    // Получаем первый итератор в списке
    model = gtk_tree_view_get_model (table);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    // Цикл по строкам буфера
    while (valid)
    {
        gchar *name_data;
        gint   oklad_data;

        gtk_tree_model_get (model, &iter,
                            C_NAME, &name_data,
                            C_OKLAD, &oklad_data,
                            -1);

        // Готовим SQL-запрос
        lenstr = 0;
        lenstr+=sprintf(str, "INSERT INTO Сотрудники(Фамилия, Оклад) VALUES(");
        lenstr+=sprintf(str+lenstr, "'%s',%d\n", name_data, oklad_data);
        lenstr+=sprintf(str+lenstr, ");");
        //ShowSQL(); // Тестовый вывод запроса
        // Записываем в БД
        if (sqlite3_exec(db, str, 0, 0, &err))
        {
            sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
            sqlite3_free(err);
            return;
        }

        g_free (name_data);
        row_count ++;
        valid = gtk_tree_model_iter_next (model, &iter);
    }
}

G_MODULE_EXPORT void on_cellrenderertext1_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_ID, atoi(new_text), -1 );
    }
}


G_MODULE_EXPORT void on_cellrenderertext2_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_NAME, new_text, -1 );
    }
}

G_MODULE_EXPORT void on_cellrenderertext3_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_OKLAD, atoi(new_text), -1 );
    }
}

// Загрузка таблицы из БД в буфер виджета
int LoadDB()
{
    char* SQL = "SELECT [Код сотрудника], Фамилия, Оклад FROM Сотрудники;";
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        gtk_list_store_append (buffer, &iter);
        gtk_list_store_set (buffer, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OKLAD, (guint) sqlite3_column_int(stmt, C_OKLAD),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // 5. Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button7_clicked (GtkButton *button, gpointer label)
{
    SaveSotrudniki(); // Запись буфера в БД
    gtk_widget_destroy (window2);
}

// Вывод таблицы
void ShowSotrudniki(GtkWidget *win)
{

    builder2 = gtk_builder_new ();
    gtk_builder_add_from_file (builder2, "main.glade", NULL);
    window2 = GTK_WIDGET(gtk_builder_get_object(builder2, "window2"));
    buffer =  GTK_LIST_STORE(gtk_builder_get_object(builder2, "liststore1"));
    table =  GTK_TREE_VIEW(gtk_builder_get_object(builder2, "treeview1"));

    gtk_window_set_default_size (GTK_WINDOW (window2), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);

    GtkWidget *close_btn = GTK_WIDGET(gtk_builder_get_object(builder2, "button7"));
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_button7_clicked), NULL);

    if(LoadDB())
    {
        ShowError();
    }
    gtk_widget_show_all(window2);
}
