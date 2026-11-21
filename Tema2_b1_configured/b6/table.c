#include "main.h"
#include <stdio.h>
#include <string.h>

/* Тема 2, Задание 1: четыре редактируемых столбца. */
sqlite3 *db = NULL;
GtkListStore *buffer = NULL;
GtkTreeView *table = NULL;
GtkTreeIter iter;

enum { C_NUM, C_NAME, C_FIRST_NAME, C_OKLAD };

static char str[1024];
static const char *SQL1 =
    "DROP TABLE IF EXISTS Сотрудники;"
    "CREATE TABLE Сотрудники (N INTEGER, Фамилия TEXT, Имя TEXT, Оклад INTEGER);"
    "INSERT INTO Сотрудники VALUES (1,'Иванов','Иван',20000);"
    "INSERT INTO Сотрудники VALUES (2,'Петров','Петр',25000);"
    "INSERT INTO Сотрудники VALUES (3,'Сидоров','Алексей',30000);";

static const char *SQL2 =
    "SELECT N, Фамилия, Имя, Оклад FROM Сотрудники ORDER BY N;";

void StartDB(void)
{
    if (sqlite3_open("b6.db", &db) != SQLITE_OK)
        g_snprintf(str, sizeof(str), "Ошибка открытия БД: %s",
                   db ? sqlite3_errmsg(db) : "SQLite");
}
void EndDB(void)
{
    if (db) { sqlite3_close(db); db = NULL; }
}
int SaveDB(void)
{
    char *error = NULL;
    if (!db) return 1;
    if (sqlite3_exec(db, SQL1, NULL, NULL, &error) != SQLITE_OK)
    {
        g_snprintf(str, sizeof(str), "Ошибка SQL: %s",
                   error ? error : sqlite3_errmsg(db));
        sqlite3_free(error);
        return 2;
    }
    return 0;
}
void ShowError(void)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        window1 ? GTK_WINDOW(window1) : NULL,
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "%s", str);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Сохранение отредактированных ячеек при нажатии «Закрыть». 
   Параметры SQL предотвращают ошибки при вводе апострофов. */
static gboolean SaveTable(void)
{
    GtkTreeModel *model;
    GtkTreeIter item;
    gboolean valid;
    sqlite3_stmt *stmt = NULL;
    char *err = NULL;
    int rc;
    if (!db || !table) return FALSE;

    rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &err);
    if (rc != SQLITE_OK) goto fail;

    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS Сотрудники "
        "(N INTEGER, Фамилия TEXT, Имя TEXT, Оклад INTEGER);"
        "DELETE FROM Сотрудники;", NULL, NULL, &err);
    if (rc != SQLITE_OK) goto fail;

    rc = sqlite3_prepare_v2(db,
        "INSERT INTO Сотрудники (N,Фамилия,Имя,Оклад) VALUES (?,?,?,?);",
        -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto fail;

    model = gtk_tree_view_get_model(table);
    valid = gtk_tree_model_get_iter_first(model, &item);
    while (valid)
    {
        gint number = 0, salary = 0;
        gchar *surname = NULL, *firstname = NULL;
        gtk_tree_model_get(model, &item,
            C_NUM, &number, C_NAME, &surname,
            C_FIRST_NAME, &firstname, C_OKLAD, &salary, -1);

        sqlite3_bind_int(stmt, 1, number);
        sqlite3_bind_text(stmt, 2, surname ? surname : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, firstname ? firstname : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, salary);
        rc = sqlite3_step(stmt);
        g_free(surname);
        g_free(firstname);
        if (rc != SQLITE_DONE) goto fail;
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        valid = gtk_tree_model_iter_next(model, &item);
    }
    sqlite3_finalize(stmt);
    stmt = NULL;
    rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, &err);
    if (rc != SQLITE_OK) goto fail;
    return TRUE;

fail:
    g_snprintf(str, sizeof(str), "Ошибка сохранения: %s",
               err ? err : sqlite3_errmsg(db));
    sqlite3_free(err);
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
    return FALSE;
}

