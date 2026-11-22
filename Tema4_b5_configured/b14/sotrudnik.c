/***************************
Ввод данных сотрудника
****************************/
#include "main.h"

int CurSotrudnik;

// Обозначения полей
enum
{
    C_ID, C_NAME, C_OKLAD
};


// Загрузка записи
int LoadRecord(int sotrudnik)
{

    char* SQL = "SELECT [Код сотрудника], Фамилия, Оклад \
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
    sqlite3_finalize(stmt);
    return 0;
}

// Сохранение записи
void SaveRecord()
{
    char name_data[1000];
    int oklad_data;

    strcpy(name_data, gtk_entry_get_text(GTK_ENTRY(gSotr)));
    oklad_data = gtk_spin_button_get_value_as_int (gOklad);

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Сотрудники SET Фамилия=");
    lenstr+=sprintf(str+lenstr, "'%s',Оклад=%d WHERE [Код сотрудника]=%d", name_data, oklad_data, CurSotrudnik);
    ShowSQL(); // Тестовый вывод запроса
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

    gSotr = GTK_ENTRY(gtk_builder_get_object(b_sotrudnik, "entry1"));
    gOklad = GTK_SPIN_BUTTON(gtk_builder_get_object(b_sotrudnik, "spinbutton1"));

    gtk_window_set_default_size (GTK_WINDOW (w_sotrudnik), 700, 300);
    gtk_window_set_position(GTK_WINDOW(w_sotrudnik), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (b_sotrudnik, NULL);

    LoadRecord(sotrudnik);

    gtk_widget_show_all(w_sotrudnik);
}
