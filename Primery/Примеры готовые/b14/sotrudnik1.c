/***************************
Ввод данных сотрудника
****************************/
#include "main.h"

int CurSotrudnik;
int CurOtdel;
char CurFoto[256];
char CurDate[256];
int gDay, gMonth, gYear;

// Обозначения полей таблицы Сотрудники
enum
{
    C_ID, C_NAME, C_OKLAD, C_KOD_OTDELA, C_FOTO, C_DAY, C_MONTH, C_YEAR
};

// Обозначения полей таблицы Отделы
enum
{
    C_ID_OTDELA, C_OTDEL, C_BOSS
};

// Вывод страницы на печать
static void DrawPageSotrudnik(GtkPrintOperation *op, GtkPrintContext *context, int page_nr)
{
  cairo_t *cr;
  PangoLayout *layout;
  PangoFontDescription *desc;
  gchar buf[256]="Печать";

  cr = gtk_print_context_get_cairo_context (context);

  // рисуем красный прямоугольник по ширине бумаги
  cairo_set_source_rgb (cr, 1.0, 0, 0);
  cairo_rectangle (cr, 0, 0, gtk_print_context_get_width (context), 50);
  cairo_fill (cr);

  /* рисуем некоторые линии */
  cairo_move_to (cr, 20, 10);
  cairo_line_to (cr, 40, 20);
  cairo_arc (cr, 60, 60, 20, 0, M_PI);
  cairo_line_to (cr, 80, 20);

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, 5);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

  cairo_stroke (cr);


        // Зеленая линия
    cairo_set_source_rgb( cr, 0, 65535, 0);
    cairo_set_line_width (cr, 6.0);
    cairo_move_to (cr, 10, 75);
    cairo_line_to (cr, 125, 75);
    cairo_stroke (cr);
    // Красный прямоугольник
    cairo_set_source_rgb( cr, 65535, 0, 0);
    cairo_rectangle( cr, 10, 100, 50, 50 );
    cairo_fill( cr );
    // Синий круг
    cairo_set_source_rgb( cr, 0, 0, 65535);
    cairo_arc (cr, 40, 125, 25, 0, 2*M_PI);
    cairo_fill( cr );

  /* рисуем некоторый текст */
  layout = gtk_print_context_create_pango_layout (context);
  pango_layout_set_text (layout, "Сотрудники", -1);
  desc = pango_font_description_from_string ("arial 28");
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  cairo_move_to (cr, 30, 20);
  pango_cairo_layout_path (cr, layout);

  // Вторая надпись
      cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 100.0);

    cairo_move_to (cr, 10.0, 120.0);
    cairo_show_text (cr, buf);
    cairo_stroke(cr);


    // Рисуем картинку
//  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("C:\Data\1\man1.jpg",NULL);
//  pixbuf =  gdk_pixbuf_scale_simple (pixbuf,197, 210, GDK_INTERP_HYPER);
//  gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);



  /* схема шрифта */
  cairo_set_source_rgb (cr, 0.93, 1.0, 0.47);
  cairo_set_line_width (cr, 0.5);
  cairo_stroke_preserve (cr);

  /* заполнение шрифта */
  cairo_set_source_rgb (cr, 0, 0.0, 1.0);
  cairo_fill (cr);

  g_object_unref (layout);
}


// Печать сотрудника
void PrintSotrudnik()
{
    GtkPrintOperation *op;
    GtkPrintOperationResult res;
    GtkPrintSettings *settings;

    op = gtk_print_operation_new ();

    gtk_print_operation_set_n_pages (op, 1);
    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "draw_page", G_CALLBACK (DrawPageSotrudnik), NULL);
    res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (w_sotrudnik), NULL);
}


// Загрузка записи
int LoadRecord(int sotrudnik)
{

    CurOtdel = -1;

    char* SQL = "SELECT [Код сотрудника], Фамилия, Оклад, [Код отдела], Фото, \
                strftime('%d',[Дата рождения]), \
                strftime('%m',[Дата рождения]), \
                strftime('%Y',[Дата рождения]) \
                FROM Сотрудники WHERE [Код сотрудника]=";

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
    // Получение имени фото
    if (sqlite3_column_text(stmt, C_FOTO) > 0)
    {
        sprintf(CurFoto,"%s", sqlite3_column_text(stmt, C_FOTO));
        gtk_image_set_from_file (GTK_IMAGE(gFoto), CurFoto);
    }
    else
    {
        strcpy(CurFoto,"");
    }
    // Получение даты рождения
    if (sqlite3_column_text(stmt, C_DAY) > 0)
    {
        gDay = atoi((char*) sqlite3_column_text(stmt, C_DAY));
        gMonth = atoi((char*)sqlite3_column_text(stmt, C_MONTH))-1;
        gYear = atoi((char*)sqlite3_column_text(stmt, C_YEAR));

        gtk_calendar_select_month(gCalendar, gMonth, gYear);
        gtk_calendar_select_day(gCalendar, gDay);
    }
    else
    {
        gDay=1;
        gMonth=1;
        gYear=1980;
    }


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
        gtk_combo_box_text_insert_text (gOtdel, CurStr, (char*) sqlite3_column_text(stmt, C_NAME));
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
    gtk_calendar_get_date(gCalendar, &gYear, &gMonth, &gDay);
    sprintf(CurDate,"%4d-%02d-%02d", gYear, ++gMonth, gDay);

    // Готовим SQL-запрос
    lenstr = 0;
    lenstr+=sprintf(str, "UPDATE Сотрудники SET Фамилия=");
    lenstr+=sprintf(str+lenstr, "'%s',Оклад=%d, [Код отдела]=%d", name_data, oklad_data, otdel_data);
    lenstr+=sprintf(str+lenstr, ", Фото='%s', [Дата рождения]='%s'", CurFoto, CurDate);
    lenstr+=sprintf(str+lenstr, " WHERE [Код сотрудника]=%d", CurSotrudnik);
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

// Кнопка Печать
G_MODULE_EXPORT void on_button14_clicked (GtkButton *button, gpointer label)
{
    PrintSotrudnik();
}

// Кнопка Выбрать фото
G_MODULE_EXPORT void on_button13_clicked (GtkButton *button, gpointer label)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          GTK_WINDOW(w_sotrudnik),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    // Устанавливаем текущий каталог
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), ".");
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        strcpy(CurFoto, filename);
        gtk_image_set_from_file (GTK_IMAGE(gFoto), CurFoto);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
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
    gFoto = GTK_WIDGET(gtk_builder_get_object(b_sotrudnik, "image1"));
    gCalendar = GTK_CALENDAR(gtk_builder_get_object(b_sotrudnik, "calendar1"));

    gtk_window_set_default_size (GTK_WINDOW (w_sotrudnik), 700, 300);
    gtk_window_set_position(GTK_WINDOW(w_sotrudnik), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (b_sotrudnik, NULL);

    LoadRecord(sotrudnik);
    LoadCombobox();

    gtk_widget_show_all(w_sotrudnik);
}
