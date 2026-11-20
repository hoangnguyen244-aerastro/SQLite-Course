/*******************************
Ввод строки приходной накладной
********************************/
#include "main.h"

int CurStrNakl;
int CurTovar;

char stroka_data[1000];
int id_tovar_data;
int prihod_data;

GtkWidget       *gStrNakl;
GtkComboBoxText *gTovar;
GtkWidget       *gEdizm;
GtkSpinButton   *gPrihod;

// Поля таблицы "Строки накладной"
enum
{
    C_ID_STR_NAKL, C_TOVAR, C_ID_TOVAR, C_ID_NAKL, C_PRIHOD, C_EDIZM
};

enum
{
    C_ID, C_NAME_TOVAR
};


// Загрузка строки накладной
int LoadStrNakl(int StrNakl)
{
    CurTovar = -1;

    char* SQL = "SELECT [Код строки накладной], t.[Товар], s.[Код товара], \
                [Код накладной], Приход, t.[Единица измерения] \
                FROM [Строки накладной] AS s JOIN Товары AS t \
                ON s.[Код товара] = t.[Код товара] \
                WHERE [Код строки накладной]=";

    // Готовим SQL-запрос
    sprintf(str, "%s%d;",SQL, StrNakl);
    ///ShowSQL(); // Тестовый вывод запроса
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }
    // Выполнение SQL-запроса
    rc = sqlite3_step(stmt);

    // Запись в виджеты
    CurTovar = sqlite3_column_int(stmt, C_ID_TOVAR); // Получения текущего кода товара
    // Вывод количества
    gtk_spin_button_set_value(gPrihod,sqlite3_column_int(stmt, C_PRIHOD));
    // Вывод единицы измерения
    if (sqlite3_column_text(stmt, C_EDIZM)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_EDIZM));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gEdizm),str);

    sqlite3_finalize(stmt);
    return 0;
}

// Заполнение списка товаров
int LoadCombobox()
{

    GtkTreeIter iter;
    GtkTreeIter CurIter;

    int CurStr=0;
    int CurId=0;

    char* SQL = "SELECT [Код товара], Товар FROM Товары ORDER BY Товар";


    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
      if (CurTovar == sqlite3_column_int(stmt, C_ID))
        {
            CurId = CurStr;

        }
        gtk_combo_box_text_append_text (gTovar, (char*) sqlite3_column_text(stmt, C_NAME_TOVAR));
        CurStr++;
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);

    // Устанавливаем активный элемент
    if (CurTovar > 0)
    {
        gtk_combo_box_set_active(GTK_COMBO_BOX(gTovar), CurId);
    }
    return 0;
}

// Сохранение записи
void SavePrihStroka()
{
    int index = gtk_combo_box_get_active (GTK_COMBO_BOX(gTovar));
    // Получаем код отдела по индексу
    lenstr = 0;
    lenstr+=sprintf(str, "SELECT [Код товара], Товар FROM Товары ORDER BY Товар ");
    lenstr+=sprintf(str+lenstr, "LIMIT 1 OFFSET %d", index);
    ///ShowSQL(); // Тестовый вывод запроса
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }
    rc = sqlite3_step(stmt);
    int id_tovar_data = sqlite3_column_int(stmt, C_ID);
    // Освобождаем строку запроса
    sqlite3_finalize(stmt);

    prihod_data = gtk_spin_button_get_value_as_int (gPrihod);

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE [Строки накладной] SET ");
    lenstr+=sprintf(str+lenstr, "[Код товара] = %d, Приход=%d", id_tovar_data, prihod_data);
    lenstr+=sprintf(str+lenstr, " WHERE [Код строки накладной]=%d", CurStrNakl);
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

// Кнопка Закрыть
G_MODULE_EXPORT void on_button25_clicked (GtkButton *button, gpointer label)
{
    SavePrihStroka(); // Запись значений в БД
    ReloadNakl(CurStrNakl); // Обновление записи в окне Накладные
    gtk_widget_destroy (wPrihStroka);
}

// Вывод окна
void ShowStrNakl(int nakl)
{
    CurStrNakl = nakl;

    bPrihStroka = gtk_builder_new ();
    gtk_builder_add_from_file (bPrihStroka, "main.glade", NULL);
    wPrihStroka = GTK_WIDGET(gtk_builder_get_object(bPrihStroka, "wPrihStroka"));

    gTovar = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(bPrihStroka, "gTovar"));
    gEdizm = GTK_WIDGET(gtk_builder_get_object(bPrihStroka, "gEdizm"));
    gPrihod = GTK_SPIN_BUTTON(gtk_builder_get_object(bPrihStroka, "gPrihod"));

    gtk_window_set_default_size (GTK_WINDOW (wPrihStroka), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wPrihStroka), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bPrihStroka, NULL);

    LoadStrNakl(nakl);
    LoadCombobox();

    gtk_widget_show_all(wPrihStroka);
}
