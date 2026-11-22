#include "main.h"

/* Тема 3, задание 2: редактирование, добавление и удаление отделов. */
static GtkListStore *buffer2 = NULL;
static GtkTreeView *table2 = NULL;
enum { C_OTDEL_ID, C_OTDEL, C_BOSS };

static void ShowDepartmentError(const char *message)
{
    g_snprintf(str, sizeof(str), "%s", message ? message : "Ошибка SQLite");
    ShowError();
}

/* Сохранение текущей строки без пересоздания таблицы и потери ключей. */
static gboolean SaveCurrentDepartment(GtkTreeIter *it)
{
    GtkTreeModel *model = GTK_TREE_MODEL(buffer2);
    gint id = 0;
    gchar *department = NULL, *boss = NULL;
    sqlite3_stmt *q = NULL;
    int result;
    gtk_tree_model_get(model, it, C_OTDEL_ID, &id,
                       C_OTDEL, &department, C_BOSS, &boss, -1);
    result = sqlite3_prepare_v2(db,
        "UPDATE Отделы SET Отдел=?, Руководитель=? WHERE [Код отдела]=?;",
        -1, &q, NULL);
    if (result == SQLITE_OK) {
        sqlite3_bind_text(q, 1, department ? department : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(q, 2, boss ? boss : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(q, 3, id);
        result = sqlite3_step(q);
    }
    if (result != SQLITE_DONE) {
        ShowDepartmentError(sqlite3_errmsg(db));
    }
    if (q) sqlite3_finalize(q);
    g_free(department);
    g_free(boss);
    return result == SQLITE_DONE;
}

static gboolean GetSelectedRow(GtkTreeIter *it)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(table2);
    return gtk_tree_selection_get_selected(selection, NULL, it);
}

static void CellEdited(gchar *path, gint column, gchar *value)
{
    GtkTreeIter it;
    GtkTreeModel *model = GTK_TREE_MODEL(buffer2);
    if (!gtk_tree_model_get_iter_from_string(model, &it, path)) return;
    gtk_list_store_set(buffer2, &it, column, value, -1);
    SaveCurrentDepartment(&it);
}

static void on_cellrenderertext5_edited(GtkCellRendererText *renderer,
     gchar *path, gchar *new_text, gpointer data)
{
    (void)renderer; (void)data;
    CellEdited(path, C_OTDEL, new_text);
}

static void on_cellrenderertext6_edited(GtkCellRendererText *renderer,
     gchar *path, gchar *new_text, gpointer data)
{
    (void)renderer; (void)data;
    CellEdited(path, C_BOSS, new_text);
}

/* Добавление записи с автоматическим назначением ключа SQLite. */
static void on_button9_clicked(GtkButton *button, gpointer data)
{
    sqlite3_stmt *q = NULL;
    GtkTreeIter it;
    int result;
    (void)button; (void)data;
    result = sqlite3_prepare_v2(db,
        "INSERT INTO Отделы(Отдел, Руководитель) VALUES(?, ?);",
        -1, &q, NULL);
    if (result == SQLITE_OK) {
        sqlite3_bind_text(q, 1, "Новый отдел", -1, SQLITE_STATIC);
        sqlite3_bind_text(q, 2, "Не назначен", -1, SQLITE_STATIC);
        result = sqlite3_step(q);
    }
    if (result == SQLITE_DONE) {
        gint id = (gint)sqlite3_last_insert_rowid(db);
        gtk_list_store_append(buffer2, &it);
        gtk_list_store_set(buffer2, &it,
            C_OTDEL_ID, id, C_OTDEL, "Новый отдел",
            C_BOSS, "Не назначен", -1);
        gtk_tree_selection_select_iter(
            gtk_tree_view_get_selection(table2), &it);
    } else ShowDepartmentError(sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
}

/* Удаление записи: сначала SQLite, только потом строка модели.
   Защитим отделы, к которым привязаны сотрудники. */
static void on_button10_clicked(GtkButton *button, gpointer data)
{
    GtkTreeIter it;
    gint id;
    sqlite3_stmt *q = NULL;
    int result;
    (void)button; (void)data;
    if (!GetSelectedRow(&it)) return;
    gtk_tree_model_get(GTK_TREE_MODEL(buffer2), &it, C_OTDEL_ID, &id, -1);

    result = sqlite3_prepare_v2(db,
        "SELECT COUNT(*) FROM Сотрудники WHERE [Код отдела]=?;",
        -1, &q, NULL);
    if (result != SQLITE_OK) {
        ShowDepartmentError(sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(q, 1, id);
    result = sqlite3_step(q);
    if (result != SQLITE_ROW) {
        ShowDepartmentError(sqlite3_errmsg(db));
        sqlite3_finalize(q);
        return;
    }
    int count = sqlite3_column_int(q, 0);
    sqlite3_finalize(q);
    if (count > 0) {
        ShowDepartmentError("Нельзя удалить отдел: к нему прикреплены сотрудники.");
        return;
    }

    q = NULL;
    result = sqlite3_prepare_v2(db,
        "DELETE FROM Отделы WHERE [Код отдела]=?;", -1, &q, NULL);
    if (result == SQLITE_OK) {
        sqlite3_bind_int(q, 1, id);
        result = sqlite3_step(q);
    }
    if (result == SQLITE_DONE)
        gtk_list_store_remove(buffer2, &it);
    else
        ShowDepartmentError(sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
}

static void on_button11_clicked(GtkButton *button, gpointer data)
{
    (void)button; (void)data;
    gtk_widget_destroy(window3);
}

static gboolean LoadDepartments(void)
{
    sqlite3_stmt *q = NULL;
    int result = sqlite3_prepare_v2(db,
        "SELECT [Код отдела], Отдел, Руководитель FROM Отделы ORDER BY [Код отдела];",
        -1, &q, NULL);
    if (result != SQLITE_OK) {
        ShowDepartmentError(sqlite3_errmsg(db));
        return FALSE;
    }
    gtk_list_store_clear(buffer2);
    while ((result = sqlite3_step(q)) == SQLITE_ROW) {
        GtkTreeIter it;
        gtk_list_store_append(buffer2, &it);
        gtk_list_store_set(buffer2, &it,
            C_OTDEL_ID, sqlite3_column_int(q, 0),
            C_OTDEL, sqlite3_column_text(q, 1),
            C_BOSS, sqlite3_column_text(q, 2), -1);
    }
    sqlite3_finalize(q);
    if (result != SQLITE_DONE) {
        ShowDepartmentError(sqlite3_errmsg(db));
        return FALSE;
    }
    return TRUE;
}

void ShowOtdeli(GtkWidget *win)
{
    GError *error = NULL;
    GtkWidget *add_button, *del_button, *close_button;
    (void)win;
    builder3 = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder3, "main.glade", &error)) {
        ShowDepartmentError(error ? error->message : "Не найден main.glade");
        if (error) g_error_free(error);
        return;
    }
    window3 = GTK_WIDGET(gtk_builder_get_object(builder3, "window3"));
    buffer2 = GTK_LIST_STORE(gtk_builder_get_object(builder3, "liststore2"));
    table2 = GTK_TREE_VIEW(gtk_builder_get_object(builder3, "treeview2"));
    add_button = GTK_WIDGET(gtk_builder_get_object(builder3, "button9"));
    del_button = GTK_WIDGET(gtk_builder_get_object(builder3, "button10"));
    close_button = GTK_WIDGET(gtk_builder_get_object(builder3, "button11"));
    if (!window3 || !buffer2 || !table2 ||
        !add_button || !del_button || !close_button) {
        ShowDepartmentError("Не найдены элементы интерфейса Glade");
        return;
    }

    /* Не изменять служебный код отдела. */
    GtkCellRenderer *id_cell = GTK_CELL_RENDERER(
        gtk_builder_get_object(builder3, "cellrenderertext4"));
    GtkCellRenderer *name_cell = GTK_CELL_RENDERER(
        gtk_builder_get_object(builder3, "cellrenderertext5"));
    GtkCellRenderer *boss_cell = GTK_CELL_RENDERER(
        gtk_builder_get_object(builder3, "cellrenderertext6"));
    g_object_set(id_cell, "editable", FALSE, NULL);
    g_object_set(name_cell, "editable", TRUE, NULL);
    g_object_set(boss_cell, "editable", TRUE, NULL);
    g_signal_connect(name_cell, "edited", G_CALLBACK(on_cellrenderertext5_edited), NULL);
    g_signal_connect(boss_cell, "edited", G_CALLBACK(on_cellrenderertext6_edited), NULL);
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_button9_clicked), NULL);
    g_signal_connect(del_button, "clicked", G_CALLBACK(on_button10_clicked), NULL);
    g_signal_connect(close_button, "clicked", G_CALLBACK(on_button11_clicked), NULL);
    gtk_window_set_default_size(GTK_WINDOW(window3), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window3), GTK_WIN_POS_CENTER);
    if (!LoadDepartments()) return;
    gtk_widget_show_all(window3);
}