/* Обработчики каждого редактируемого столбца. */
static void edit_cell(gchar *path, gint index, gchar *value)
{
    GtkTreeIter item;
    GtkTreeModel *model = gtk_tree_view_get_model(table);
    if (!gtk_tree_model_get_iter_from_string(model, &item, path)) return;
    if (index == C_NUM || index == C_OKLAD)
        gtk_list_store_set(GTK_LIST_STORE(model), &item, index,
                           (gint)g_ascii_strtoll(value, NULL, 10), -1);
    else
        gtk_list_store_set(GTK_LIST_STORE(model), &item, index, value, -1);
}
void on_cellrenderertext1_edited(GtkCellRendererText *r, gchar *p, gchar *v, gpointer d)
{ (void)r; (void)d; edit_cell(p, C_NUM, v); }
void on_cellrenderertext2_edited(GtkCellRendererText *r, gchar *p, gchar *v, gpointer d)
{ (void)r; (void)d; edit_cell(p, C_NAME, v); }
void on_cellrenderertext3_edited(GtkCellRendererText *r, gchar *p, gchar *v, gpointer d)
{ (void)r; (void)d; edit_cell(p, C_FIRST_NAME, v); }
void on_cellrenderertext4_edited(GtkCellRendererText *r, gchar *p, gchar *v, gpointer d)
{ (void)r; (void)d; edit_cell(p, C_OKLAD, v); }

static int LoadDB(void)
{
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(db, SQL2, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto fail;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        gtk_list_store_append(buffer, &iter);
        gtk_list_store_set(buffer, &iter,
            C_NUM, sqlite3_column_int(stmt, 0),
            C_NAME, sqlite3_column_text(stmt, 1),
            C_FIRST_NAME, sqlite3_column_text(stmt, 2),
            C_OKLAD, sqlite3_column_int(stmt, 3), -1);
    }
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) return 0;
    g_snprintf(str, sizeof(str), "Ошибка чтения: %s", sqlite3_errmsg(db));
    return 1;
fail:
    g_snprintf(str, sizeof(str), "Ошибка SELECT: %s", sqlite3_errmsg(db));
    if (stmt) sqlite3_finalize(stmt);
    return 1;
}

static void on_button7_clicked(GtkButton *btn, gpointer data)
{
    (void)btn; (void)data;
    if (!SaveTable()) { ShowError(); return; }
    gtk_widget_destroy(window2);
}

void ShowTable(GtkWidget *win)
{
    GtkWidget *close_btn;
    GtkCellRenderer *cell;
    gint i;
    GError *error = NULL;
    (void)win;
    builder2 = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder2, "main.glade", &error))
    {
        g_snprintf(str, sizeof(str), "Не найден main.glade: %s",
                   error ? error->message : "");
        if (error) g_error_free(error);
        ShowError();
        return;
    }
    window2 = GTK_WIDGET(gtk_builder_get_object(builder2, "window2"));
    buffer = GTK_LIST_STORE(gtk_builder_get_object(builder2, "liststore1"));
    table = GTK_TREE_VIEW(gtk_builder_get_object(builder2, "treeview1"));
    close_btn = GTK_WIDGET(gtk_builder_get_object(builder2, "button7"));
    if (!window2 || !buffer || !table || !close_btn)
    {
        g_snprintf(str, sizeof(str), "Ошибка элементов Glade");
        ShowError();
        return;
    }

    /* Все четыре ячейки редактируемы; сигналы привязаны явно. */
    for (i = 0; i < 4; ++i) {
        GtkTreeViewColumn *col = gtk_tree_view_get_column(table, i);
        GList *cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(col));
        cell = cells ? GTK_CELL_RENDERER(cells->data) : NULL;
        if (cell) {
            g_object_set(cell, "editable", TRUE, NULL);
            if (i == 0) g_signal_connect(cell, "edited", G_CALLBACK(on_cellrenderertext1_edited), NULL);
            if (i == 1) g_signal_connect(cell, "edited", G_CALLBACK(on_cellrenderertext2_edited), NULL);
            if (i == 2) g_signal_connect(cell, "edited", G_CALLBACK(on_cellrenderertext3_edited), NULL);
            if (i == 3) g_signal_connect(cell, "edited", G_CALLBACK(on_cellrenderertext4_edited), NULL);
        }
        g_list_free(cells);
    }
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_button7_clicked), NULL);

    gtk_window_set_default_size(GTK_WINDOW(window2), 720, 320);
    gtk_window_set_position(GTK_WINDOW(window2), GTK_WIN_POS_CENTER);
    if (LoadDB()) ShowError();
    gtk_widget_show_all(window2);
}
