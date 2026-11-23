#include "main.h"

// Структуры данных для виджета
GtkListStore        *liststore_prih_stroki;     // Буфер таблицы
GtkTreeView         *treeview_prih_stroki;      // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

GtkSpinButton   *gNomer;
GtkSpinButton   *gDay;
GtkSpinButton   *gMonth;
GtkSpinButton   *gYear;

int CurNakl ; // Код текущей накладной

// Обозначения полей таблицы "Накладные"
enum
{
    C_IDNAKL, C_NOMER, C_DATE
};


// Обозначения полей таблицы "Строки накладной"
enum
{
    C_ID_STR_NAKL, C_STR_NAKL, C_ID_TOVAR, C_ID_NAKL, C_PRIHOD, C_EDIZM
};

char* subString (const char* input, int offset, int len, char* dest)
{
    int input_len = strlen (input);
    if (offset + len > input_len)
    {
        return NULL;
    }
}

// Загрузка полей накладной
int LoadNakl(int nakl)
{

 /*   char* SQL = "SELECT [Код накладной], Номер,  \
                strftime('%%d', Дата), \
                strftime('%%m', Дата), \
                strftime('%%Y', Дата) \
                FROM Накладные  \
                WHERE [Код накладной] = ";*/
   char* SQL = "SELECT [Код накладной], Номер, Дата  \
                FROM Накладные WHERE [Код накладной] = ";

    sprintf(str, "%s%d",SQL, nakl);
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK )
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        ShowSQL();
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса
    rc = sqlite3_step(stmt);
    // Запись в виджеты
    gtk_spin_button_set_value(gNomer,sqlite3_column_int(stmt, C_NOMER));

    char buf[100];
    char buf2[100];
    sprintf(buf,"%s", sqlite3_column_text(stmt, C_DATE));

    // Получаем день
    memcpy( buf2, &buf[8], 2 );
    buf2[2] = '\0';
    gtk_spin_button_set_value(gDay, atoi(buf2));
    // Получаем месяц
    memcpy( buf2, &buf[5], 2 );
    buf2[2] = '\0';
    gtk_spin_button_set_value(gMonth,atoi(buf2));
    // Получаем год
    strncpy(buf2, buf, 4);
    buf2[4] = '\0';
    gtk_spin_button_set_value(gYear,atoi(buf2));

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Загрузка таблицы из БД в буфер виджета
int LoadPrihStroki()
{

    char* SQL = "SELECT [Код строки накладной], t.[Товар], s.[Код товара], \
                [Код накладной], Приход, t.[Единица измерения] \
                FROM [Строки накладной] AS s JOIN Товары AS t \
                ON s.[Код товара] = t.[Код товара]";

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
        gtk_list_store_append (liststore_prih_stroki, &iter);
        gtk_list_store_set (liststore_prih_stroki, &iter,
                            C_ID_STR_NAKL, sqlite3_column_int(stmt, C_ID_STR_NAKL),
                            C_STR_NAKL, sqlite3_column_text(stmt, C_STR_NAKL),
                            C_ID_TOVAR, sqlite3_column_int(stmt, C_ID_TOVAR),
                            C_ID_NAKL, sqlite3_column_int(stmt, C_ID_NAKL),
                            C_PRIHOD,  sqlite3_column_int(stmt, C_PRIHOD),
                            C_EDIZM,  sqlite3_column_text(stmt, C_EDIZM),
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
G_MODULE_EXPORT void on_button20_clicked (GtkButton *button, gpointer label)
{
    // Определяем текущий код
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview_prih_stroki);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview_prih_stroki);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;
        // Получение кода
        gtk_tree_model_get (model, &iter,
                            C_ID_STR_NAKL, &id_data,
                            -1);
        ShowStrNakl(id_data);
    }
}

