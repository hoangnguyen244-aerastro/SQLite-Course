#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer;    // Буфер таблицы
GtkTreeView         *treeview1; // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначения полей
enum
{
    C_ID, C_TOVAR, C_ARTIKUL, C_KOLVO
};


// Загрузка таблицы Товары из БД в буфер виджета
int LoadTovari()
{
    char* SQL = "SELECT [Код товара], Товар, Артикул, Количество \
                FROM Товары";

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
                            C_TOVAR, sqlite3_column_text(stmt, C_TOVAR),
                            C_ARTIKUL, sqlite3_column_text(stmt, C_ARTIKUL),
                            C_KOLVO, (guint) sqlite3_column_int(stmt, C_KOLVO),
                            -1);
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Кнопка Открыть
G_MODULE_EXPORT void on_button4_clicked (GtkButton *button, gpointer label)
{
    // Определяем текущий код товара
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;
        // Получение кода товара
        gtk_tree_model_get (model, &iter,
                            C_ID, &id_data,
                            -1);
        ShowTovar(id_data);
    }
}

// Добавить запись
int AddItem ()
{
    // Вставка новой записи в базу
    char* SQL="INSERT INTO Товары DEFAULT VALUES;";
    // Записываем в БД
    if (sqlite3_exec(db, SQL, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 1;
    }
    // Получение последней вставленной строки
    char* SQL2 = "SELECT [Код товара], Товар, Артикул, Количество \
                FROM Товары    \
                ORDER BY [Код товара] DESC LIMIT 1";

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
                        C_TOVAR, sqlite3_column_text(stmt, C_TOVAR),
                        C_ARTIKUL, sqlite3_column_text(stmt, C_ARTIKUL),
                        C_KOLVO, (guint) sqlite3_column_int(stmt, C_KOLVO),
                        -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Обновить запись
int Reload(int tovar)
{
    GtkTreeIter iter;
    //GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

    gtk_tree_selection_get_selected (selection, NULL, &iter);

    // Получение записи
    char* SQL = "SELECT [Код товара], Товар, Артикул, Количество \
                FROM Товары    \
                WHERE [Код товара]=";

    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, "%d", tovar);
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_set (buffer, &iter,
                            C_ID, sqlite3_column_int(stmt, C_ID),
                            C_TOVAR, sqlite3_column_text(stmt, C_TOVAR),
                            C_ARTIKUL, sqlite3_column_text(stmt, C_ARTIKUL),
                            C_KOLVO, (guint) sqlite3_column_int(stmt, C_KOLVO),
                            -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}


// Удаление записи
void DeleteItem ()
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview1);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview1);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;

        // Получение кода сотрудника
        gtk_tree_model_get (model, &iter,
                            C_ID, &id_data,
                            -1);

        // Удаление строки из буфера
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

        // Удаление записи из БД
        lenstr = 0;
        lenstr+=sprintf(str, "DELETE FROM Товары WHERE [Код товара]=");
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
    gtk_widget_destroy (wTovari);
}


// Вывод таблицы
void ShowTovari()
{

    builder2 = gtk_builder_new ();
    gtk_builder_add_from_file (builder2, "main.glade", NULL);
    wTovari = GTK_WIDGET(gtk_builder_get_object(builder2, "wTovari"));
    buffer =  GTK_LIST_STORE(gtk_builder_get_object(builder2, "liststore1"));
    treeview1 =  GTK_TREE_VIEW(gtk_builder_get_object(builder2, "treeview1"));

    gtk_window_set_default_size (GTK_WINDOW (wTovari), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wTovari), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (builder2, NULL);

    if(LoadTovari())
    {
        ShowError();
    }
    gtk_widget_show_all(wTovari);
}
