/***************************
Ввод товара
****************************/
#include "main.h"

int CurTovar;
int CurTovarPrint;
int CurOtdel;
int StrInPage = 4;  // Товаров на странице
int Pages=0;        // Число страниц
char CurFoto[256];
char CurDate[256];

char tovar_data[1000];
char articul_data[1000];
int kolvo_data;
char edizm_data[1000];

GtkWidget    *gTovar;
GtkWidget    *gArticul;
GtkSpinButton *gKolvo;
GtkWidget    *gEdizm;

// Поля таблицы Товары
enum
{
    C_ID, C_TOVAR, C_ARTIKUL, C_KOLVO, C_ED_IZM
};


// Вывод карточки товара на печать
static void DrawPageTovar(GtkPrintOperation *op, GtkPrintContext *context, int page_nr)
{
    cairo_t *cr;
    gchar buf[1000];
    cr = gtk_print_context_get_cairo_context (context);

    strcpy(tovar_data, gtk_entry_get_text(GTK_ENTRY(gTovar)));
    strcpy(articul_data, gtk_entry_get_text(GTK_ENTRY(gArticul)));
    kolvo_data = gtk_spin_button_get_value_as_int (gKolvo);

    // Выводим заголовок
    cairo_move_to (cr, 80.0, 10.0);
    cairo_set_font_size (cr, 5.0);
    cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_show_text (cr, "Карточка товара");
    cairo_stroke(cr);

    // Выводим поле "Товар"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 20.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Товар - %s", tovar_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Артикул"
    cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (cr, 10.0, 25.0);
    cairo_set_font_size (cr, 5.0);
    sprintf(buf, "Артикул - %s", articul_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим поле "Количество"
    cairo_move_to (cr, 10.0, 30.0);
    sprintf(buf, "Количество - %d", kolvo_data);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

}

// Печать карточки товара
void PrintTovar()
{
    GtkPrintOperation *op;

    op = gtk_print_operation_new ();

    gtk_print_operation_set_n_pages (op, 1);
    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "draw_page", G_CALLBACK (DrawPageTovar), NULL);
    gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (wTovar), NULL);
}

// Вывод страницы из списка печать
int DrawPageTovari(GtkPrintOperation *op, GtkPrintContext *context, int page_nr)
{
    cairo_t *cr;
    gchar buf[1000];

    cr = gtk_print_context_get_cairo_context (context);

    char* SQL = "SELECT [Код товара], Товар, Артикул, Количество, [Единица измерения] FROM Товары LIMIT ";

    // Готовим SQL-запрос
    sprintf(str, "%s%d OFFSET %d;",SQL, StrInPage, page_nr*StrInPage);
    ///ShowSQL(); // Тестовый вывод запроса
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, str, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 3;
    }

    // Выводим заголовок
    cairo_move_to (cr, 80.0, 10.0);
    cairo_set_font_size (cr, 5.0);
    cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    sprintf(buf, "Страница № %d из %d", ++page_nr, Pages);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);

    // Выводим шапку таблицы
    cairo_set_line_width (cr, 1.0);
    cairo_move_to (cr, 10.0, 20.0);
    cairo_line_to (cr, 145, 20);
    cairo_stroke (cr);
    cairo_move_to (cr, 12.0, 26.0);
    sprintf(buf, "Товар             Артикул          Количество     Ед. изм.");
    cairo_show_text (cr, buf);
    cairo_stroke(cr);
    cairo_move_to (cr, 10.0, 28.0);
    cairo_line_to (cr, 145.0, 28.0);
    cairo_stroke (cr);

    double CurStr = 35.0;
    // Цикл по SQL-запросу и печать строк
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // Выводим поле "Товар"
        cairo_select_font_face(cr, "sans serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_move_to (cr, 10.0, CurStr);
        cairo_set_font_size (cr, 5.0);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_TOVAR));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        // Выводим поле "Артикул"
        cairo_move_to (cr, 50.0, CurStr);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_ARTIKUL));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        // Выводим поле "Количество"
        cairo_move_to (cr, 80.0, CurStr);
        sprintf(buf, "%3d", sqlite3_column_int(stmt, C_KOLVO));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);

        // Выводим поле "Ед. изм."
        cairo_move_to (cr, 120.0, CurStr);
        sprintf(buf, "%s", sqlite3_column_text(stmt, C_ED_IZM));
        cairo_show_text (cr, buf);
        cairo_stroke(cr);


        CurStr += 5.0; // Переходим на следующую строку
    }

    if(rc != SQLITE_DONE)
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 4;
    }


    sqlite3_finalize(stmt);
    return 0;
}