// Добавить запись
int AddPrihStroki()
{

    int id = 1;
    // Вставка новой записи в базу
    char* SQL="INSERT INTO [Строки накладной] ([Код товара],[Код накладной]) VALUES (";
    sprintf(str, "%s%d,%d)", SQL, id, CurNakl);
    ///ShowSQL(); // Тестовый вывод запроса
    // Записываем в БД
    if (sqlite3_exec(db, str, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 1;
    }
    // Получение последней вставленной строки
    char* SQL2 = "SELECT [Код строки накладной], t.[Товар], s.[Код товара], \
                [Код накладной], Приход, t.[Единица измерения] \
                FROM [Строки накладной] AS s JOIN Товары AS t \
                ON s.[Код товара] = t.[Код товара] \
                ORDER BY [Код строки накладной] DESC LIMIT 1";

    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_append (liststore_prih_stroki, &iter);
    gtk_list_store_set (liststore_prih_stroki, &iter,
                        C_ID_STR_NAKL, sqlite3_column_int(stmt, C_ID_STR_NAKL),
                        C_STR_NAKL, sqlite3_column_text(stmt, C_STR_NAKL),
                        C_ID_TOVAR, sqlite3_column_int(stmt, C_ID_TOVAR),
                        C_ID_NAKL, sqlite3_column_int(stmt, C_ID_NAKL),
                        C_PRIHOD,  sqlite3_column_int(stmt, C_PRIHOD),
                        C_EDIZM,  sqlite3_column_text(stmt, C_EDIZM),
                        -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Обновить запись
int ReloadNakl(int CurNakl)
{
    GtkTreeIter iter;
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview_prih_stroki);

    gtk_tree_selection_get_selected (selection, NULL, &iter);

    // Получение записи
    char* SQL = "SELECT [Код строки накладной], t.[Товар], s.[Код товара], \
                [Код накладной], Приход, t.[Единица измерения] \
                FROM [Строки накладной] AS s JOIN Товары AS t \
                ON s.[Код товара] = t.[Код товара] \
                WHERE [Код строки накладной]=";
    // Готовим SQL-запрос к БД
    lenstr = 0;
    lenstr+=sprintf(str, SQL);
    lenstr+=sprintf(str+lenstr, "%d", CurNakl);
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выполнение SQL-запроса и запись в буфер таблицы
    rc = sqlite3_step(stmt);

    gtk_list_store_set (liststore_prih_stroki, &iter,
                        C_ID_STR_NAKL, sqlite3_column_int(stmt, C_ID_STR_NAKL),
                        C_STR_NAKL, sqlite3_column_text(stmt, C_STR_NAKL),
                        C_ID_TOVAR, sqlite3_column_int(stmt, C_ID_TOVAR),
                        C_ID_NAKL, sqlite3_column_int(stmt, C_ID_NAKL),
                        C_PRIHOD,  sqlite3_column_int(stmt, C_PRIHOD),
                        C_EDIZM,  sqlite3_column_text(stmt, C_EDIZM),                            -1);

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);
    return 0;
}

// Удаление записи
void DeletePrihStroki ()
{
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (treeview_prih_stroki);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview_prih_stroki);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        gint id_data;

        // Получение кода
        gtk_tree_model_get (model, &iter,
                            C_ID_STR_NAKL, &id_data,
                            -1);

        // Удаление строки из буфера
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

        // Удаление записи из БД
        lenstr = 0;
        lenstr+=sprintf(str, "DELETE FROM [Строки накладной] WHERE [Код строки накладной]=");
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
    }
    sqlite3_finalize(stmt);
}

// Сохранение записи
void SaveNakl()
{
    char CurDate[256];

    int nomer_data=gtk_spin_button_get_value_as_int (gNomer);
    int day_data=gtk_spin_button_get_value_as_int (gDay);
    int month_data=gtk_spin_button_get_value_as_int (gMonth);
    int year_data=gtk_spin_button_get_value_as_int (gYear);

    sprintf(CurDate,"%4d-%02d-%02d", year_data, month_data, day_data);

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Накладные SET Номер=%d, ", nomer_data);
    lenstr+=sprintf(str+lenstr, "Дата='%s'", CurDate);
    lenstr+=sprintf(str+lenstr, " WHERE [Код накладной]=%d", CurNakl);
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

// Кнопка Добавить
G_MODULE_EXPORT void on_button21_clicked (GtkButton *button, gpointer label)
{
    AddPrihStroki();
}

// Кнопка Удалить
G_MODULE_EXPORT void on_button24_clicked (GtkButton *button, gpointer label)
{
    DeletePrihStroki ();
}

// Кнопка Закрыть
G_MODULE_EXPORT void on_button22_clicked (GtkButton *button, gpointer label)
{
    SaveNakl(); // Запись значений в БД
    ReloadPrih(CurNakl); // Обновление записи в окне Накладные
    gtk_widget_destroy (wPrihStroki);
}

// Вывод таблицы
void ShowPrihStroki(int nakl)
{
    CurNakl = nakl;

    bPrihStroki = gtk_builder_new ();
    gtk_builder_add_from_file (bPrihStroki, "main.glade", NULL);
    wPrihStroki = GTK_WIDGET(gtk_builder_get_object(bPrihStroki, "wPrihStroki"));
    liststore_prih_stroki =  GTK_LIST_STORE(gtk_builder_get_object(bPrihStroki, "liststore_prih_stroki"));
    treeview_prih_stroki =  GTK_TREE_VIEW(gtk_builder_get_object(bPrihStroki, "treeview_prih_stroki"));

    gNomer = GTK_SPIN_BUTTON(gtk_builder_get_object(bPrihStroki, "gNomer"));
    gDay = GTK_SPIN_BUTTON(gtk_builder_get_object(bPrihStroki, "gDay"));
    gMonth = GTK_SPIN_BUTTON(gtk_builder_get_object(bPrihStroki, "gMonth"));
    gYear = GTK_SPIN_BUTTON(gtk_builder_get_object(bPrihStroki, "gYear"));

    gtk_window_set_default_size (GTK_WINDOW (wPrihStroki), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wPrihStroki), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bPrihStroki, NULL);

    LoadNakl(nakl);
    if(LoadPrihStroki())
    {
        ShowError();
    }
    gtk_widget_show_all(wPrihStroki);
}
