/***************************
Ввод данных сотрудника
****************************/
#include "main.h"

int CurSotrudnik;
int CurOtdel;

// Обозначения полей таблицы Сотрудники
enum
{
    C_ID, C_NAME, C_OKLAD, C_KOD_OTDELA
};

// Обозначения полей таблицы Отделы
enum
{
    C_ID_OTDELA, C_OTDEL, C_BOSS
};


// Загрузка записи
int LoadRecord(int sotrudnik)
{

    CurOtdel = -1;
    char* SQL = "SELECT [Код сотрудника], Фамилия, Оклад, [Код отдела] \
                FROM Сотрудники     \
                WHERE [Код сотрудника]=";


    // Готовим SQL-запрос
    sprintf(str, "%s%d;",SQL, sotrudnik);
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
    if (sqlite3_column_text(stmt, C_NAME)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_NAME));
    }
    else
    {
        sprintf(str, "(не указано)");
    }
    gtk_entry_set_text(GTK_ENTRY(gSotr),str);
    gtk_spin_button_set_value(gOklad,sqlite3_column_int(stmt, C_OKLAD));
    CurOtdel = sqlite3_column_int(stmt, C_KOD_OTDELA);
    sqlite3_finalize(stmt);
    return 0;
}

// Заполнение списка отделов
int LoadCombobox()
{
    int CurStr=0;
    char* SQL = "SELECT [Код отдела], Отдел FROM Отделы";


    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        CurStr = sqlite3_column_int(stmt, C_ID_OTDELA);
        gtk_combo_box_text_insert_text (gOtdel, CurStr, (char*) sqlite3_column_text(stmt, C_OTDEL));
    }
    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }

    // 5. Освобождаем строку запроса
    sqlite3_finalize(stmt);

    // Устанавливаем активный элемент
    if (CurOtdel > 0)
    {
        gtk_combo_box_set_active(GTK_COMBO_BOX(gOtdel), CurOtdel-1);
    }
    return 0;
}

// Сохранение записи
void SaveRecord()
{
    char name_data[1000];
    int oklad_data;
    int otdel_data;

    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gSotr)));
    oklad_data = gtk_spin_button_get_value_as_int (gOklad);
    otdel_data = gtk_combo_box_get_active(GTK_COMBO_BOX(gOtdel))+1;

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Сотрудники SET Фамилия=");
    lenstr+=sprintf(str+lenstr, "'%s',Оклад=%d, [Код отдела]=%d WHERE [Код сотрудника]=%d", name_data, oklad_data, otdel_data, CurSotrudnik);
    ///ShowSQL(); // Тестовый вывод запроса
    // Записываем в БД
    if (sqlite3_exec(db, str, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }

}
// Кнопка Закрыть
G_MODULE_EXPORT void on_button12_clicked (GtkButton *button, gpointer label)
{
    SaveRecord(); // Запись значений в БД
    Reload(CurSotrudnik); // Обновление записи в окне Сотрудники
    gtk_widget_destroy (w_sotrudnik);
}


// Вывод окна
void ShowSotrudnik(int sotrudnik)
{
    CurSotrudnik = sotrudnik;

    b_sotrudnik = gtk_builder_new ();
    gtk_builder_add_from_file (b_sotrudnik, "main.glade", NULL);
    w_sotrudnik = GTK_WIDGET(gtk_builder_get_object(b_sotrudnik, "w_sotrudnik"));

    gSotr = GTK_WIDGET(gtk_builder_get_object(b_sotrudnik, "entry1"));
    gOklad = GTK_SPIN_BUTTON(gtk_builder_get_object(b_sotrudnik, "spinbutton1"));
    gOtdel = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(b_sotrudnik, "comboboxtext1"));

    gtk_window_set_default_size (GTK_WINDOW (w_sotrudnik), 700, 300);
    gtk_window_set_position(GTK_WINDOW(w_sotrudnik), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (b_sotrudnik, NULL);

    LoadRecord(sotrudnik);
    LoadCombobox();

    gtk_widget_show_all(w_sotrudnik);
}
