#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer2;    // Буфер таблицы Отделы
GtkTreeView         *table2;     // Таблица Отделы


// Обозначения полей
enum
{
    C_OTDEL_ID, C_OTDEL, C_BOSS
};

// Сохранение таблицы Отделы
void SaveOtdeli()
{
    GtkTreeModel *model;
    GtkTreeIter  iter;       // Итератор таблицы (текущая строка)
    gboolean valid;
    gint row_count = 0;

    char* SQLstart = "DROP TABLE IF EXISTS Отделы; \
            CREATE TABLE Отделы([Код отдела] INTEGER PRIMARY KEY, Отдел, Руководитель);";

    // Очищаем таблицу Отделы
    if (sqlite3_exec(db, SQLstart, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }
    // Получаем первый итератор в списке
    model = gtk_tree_view_get_model (table2);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    // Цикл по строкам буфера
    while (valid)
    {
        gchar *otdel_data;
        gchar *boss_data;

        gtk_tree_model_get (model, &iter,
                            C_OTDEL, &otdel_data,
                            C_BOSS, &boss_data,
                            -1);

        // Готовим SQL-запрос
        lenstr = 0;
        lenstr+=sprintf(str, "INSERT INTO Отделы(Отдел,Руководитель) VALUES(");
        lenstr+=sprintf(str+lenstr, "'%s','%s'\n", otdel_data, boss_data);
        lenstr+=sprintf(str+lenstr, ");");
        //ShowSQL(); // Тестовый вывод запроса
        // Записываем в БД
        if (sqlite3_exec(db, str, 0, 0, &err))
        {
            sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
            sqlite3_free(err);
            return;
        }

        g_free (otdel_data);
        g_free (boss_data);
        row_count++;
        valid = gtk_tree_model_iter_next (model, &iter);
    }
}

G_MODULE_EXPORT void on_cellrenderertext4_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table2);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_OTDEL_ID, atoi(new_text), -1 );
    }
}


G_MODULE_EXPORT void on_cellrenderertext5_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table2);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_OTDEL, new_text, -1 );
    }
}

G_MODULE_EXPORT void on_cellrenderertext6_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table2);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_BOSS, new_text, -1 );
    }
}

// Загрузка таблицы Отделы из БД в буфер виджета
int LoadOtdeli()
{
    GtkTreeIter iter;
    char* SQL = "SELECT [Код отдела], Отдел, Руководитель FROM Отделы;";
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
        gtk_list_store_append (buffer2, &iter);
        gtk_list_store_set (buffer2, &iter,
                            C_OTDEL_ID, sqlite3_column_int(stmt, C_OTDEL_ID),
                            C_OTDEL, sqlite3_column_text(stmt, C_OTDEL),
                            C_BOSS,  sqlite3_column_text(stmt, C_BOSS),
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
G_MODULE_EXPORT void on_button11_clicked (GtkButton *button, gpointer label)
{
    // SaveOtdeli(); // Запись буфера в БД
    gtk_widget_destroy (window3);
}



// Вывод таблицы
void ShowOtdeli(GtkWidget *win)
{

    builder3 = gtk_builder_new ();
    gtk_builder_add_from_file (builder3, "main.glade", NULL);
    window3 = GTK_WIDGET(gtk_builder_get_object(builder3, "window3"));
    buffer2 =  GTK_LIST_STORE(gtk_builder_get_object(builder3, "liststore2"));
    table2 =  GTK_TREE_VIEW(gtk_builder_get_object(builder3, "treeview2"));

    gtk_window_set_default_size (GTK_WINDOW (window3), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window3), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (builder3, NULL);

    if(LoadOtdeli())
    {
        ShowError();
    }
    gtk_widget_show_all(window3);
}
