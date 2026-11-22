/* Tema 4, tasks 1–5: department list, edit form and printing. */
#include "main.h"

static GtkListStore *buffer2 = NULL;
static GtkTreeView *table2 = NULL;

static void department_sql_error(const char *context)
{
    g_snprintf(str, sizeof(str), "%s: %s", context, sqlite3_errmsg(db));
    ShowError();
}

static gboolean current_department(int *id, GtkTreeIter *iter)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    if (!table2) return FALSE;
    selection = gtk_tree_view_get_selection(table2);
    if (!gtk_tree_selection_get_selected(selection, &model, iter)) return FALSE;
    gtk_tree_model_get(model, iter, 0, id, -1);
    return TRUE;
}

int LoadOtdeli(void)
{
    sqlite3_stmt *q = NULL;
    GtkTreeIter iter;
    int result = sqlite3_prepare_v2(db,
        "SELECT [Код отдела], Отдел, Руководитель FROM Отделы ORDER BY [Код отдела]",
        -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    gtk_list_store_clear(buffer2);
    while ((result = sqlite3_step(q)) == SQLITE_ROW) {
        gtk_list_store_append(buffer2, &iter);
        gtk_list_store_set(buffer2, &iter,
            0, sqlite3_column_int(q, 0),
            1, sqlite3_column_text(q, 1),
            2, sqlite3_column_text(q, 2), -1);
    }
    if (result != SQLITE_DONE) goto fail;
    sqlite3_finalize(q);
    return 0;
fail:
    g_snprintf(str, sizeof(str), "Ошибка чтения списка отделов: %s", sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
    return 1;
}

/* Update only the matching row, without depending on the current selection. */
int Reload1(int otdel)
{
    sqlite3_stmt *q = NULL;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean valid, found = FALSE;
    int result;

    if (!buffer2 || !table2) return 1;
    result = sqlite3_prepare_v2(db,
        "SELECT [Код отдела], Отдел, Руководитель FROM Отделы WHERE [Код отдела]=?",
        -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    sqlite3_bind_int(q, 1, otdel);
    result = sqlite3_step(q);
    if (result != SQLITE_ROW) goto fail;

    model = GTK_TREE_MODEL(buffer2);
    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gint current_id;
        gtk_tree_model_get(model, &iter, 0, &current_id, -1);
        if (current_id == otdel) { found = TRUE; break; }
        valid = gtk_tree_model_iter_next(model, &iter);
    }
    if (!found) gtk_list_store_append(buffer2, &iter);
    gtk_list_store_set(buffer2, &iter,
        0, sqlite3_column_int(q, 0),
        1, sqlite3_column_text(q, 1),
        2, sqlite3_column_text(q, 2), -1);
    sqlite3_finalize(q);
    return 0;
fail:
    g_snprintf(str, sizeof(str), "Ошибка обновления отдела: %s", sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
    return 1;
}

G_MODULE_EXPORT void on_button8_clicked(GtkButton *button, gpointer user_data)
{
    GtkTreeIter iter;
    int id;
    (void)button; (void)user_data;
    if (current_department(&id, &iter)) ShowOtdel(id);
}

/* Add a draft record, then open the same form to name it. */
G_MODULE_EXPORT void on_button9_clicked(GtkButton *button, gpointer user_data)
{
    sqlite3_stmt *q = NULL;
    int result;
    (void)button; (void)user_data;
    result = sqlite3_prepare_v2(db,
        "INSERT INTO Отделы(Отдел, Руководитель) VALUES(?, ?)", -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    sqlite3_bind_text(q, 1, "Новый отдел", -1, SQLITE_STATIC);
    sqlite3_bind_text(q, 2, "", -1, SQLITE_STATIC);
    result = sqlite3_step(q);
    if (result != SQLITE_DONE) goto fail;
    sqlite3_finalize(q);
    q = NULL;
    int id = (int)sqlite3_last_insert_rowid(db);
    if (Reload1(id)) ShowError();
    ShowOtdel(id);
    return;
fail:
    department_sql_error("Ошибка добавления");
    if (q) sqlite3_finalize(q);
}

G_MODULE_EXPORT void on_button10_clicked(GtkButton *button, gpointer user_data)
{
    GtkTreeIter iter;
    int id, count, result;
    sqlite3_stmt *q = NULL;
    (void)button; (void)user_data;
    if (!current_department(&id, &iter)) return;
    result = sqlite3_prepare_v2(db,
        "SELECT COUNT(*) FROM Сотрудники WHERE [Код отдела]=?", -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    sqlite3_bind_int(q, 1, id);
    result = sqlite3_step(q);
    if (result != SQLITE_ROW) goto fail;
    count = sqlite3_column_int(q, 0);
    sqlite3_finalize(q);
    q = NULL;
    if (count > 0) {
        g_snprintf(str, sizeof(str), "Нельзя удалить отдел: в нём работают сотрудники (%d).", count);
        ShowError();
        return;
    }
    result = sqlite3_prepare_v2(db,
        "DELETE FROM Отделы WHERE [Код отдела]=?", -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    sqlite3_bind_int(q, 1, id);
    result = sqlite3_step(q);
    if (result != SQLITE_DONE) goto fail;
    sqlite3_finalize(q);
    gtk_list_store_remove(buffer2, &iter);
    return;
fail:
    department_sql_error("Ошибка удаления");
    if (q) sqlite3_finalize(q);
}

G_MODULE_EXPORT void on_button19_clicked(GtkButton *button, gpointer user_data)
{
    (void)button; (void)user_data;
    PrintAllDepartments(GTK_WINDOW(window3));
}

G_MODULE_EXPORT void on_button11_clicked(GtkButton *button, gpointer user_data)
{
    (void)button; (void)user_data;
    gtk_widget_destroy(window3);
}

void ShowOtdeli(GtkWidget *win)
{
    GError *error = NULL;
    (void)win;
    builder3 = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder3, "main.glade", &error)) {
        g_snprintf(str, sizeof(str), "Ошибка загрузки main.glade: %s",
                   error ? error->message : "неизвестная ошибка");
        if (error) g_error_free(error);
        ShowError();
        return;
    }
    window3 = GTK_WIDGET(gtk_builder_get_object(builder3, "window3"));
    buffer2 = GTK_LIST_STORE(gtk_builder_get_object(builder3, "liststore2"));
    table2 = GTK_TREE_VIEW(gtk_builder_get_object(builder3, "treeview2"));
    if (!window3 || !buffer2 || !table2) {
        g_snprintf(str, sizeof(str), "Не найдены элементы списка Отделы в Glade.");
        ShowError();
        return;
    }
    gtk_window_set_default_size(GTK_WINDOW(window3), 700, 300);
    gtk_window_set_position(GTK_WINDOW(window3), GTK_WIN_POS_CENTER);
    gtk_builder_connect_signals(builder3, NULL);
    if (LoadOtdeli()) ShowError();
    gtk_widget_show_all(window3);
}
