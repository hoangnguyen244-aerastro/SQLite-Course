#include "main.h"

// Структуры данных для виджета
GtkListStore        *buffer;    // Буфер таблицы
GtkTreeView         *table;     // Таблица
GtkTreeViewColumn   *column;    // Отображаемая колонка
GtkTreeIter         iter;       // Итератор таблицы (текущая строка)
GtkCellRenderer     *renderer;  // Рендер таблицы (текущая ячейка)

// Обозначения полей
enum
{
    C_NUM, C_NAME, C_OKLAD
};

// Структуры данных для базы данных
sqlite3_stmt* stmt; // строка запроса к БД
char *err = 0;
int rc = 0;

char* SQL1 = "DROP TABLE IF EXISTS Сотрудники; \
            CREATE TABLE Сотрудники(N,Фамилия,Оклад); \
            INSERT INTO Сотрудники VALUES(1,'Иванов',20000);  \
            INSERT INTO Сотрудники VALUES(2,'Петров',25000);  \
            INSERT INTO Сотрудники VALUES(3,'Сидоров',30000);  \
            ";
char* SQL2 = "SELECT N, Фамилия, Оклад FROM Сотрудники;";
char str[1000];
int lenstr=0;

int ShowSQL()
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    str);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return 0;
}

// Открываем БД
void StartDB()
{
    if( sqlite3_open("b6.db", &db))
    {
        sprintf(str, "Ошибка открытия БД: %s\n", sqlite3_errmsg(db));
    }
}

// Закрываем БД
void EndDB()
{
    sqlite3_close(db);
}

// Запись в БД
int SaveDB()
{
    if (sqlite3_exec(db, SQL1, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return 2;
    }
    return 0;
}


// Сохранение таблицы Сотрудники
void SaveTable()
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    gint row_count = 0;

    char* SQLstart = "DROP TABLE IF EXISTS Сотрудники; \
            CREATE TABLE Сотрудники(N,Фамилия,Оклад);";

    // Очищаем таблицу Сотрудники
    if (sqlite3_exec(db, SQLstart, 0, 0, &err))
    {
        sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
        sqlite3_free(err);
        return;
    }

    // Получаем первый итератор в списке
    model = gtk_tree_view_get_model (table);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    // Цикл по строкам буфера
    while (valid)
    {
        gint   num_data;
        gchar *name_data;
        gint   oklad_data;

        gtk_tree_model_get (model, &iter,
                            C_NUM, &num_data,
                            C_NAME, &name_data,
                            C_OKLAD, &oklad_data,
                            -1);

        // Готовим SQL-запрос
        lenstr = 0;
        lenstr+=sprintf(str, "INSERT INTO Сотрудники VALUES(");
        lenstr+=sprintf(str+lenstr, "%d,'%s',%d\n", num_data, name_data, oklad_data);
        lenstr+=sprintf(str+lenstr, ");");
        //ShowSQL(); // Тестовый вывод запроса
        // Записываем в БД
        if (sqlite3_exec(db, str, 0, 0, &err))
        {
            sprintf(str, "Ошибка выполнения SQL-запроса: %sn", err);
            sqlite3_free(err);
            return;
        }

        g_free (name_data);
        row_count ++;
        valid = gtk_tree_model_iter_next (model, &iter);
    }
}

G_MODULE_EXPORT void on_cellrenderertext1_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_NUM, atoi(new_text), -1 );
    }
}


G_MODULE_EXPORT void on_cellrenderertext2_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_NAME, new_text, -1 );
    }
}

G_MODULE_EXPORT void on_cellrenderertext3_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (table);
    if (gtk_tree_model_get_iter_from_string(model, &iter, path))
    {
        gtk_list_store_set(GTK_LIST_STORE (model), &iter, C_OKLAD, atoi(new_text), -1 );
    }
}

// Загрузка таблицы из БД в буфер виджета
int LoadDB()
{
    // Готовим SQL-запрос к БД
    if(sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL) != SQLITE_OK)
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
                            C_NUM, sqlite3_column_int(stmt, C_NUM),
                            C_NAME, sqlite3_column_text(stmt, C_NAME),
                            C_OKLAD, (guint) sqlite3_column_int(stmt, C_OKLAD),
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

// Кнопка Закрыть
G_MODULE_EXPORT void on_button7_clicked (GtkButton *button, gpointer label)
{
    SaveTable(); // Запись буфера в БД
    gtk_widget_destroy (window2);
}

void ShowError ()
{
    GtkWidget *dialog = NULL;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window1), GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, str);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

// Вывод таблицы
void ShowTable(GtkWidget *win)
{

    builder2 = gtk_builder_new ();
    gtk_builder_add_from_file (builder2, "main.glade", NULL);
    window2 = GTK_WIDGET(gtk_builder_get_object(builder2, "window2"));
    buffer =  GTK_LIST_STORE(gtk_builder_get_object(builder2, "liststore1"));
    table =  GTK_TREE_VIEW(gtk_builder_get_object(builder2, "treeview1"));

    gtk_window_set_default_size (GTK_WINDOW (window2), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);

    gtk_builder_connect_signals (builder2, NULL);

    if(LoadDB())
    {
        ShowError();
    }
    gtk_widget_show_all(window2);
}
