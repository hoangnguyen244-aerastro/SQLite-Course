#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer3;    // Буфер таблицы
GtkTreeView         *table3;     // Таблица
GtkTreeIter         iter3;

// Обозначения полей
enum
{
    C_ID_NAKL, C_NOMER, C_DATE, C_POSTAV, C_POKUP
};


// Загрузка таблицы из БД в буфер виджета
int LoadRash()
{

    char* SQL = "SELECT [Код накладной], Номер, Дата FROM [Расходные накладные]";
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
        gtk_list_store_append (buffer3, &iter3);
        gtk_list_store_set (buffer3, &iter3,
                            C_ID_NAKL, sqlite3_column_int(stmt, C_ID_NAKL),
                            C_NOMER, sqlite3_column_int(stmt, C_NOMER),
                            C_DATE,  sqlite3_column_text(stmt, C_DATE),
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

// Добавить запись
int AddItemRash ()
{
    char CurDate[80];

    // Получение текущей даты
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );  // текущая дата в секундах
    timeinfo = localtime ( &rawtime );
    strftime(CurDate, 80, "%Y-%m-%d", timeinfo);

    // Вставка новой записи в базу
    char* SQL="INSERT INTO Накладные(Тип, Дата) VALUES (-1, ";
    sprintf(str,"%s '%s')", SQL, CurDate);
    ///ShowSQL(); // Тестовый вывод запроса
    // Записываем в БД
    if (sqlite3_exec(db, str, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 1;
    }
    // Получение последней вставленной строки
    char* SQL2 = "SELECT [Код накладной], Номер, Дата \
                FROM Накладные    \
                ORDER BY [Код накладной] DESC LIMIT 1";

    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_append (buffer3, &iter3);
    gtk_list_store_set (buffer3, &iter3,
                            C_ID_NAKL, sqlite3_column_int(stmt, C_ID_NAKL),
                            C_NOMER, sqlite3_column_int(stmt, C_NOMER),
                            C_DATE,  sqlite3_column_text(stmt, C_DATE),
                        -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Удаление записи
void DeleteItemRash ()
{
    GtkTreeModel *model = gtk_tree_view_get_model (table3);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (table3);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter3))
    {
        gint id_data;

        // Получение кода сотрудника
        gtk_tree_model_get (model, &iter3,
                            C_ID_NAKL, &id_data,
                            -1);

        // Удаление строки из буфера
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter3);

        // Удаление записи из БД
        lenstr = 0;
        lenstr+=sprintf(str, "DELETE FROM Накладные WHERE [Код накладной]=");
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
G_MODULE_EXPORT void on_button17_clicked (GtkButton *button, gpointer label)
{
    AddItemRash ();
}

// Кнопка Удалить
G_MODULE_EXPORT void on_button18_clicked (GtkButton *button, gpointer label)
{
    DeleteItemRash ();
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button19_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wRash);
}

// Вывод таблицы
void ShowRash()
{
    builder_rash = gtk_builder_new ();
    gtk_builder_add_from_file (builder_rash, "main.glade", NULL);
    wRash = GTK_WIDGET(gtk_builder_get_object(builder_rash, "w_rash"));
    buffer3 =  GTK_LIST_STORE(gtk_builder_get_object(builder_rash, "liststore3"));
    table3 =  GTK_TREE_VIEW(gtk_builder_get_object(builder_rash, "treeview3"));

    gtk_window_set_default_size (GTK_WINDOW (wRash), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wRash), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (builder_rash, NULL);

    if(LoadRash())
    {
        ShowError();
    }
    gtk_widget_show_all(wRash);
}
