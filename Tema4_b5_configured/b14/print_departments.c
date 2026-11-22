/* Tema 4, task 5: print department card and paginated department listing.
   Data are read from SQLite; GTK print dialog may print to a PDF printer. */
#include "main.h"
#include <pango/pangocairo.h>

typedef struct {
    gboolean all;
    gint selected_id;
    gint pages;
} DepartmentPrintJob;

static void line(GtkPrintContext *ctx, gdouble x, gdouble y,
                 const gchar *label, const gchar *value, gboolean heading)
{
    cairo_t *cr = gtk_print_context_get_cairo_context(ctx);
    PangoLayout *layout = gtk_print_context_create_pango_layout(ctx);
    PangoFontDescription *font = pango_font_description_from_string(
        heading ? "Sans Bold 17" : "Sans 12");
    gchar *txt = label ? g_strdup_printf("%s: %s", label, value ? value : "") :
                         g_strdup(value ? value : "");
    pango_layout_set_font_description(layout, font);
    pango_layout_set_width(layout, 470 * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_text(layout, txt, -1);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);
    g_free(txt);
    pango_font_description_free(font);
    g_object_unref(layout);
}

static void begin_print(GtkPrintOperation *op, GtkPrintContext *ctx, gpointer data)
{
    DepartmentPrintJob *job = (DepartmentPrintJob *)data;
    sqlite3_stmt *q = NULL;
    int n = 0;
    (void)ctx;
    if (!job->all) {
        gtk_print_operation_set_n_pages(op, 1);
        return;
    }
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Отделы", -1, &q, NULL) == SQLITE_OK) {
        if (sqlite3_step(q) == SQLITE_ROW) n = sqlite3_column_int(q, 0);
    }
    sqlite3_finalize(q);
    job->pages = n;
    /* At least one page avoids a GTK error for an empty list. */
    gtk_print_operation_set_n_pages(op, n > 0 ? n : 1);
}

static void draw_page(GtkPrintOperation *op, GtkPrintContext *ctx,
                      gint page_nr, gpointer data)
{
    DepartmentPrintJob *job = (DepartmentPrintJob *)data;
    sqlite3_stmt *q = NULL;
    const gchar *sql_single = "SELECT Отдел, Руководитель, Фото, [Дата основания] "
                              "FROM Отделы WHERE [Код отдела]=?";
    const gchar *sql_list = "SELECT Отдел, Руководитель, Фото, [Дата основания] "
                            "FROM Отделы ORDER BY [Код отдела] LIMIT 1 OFFSET ?";
    gint result;
    gchar *name, *boss, *photo, *date, *title;
    (void)op;
    line(ctx, 38, 35, NULL, job->all ? "Список отделов" : "Карточка отдела", TRUE);
    if (job->all && job->pages == 0) {
        line(ctx, 38, 85, NULL, "Нет записей", FALSE);
        return;
    }
    result = sqlite3_prepare_v2(db, job->all ? sql_list : sql_single, -1, &q, NULL);
    if (result != SQLITE_OK) {
        line(ctx, 38, 85, NULL, "Ошибка подготовки SQL", FALSE);
        return;
    }
    sqlite3_bind_int(q, 1, job->all ? page_nr : job->selected_id);
    result = sqlite3_step(q);
    if (result != SQLITE_ROW) {
        line(ctx, 38, 85, NULL, "Запись не найдена", FALSE);
        sqlite3_finalize(q);
        return;
    }
    name = g_strdup((const gchar *)sqlite3_column_text(q, 0));
    boss = g_strdup((const gchar *)sqlite3_column_text(q, 1));
    photo = g_strdup((const gchar *)sqlite3_column_text(q, 2));
    date = g_strdup((const gchar *)sqlite3_column_text(q, 3));
    sqlite3_finalize(q);
    title = g_strdup_printf("%d", page_nr + 1);
    if (job->all) line(ctx, 38, 75, "Страница", title, FALSE);
    line(ctx, 38, 125, "Отдел", name ? name : "(не указано)", FALSE);
    line(ctx, 38, 165, "Руководитель", boss ? boss : "(не указан)", FALSE);
    line(ctx, 38, 205, "Дата основания", date && *date ? date : "(не указана)", FALSE);
    if (photo && *photo && g_file_test(photo, G_FILE_TEST_IS_REGULAR)) {
        GError *error = NULL;
        GdkPixbuf *pb = gdk_pixbuf_new_from_file(photo, &error);
        if (pb) {
            gint w = gdk_pixbuf_get_width(pb), h = gdk_pixbuf_get_height(pb);
            double factor = MIN(230.0 / MAX(1,w), 230.0 / MAX(1,h));
            GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pb, MAX(1,(gint)(w*factor)),
                MAX(1,(gint)(h*factor)), GDK_INTERP_BILINEAR);
            if (scaled) {
                cairo_t *cr = gtk_print_context_get_cairo_context(ctx);
                gdk_cairo_set_source_pixbuf(cr, scaled, 38, 255);
                cairo_paint(cr);
                g_object_unref(scaled);
            }
            g_object_unref(pb);
        }
        if (error) g_error_free(error);
    }
    g_free(title); g_free(name); g_free(boss); g_free(photo); g_free(date);
}

static void print_job(gboolean all, gint id, GtkWindow *parent)
{
    GtkPrintOperation *op = gtk_print_operation_new();
    DepartmentPrintJob job = {all, id, 0};
    GError *error = NULL;
    gtk_print_operation_set_unit(op, GTK_UNIT_POINTS);
    gtk_print_operation_set_use_full_page(op, FALSE);
    gtk_print_operation_set_n_pages(op, 1);
    gtk_print_operation_set_job_name(op, all ? "Список отделов" : "Карточка отдела");
    g_signal_connect(op, "begin-print", G_CALLBACK(begin_print), &job);
    g_signal_connect(op, "draw-page", G_CALLBACK(draw_page), &job);
    gtk_print_operation_run(op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, parent, &error);
    if (error) {
        GtkWidget *dlg = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", error->message);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        g_error_free(error);
    }
    g_object_unref(op);
}

void PrintCurrentDepartment(int department_id, GtkWindow *parent)
{
    print_job(FALSE, department_id, parent);
}
void PrintAllDepartments(GtkWindow *parent)
{
    print_job(TRUE, 0, parent);
}
