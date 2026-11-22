#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer;    // Буфер таблицы
GtkTreeView         *treeview1;     // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначения полей
enum
{
    C_ID, C_NAME, C_OKLAD, C_KOD_OTDELA
};


// Сохранение текущей строки в таблицу Сотрудники
void SaveStrSotrudniki()
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);
    gtk_tree_selection_get_selected (selection, NULL, &iter);

    gint   id_data;
    gchar *name_data;
    gint   oklad_data;

    gtk_tree_model_get (model, &iter,
                        C_ID, &id_data,
                        C_NAME, &name_data,
                        C_OKLAD, &oklad_data,
                        -1);

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Сотрудники SET Фамилия=");
    lenstr+=sprintf(str+lenstr, "'%s',Оклад=%d WHERE [Код сотрудника]=%d", name_data, oklad_data, id_data);
    lenstr+=sprintf(str+lenstr, ";");
    ///ShowSQL(); // Тестовый вывод запроса
    // Записываем в БД
    if (sqlite3_exec(db, str, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }

    g_free (name_data);
}


G_MODULE_EXPORT void on_cellrenderertext1_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (treeview1);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_ID, atoi(new_text), -1 );
    }
}


G_MODULE_EXPORT void on_cellrenderertext2_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (treeview1);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_NAME, new_text, -1 );
        SaveStrSotrudniki();
    }
}

G_MODULE_EXPORT void on_cellrenderertext3_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (treeview1);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_OKLAD, atoi(new_text), -1 );
        SaveStrSotrudniki();
    }
}

// Загрузка таблицы из БД в буфер виджета
int LoadDB()
{
    char* SQL = "SELECT [Код сотрудника], с.Фамилия, с.Оклад, Отделы.Отдел \
                FROM Сотрудники AS с JOIN Отделы    \
                ON с.[Код отдела] = Отделы.[Код отдела]";

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
                            C_KOD_OTDELA, sqlite3_column_text(stmt, C_KOD_OTDELA),
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

// Добавить запись
int AddItem ()
{
    // Вставка новой записи в базу
    char* SQL="INSERT INTO Сотрудники DEFAULT VALUES;";
    // Записываем в БД
    if (sqlite3_exec(db, SQL, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }
    // Получение последней вставленной строки
    char* SQL2 = "SELECT [Код сотрудника], с.Фамилия, с.Оклад, Отделы.Отдел \
                FROM Сотрудники AS с JOIN Отделы    \
                ON с.[Код отдела] = Отделы.[Код отдела] \
                ORDER BY [Код сотрудника] DESC LIMIT 1";

    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_append (buffer, &iter);
    gtk_list_store_set (buffer, &iter,
                        C_ID, sqlite3_column_int(stmt, C_ID),
                        C_NAME, sqlite3_column_text(stmt, C_NAME),
                        C_OKLAD, (guint) sqlite3_column_int(stmt, C_OKLAD),
                        C_KOD_OTDELA, sqlite3_column_text(stmt, C_KOD_OTDELA),
                        -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
}

// Удаление записи
void DeleteItem ()
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint i, id_data;
        GtkTreePath *path;

        path = gtk_tree_model_get_path (model, &iter);

        // Получение кода сотрудника
        gtk_tree_model_get (model, &iter,
                            C_ID, &id_data,
                            -1);

        // Удаление строки из буфера
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
        gtk_tree_path_free (path);

        // Удаление записи из БД
        lenstr = 0;
        lenstr+=sprintf(str, "DELETE FROM Сотрудники WHERE [Код сотрудника]=");
        lenstr+=sprintf(str+lenstr, "%d", id_data);
        lenstr+=sprintf(str+lenstr, ";");
        ///ShowSQL(); // Тестовый вывод запроса
        // Записываем в БД
        if (sqlite3_exec(db, str, 0, 0, &err))
        {
            sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
            sqlite3_free(err);
            return;
        }

    }
}

// Кнопка Добавить
G_MODULE_EXPORT void on_button5_clicked (GtkButton *button, gpointer label)
{
    AddItem ();
}

// Кнопка Удалить
G_MODULE_EXPORT void on_button6_clicked (GtkButton *button, gpointer label)
{
    DeleteItem ();
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button7_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (window2);
}


// Вывод таблицы
void ShowSotrudniki(GtkWidget *win)
{

    builder2 = gtk_builder_new ();
    gtk_builder_add_from_file (builder2, "main.glade", NULL);
    window2 = GTK_WIDGET(gtk_builder_get_object(builder2, "window2"));
    buffer =  GTK_LIST_STORE(gtk_builder_get_object(builder2, "liststore1"));
    treeview1 =  GTK_TREE_VIEW(gtk_builder_get_object(builder2, "treeview1"));

    gtk_window_set_default_size (GTK_WINDOW (window2), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);

    g_signal_connect(gtk_builder_get_object(builder2, "cellrenderertext2"), "edited", G_CALLBACK(on_cellrenderertext2_edited), NULL);
    g_signal_connect(gtk_builder_get_object(builder2, "cellrenderertext3"), "edited", G_CALLBACK(on_cellrenderertext3_edited), NULL);
    g_signal_connect(gtk_builder_get_object(builder2, "button5"), "clicked", G_CALLBACK(on_button5_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(builder2, "button6"), "clicked", G_CALLBACK(on_button6_clicked), NULL);
    g_signal_connect(gtk_builder_get_object(builder2, "button7"), "clicked", G_CALLBACK(on_button7_clicked), NULL);

    if(LoadDB())
    {
        ShowError();
    }
    gtk_widget_show_all(window2);
}