// Начало печати списка  - подсчет страниц
void BeginPrint(GtkPrintOperation *oper, GtkPrintContext *context)
{
    Pages=0;


    char* SQL = "SELECT count(Товар) FROM Товары";


    if(sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        sprintf(str, "Ошибка подготовки SQL-запроса: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    // Цикл по SQL-запросу и запись в буфер таблицы
    rc = sqlite3_step(stmt);
    Pages = sqlite3_column_int(stmt, 0)/StrInPage;
    if (sqlite3_column_int(stmt, 0)%StrInPage)
    {
        Pages++;
    }

    // Освобождаем строку запроса
    sqlite3_finalize(stmt);

    gtk_print_operation_set_n_pages(oper, Pages);
}

// Печать списка товаров
void PrintTovari()
{
    GtkPrintOperation *op;

    op = gtk_print_operation_new ();

    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "begin_print", G_CALLBACK (BeginPrint), NULL);
    g_signal_connect (op, "draw_page", G_CALLBACK (DrawPageTovari), NULL);
    gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (wTovar), NULL);
}

// Загрузка записи
int LoadRecord(int tovar)
{

    char* SQL = "SELECT [Код товара], Товар, Артикул, Количество, [Единица измерения] \
                FROM Товары WHERE [Код товара]=";

    // Готовим SQL-запрос
    sprintf(str, "%s%d;",SQL, tovar);
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
    if (sqlite3_column_text(stmt, C_TOVAR)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_TOVAR));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gTovar),str);
    if (sqlite3_column_text(stmt, C_ARTIKUL)>0)
    {
        sprintf(str, "%s",sqlite3_column_text(stmt, C_ARTIKUL));
    }
    else
    {
        sprintf(str, " ");
    }
    gtk_entry_set_text(GTK_ENTRY(gArticul),str);
    gtk_spin_button_set_value(gKolvo,sqlite3_column_int(stmt, C_KOLVO));
    sprintf(str, "%s",sqlite3_column_text(stmt, C_ED_IZM));
    gtk_entry_set_text(GTK_ENTRY(gEdizm),str);

    sqlite3_finalize(stmt);
    return 0;
}


// Сохранение записи
void SaveRecord()
{
    strcpy(tovar_data, gtk_entry_get_text(GTK_ENTRY(gTovar)));
    strcpy(articul_data, gtk_entry_get_text(GTK_ENTRY(gArticul)));
    kolvo_data = gtk_spin_button_get_value_as_int (gKolvo);
    strcpy(edizm_data, gtk_entry_get_text(GTK_ENTRY(gEdizm)));

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Товары SET Товар=");
    lenstr+=sprintf(str+lenstr, "'%s', Артикул ='%s', Количество=%d", tovar_data, articul_data, kolvo_data);
    lenstr+=sprintf(str+lenstr, ", [Единица измерения] ='%s'", edizm_data);
    lenstr+=sprintf(str+lenstr, " WHERE [Код товара]=%d", CurTovar);
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
    Reload(CurTovar); // Обновление записи в окне Сотрудники
    gtk_widget_destroy (wTovar);
}

// Кнопка Печать
G_MODULE_EXPORT void on_button14_clicked (GtkButton *button, gpointer label)
{
    PrintTovar();
}

// Кнопка Печать списка
G_MODULE_EXPORT void on_button15_clicked (GtkButton *button, gpointer label)
{
    PrintTovari();
}


// Вывод окна
void ShowTovar(int tovar)
{
    CurTovar = tovar;

    bTovar = gtk_builder_new ();
    gtk_builder_add_from_file (bTovar, "main.glade", NULL);
    wTovar = GTK_WIDGET(gtk_builder_get_object(bTovar, "wTovar"));

    gTovar = GTK_WIDGET(gtk_builder_get_object(bTovar, "entry1"));
    gArticul = GTK_WIDGET(gtk_builder_get_object(bTovar, "entry2"));
    gKolvo = GTK_SPIN_BUTTON(gtk_builder_get_object(bTovar, "spinbutton1"));
    gEdizm = GTK_WIDGET(gtk_builder_get_object(bTovar, "entry4"));

    gtk_window_set_default_size (GTK_WINDOW (wTovar), 700, 300);
    gtk_window_set_position(GTK_WINDOW(wTovar), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (bTovar, NULL);

    LoadRecord(tovar);

    gtk_widget_show_all(wTovar);
}
