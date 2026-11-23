/* Tema 4, task 1: data-entry form for an individual department. */
#include "main.h"

static GtkWidget *w_otdel = NULL;
static GtkBuilder *b_otdel = NULL;
static GtkEntry *gOtdel = NULL;
GtkComboBoxText *gBossCombo = NULL;
static char BossName[1000] = "";
static GtkImage *gDeptImage = NULL;
static GtkCalendar *gDeptCalendar = NULL;
static gboolean gDateDefined = FALSE;
static gchar *CurFoto = NULL;
static int CurOtdel = 0;

int LoadRecord1(int otdel)
{
    sqlite3_stmt *q = NULL;
    int result;

    result = sqlite3_prepare_v2(db,
        "SELECT Отдел, Руководитель, Фото, [Дата основания] FROM Отделы WHERE [Код отдела]=?",
        -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    sqlite3_bind_int(q, 1, otdel);
    result = sqlite3_step(q);
    if (result != SQLITE_ROW) goto fail;

    const unsigned char *department = sqlite3_column_text(q, 0);
    const unsigned char *manager = sqlite3_column_text(q, 1);
    gtk_entry_set_text(gOtdel, department ? (const char *)department : "");
    g_strlcpy(BossName, manager ? (const char *)manager : "", sizeof(BossName));
    g_free(CurFoto);
    const unsigned char *photo = sqlite3_column_text(q, 2);
    CurFoto = g_strdup(photo ? (const char *)photo : "");
    if (CurFoto[0] && g_file_test(CurFoto, G_FILE_TEST_IS_REGULAR))
        gtk_image_set_from_file(gDeptImage, CurFoto);
    else
        gtk_image_clear(gDeptImage);
    const unsigned char *date = sqlite3_column_text(q, 3);
    int year = 0, month = 0, day = 0;
    gDateDefined = date && sscanf((const char *)date, "%d-%d-%d", &year, &month, &day) == 3
        && year >= 1 && month >= 1 && month <= 12 && day >= 1 && day <= 31;
    if (gDateDefined) {
        gtk_calendar_select_month(gDeptCalendar, (guint)(month - 1), (guint)year);
        gtk_calendar_select_day(gDeptCalendar, (guint)day);
    }
    sqlite3_finalize(q);
    return 0;

fail:
    g_snprintf(str, sizeof(str), "Ошибка загрузки отдела: %s",
               result == SQLITE_DONE ? "Запись не найдена" : sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
    return 1;
}

/* Tema 4, zadanie 2: show employee surnames rather than free-text boss input.
   Search by text (not row number): employee IDs need not be consecutive. */
int LoadCombobox1(void)
{
    sqlite3_stmt *q = NULL;
    int result, index = 0, selected = -1;
    const char *sql = "SELECT [Код сотрудника], Фамилия FROM Сотрудники "
                      "WHERE Фамилия IS NOT NULL AND TRIM(Фамилия) <> '' "
                      "ORDER BY [Код сотрудника]";
    if (!gBossCombo) return 1;
    gtk_combo_box_text_remove_all(gBossCombo);
    result = sqlite3_prepare_v2(db, sql, -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    while ((result = sqlite3_step(q)) == SQLITE_ROW) {
        const unsigned char *surname = sqlite3_column_text(q, 1);
        if (!surname) continue;
        gtk_combo_box_text_append_text(gBossCombo, (const char *)surname);
        if (selected < 0 && g_strcmp0((const char *)surname, BossName) == 0)
            selected = index;
        index++;
    }
    if (result != SQLITE_DONE) goto fail;
    sqlite3_finalize(q);
    /* An outdated surname is not an employee: leave selection empty.
       SaveRecord1() requires the user to select a current employee. */
    gtk_combo_box_set_active(GTK_COMBO_BOX(gBossCombo), selected);
    return 0;
fail:
    g_snprintf(str, sizeof(str), "Ошибка загрузки списка руководителей: %s", sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
    return 1;
}

int SaveRecord1(void)
{
    sqlite3_stmt *q = NULL;
    int result;
    gchar *department = g_strdup(gtk_entry_get_text(gOtdel));
    gchar *manager = gtk_combo_box_text_get_active_text(gBossCombo);
    if (!manager) manager = g_strdup("");
    g_strstrip(department);
    g_strstrip(manager);

    if (!manager[0]) {
        g_snprintf(str, sizeof(str), "Выберите руководителя из списка.");
        g_free(department);
        g_free(manager);
        return 1;
    }
    if (!department[0]) {
        g_snprintf(str, sizeof(str), "Введите наименование отдела.");
        g_free(department);
        g_free(manager);
        return 1;
    }
    result = sqlite3_prepare_v2(db,
        "UPDATE Отделы SET Отдел=?, Руководитель=?, Фото=?, [Дата основания]=? WHERE [Код отдела]=?",
        -1, &q, NULL);
    if (result != SQLITE_OK) goto fail;
    sqlite3_bind_text(q, 1, department, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(q, 2, manager, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(q, 3, CurFoto ? CurFoto : "", -1, SQLITE_TRANSIENT);
    if (gDateDefined) {
        guint year, month, day;
        char date[16];
        gtk_calendar_get_date(gDeptCalendar, &year, &month, &day);
        g_snprintf(date, sizeof(date), "%04u-%02u-%02u", year, month + 1, day);
        sqlite3_bind_text(q, 4, date, -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(q, 4);
    }
    sqlite3_bind_int(q, 5, CurOtdel);
    result = sqlite3_step(q);
    if (result != SQLITE_DONE) goto fail;
    sqlite3_finalize(q);
    g_free(department);
    g_free(manager);
    return 0;

fail:
    g_snprintf(str, sizeof(str), "Ошибка сохранения: %s", sqlite3_errmsg(db));
    if (q) sqlite3_finalize(q);
    g_free(department);
    g_free(manager);
    return 1;
}

G_MODULE_EXPORT void on_button13_clicked(GtkButton *button, gpointer user_data)
{
    (void)button; (void)user_data;
    if (SaveRecord1()) {
        ShowError();
        return; /* Keep the form open when validation or SQLite fails. */
    }
    Reload1(CurOtdel);
    gtk_widget_destroy(w_otdel);
}

G_MODULE_EXPORT void on_button14_clicked(GtkButton *button, gpointer user_data)
{
    (void)button; (void)user_data;
    gtk_widget_destroy(w_otdel); /* Cancel without saving form changes. */
}

/* Same path-based storage model as the instructor's b12 sample. */
G_MODULE_EXPORT void on_button15_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    (void)button; (void)user_data;
    dialog = gtk_file_chooser_dialog_new("Выбрать изображение", GTK_WINDOW(w_otdel),
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Отмена", GTK_RESPONSE_CANCEL,
        "_Открыть", GTK_RESPONSE_ACCEPT, NULL);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Изображения PNG, JPEG, BMP");
    gtk_file_filter_add_mime_type(filter, "image/png");
    gtk_file_filter_add_mime_type(filter, "image/jpeg");
    gtk_file_filter_add_mime_type(filter, "image/bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *chosen = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (chosen && g_file_test(chosen, G_FILE_TEST_IS_REGULAR)) {
            g_free(CurFoto);
            CurFoto = g_strdup(chosen);
            gtk_image_set_from_file(gDeptImage, CurFoto);
        }
        g_free(chosen);
    }
    gtk_widget_destroy(dialog);
}

G_MODULE_EXPORT void on_button16_clicked(GtkButton *button, gpointer user_data)
{
    (void)button; (void)user_data;
    g_free(CurFoto);
    CurFoto = g_strdup("");
    gtk_image_clear(gDeptImage);
}

G_MODULE_EXPORT void on_calendar_otdel_day_selected(GtkCalendar *calendar, gpointer user_data)
{
    (void)calendar; (void)user_data;
    gDateDefined = TRUE;
}

/* Explicit opt-out allows the NULL date to survive editing and reopening. */
G_MODULE_EXPORT void on_button17_clicked(GtkButton *button, gpointer user_data)
{
    (void)button; (void)user_data;
    gDateDefined = FALSE;
}

void ShowOtdel(int otdel)
{
    GError *error = NULL;
    CurOtdel = otdel;
    b_otdel = gtk_builder_new();
    if (!gtk_builder_add_from_file(b_otdel, "main.glade", &error)) {
        g_snprintf(str, sizeof(str), "Не удалось открыть main.glade: %s",
                   error ? error->message : "неизвестная ошибка");
        if (error) g_error_free(error);
        ShowError();
        g_object_unref(b_otdel);
        b_otdel = NULL;
        return;
    }
    w_otdel = GTK_WIDGET(gtk_builder_get_object(b_otdel, "w_otdel"));
    gOtdel = GTK_ENTRY(gtk_builder_get_object(b_otdel, "entry_otdel"));
    gBossCombo = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(b_otdel, "comboboxtext2"));
    gDeptImage = GTK_IMAGE(gtk_builder_get_object(b_otdel, "image_otdel"));
    gDeptCalendar = GTK_CALENDAR(gtk_builder_get_object(b_otdel, "calendar_otdel"));
    if (!w_otdel || !gOtdel || !gBossCombo || !gDeptImage || !gDeptCalendar) {
        g_snprintf(str, sizeof(str), "Не найдены виджеты формы w_otdel.");
        ShowError();
        g_object_unref(b_otdel);
        b_otdel = NULL;
        return;
    }
    gtk_window_set_title(GTK_WINDOW(w_otdel), "Ввод отдела");
    gtk_window_set_default_size(GTK_WINDOW(w_otdel), 730, 510);
    gtk_window_set_position(GTK_WINDOW(w_otdel), GTK_WIN_POS_CENTER);
    if (window3) gtk_window_set_transient_for(GTK_WINDOW(w_otdel), GTK_WINDOW(window3));
    gtk_window_set_modal(GTK_WINDOW(w_otdel), TRUE);
    gtk_builder_connect_signals(b_otdel, NULL);
    if (LoadRecord1(otdel)) {
        ShowError();
        gtk_widget_destroy(w_otdel);
        g_object_unref(b_otdel);
        b_otdel = NULL;
        return;
    }
    if (LoadCombobox1()) {
        ShowError();
        gtk_widget_destroy(w_otdel);
        g_object_unref(b_otdel);
        b_otdel = NULL;
        return;
    }
    gtk_widget_show_all(w_otdel);
    /* Builder is retained for the lifetime of this form. */
}
