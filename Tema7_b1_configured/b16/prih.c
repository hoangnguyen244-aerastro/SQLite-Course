#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer2;    // Буфер таблицы
GtkTreeView         *treeview_prih;     // Таблица
GtkTreeIter         iter2;

// Обозначения полей
enum
{
    C_ID_NAKL, C_NOMER, C_DATE, C_POSTAV, C_POKUP
};


// Загрузка таблицы из БД в буфер виджета
int LoadPrih()
{

    char* SQL = "SELECT [Код накладной], Номер, strftime('%d.%m.%Y',Дата) FROM [Приходные накладные]";
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
        gtk_list_store_append (buffer2, &iter2);
        gtk_list_store_set (buffer2, &iter2,
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

// Обновить запись
int ReloadPrih(int nakl)
{
    gtk_list_store_clear(buffer2);
    char* SQL = "SELECT [Код накладной], Номер, strftime('%d.%m.%Y',Дата) FROM [Приходные накладные]";
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
        gtk_list_store_append (buffer2, &iter2);
        gtk_list_store_set (buffer2, &iter2,
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
int AddPrih ()
{
    char CurDate[80];

    // Получение текущей даты
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );  // текущая дата в секундах
    timeinfo = localtime ( &rawtime );
    strftime(CurDate, 80, "%Y-%m-%d", timeinfo);

    // Вставка новой записи в базу
    char* SQL="INSERT INTO Накладные(Тип, Дата) VALUES (1, ";
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
    char* SQL2 = "SELECT [Код накладной], Номер, strftime('%d.%m.%Y',Дата) \
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

    gtk_list_store_append (buffer2, &iter2);
    gtk_list_store_set (buffer2, &iter2,
                            C_ID_NAKL, sqlite3_column_int(stmt, C_ID_NAKL),
                            C_NOMER, sqlite3_column_int(stmt, C_NOMER),
                            C_DATE,  sqlite3_column_text(stmt, C_DATE),
                        -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Удаление записи
void DeletePrih ()
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview_prih);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview_prih);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter2))
    {
        gint id_data;

        // Получение кода сотрудника
        gtk_tree_model_get (model, &iter2,
                            C_ID_NAKL, &id_data,
                            -1);

        // Удаление строки из буфера
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter2);

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
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);
    }
}

// Кнопка Открыть
G_MODULE_EXPORT void on_button8_clicked (GtkButton *button, gpointer label)
{
    // Определяем текущий код
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview_prih);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview_prih);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;
        // Получение кода накладной
        gtk_tree_model_get (model, &iter,
                            C_ID_NAKL, &id_data,
                            -1);
        ShowPrihStroki(id_data);
    }
}


// Кнопка Добавить
G_MODULE_EXPORT void on_button9_clicked (GtkButton *button, gpointer label)
{
    AddPrih ();
}

// Кнопка Удалить
G_MODULE_EXPORT void on_button10_clicked (GtkButton *button, gpointer label)
{
    DeletePrih ();
}


// Кнопка Закрыть
G_MODULE_EXPORT void on_button11_clicked (GtkButton *button, gpointer label)
{
    gtk_widget_destroy (wPrih);
}

// Вывод таблицы
void ShowPrih(GtkWidget *win)
{

    builder_prih = gtk_builder_new ();
    gtk_builder_add_from_file (builder_prih, "main.glade", NULL);
    wPrih = GTK_WIDGET(gtk_builder_get_object(builder_prih, "w_prih"));
    buffer2 =  GTK_LIST_STORE(gtk_builder_get_object(builder_prih, "liststore2"));
    treeview_prih =  GTK_TREE_VIEW(gtk_builder_get_object(builder_prih, "treeview2"));

    gtk_window_set_default_size (GTK_WINDOW (wPrih), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wPrih), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (builder_prih, NULL);

    if(LoadPrih())
    {
        ShowError();
    }
    gtk_widget_show_all(wPrih);
}
