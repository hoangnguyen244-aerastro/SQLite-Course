/* Тема 2, Задание 2: Первичный ключ в SQLite.
   Интерфейс Отделы не показывает служебное поле Код отдела. */
#include "main.h"
sqlite3 *db = NULL;
sqlite3_stmt *stmt = NULL;
char *err = NULL;
int rc = 0;
GtkWidget *window1 = NULL, *window2 = NULL, *window3 = NULL;
GtkBuilder *builder1 = NULL, *builder2 = NULL, *builder3 = NULL;
char str[1000];
int lenstr = 0;

void ShowError(void) {
    GtkWidget *d = gtk_message_dialog_new(window1 ? GTK_WINDOW(window1) : NULL,
         GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", str);
    gtk_dialog_run(GTK_DIALOG(d)); gtk_widget_destroy(d);
}
int ShowSQL(void) { ShowError(); return 0; }
void StartDB(void) {
    if (sqlite3_open("b7.db", &db) != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка открытия БД: %s", sqlite3_errmsg(db));
        return;
    }
    /* Обеспечиваем воспроизводимый запуск, если b7.db еще нет. */
    const char *init =
      "CREATE TABLE IF NOT EXISTS Отделы ([Код отдела] INTEGER PRIMARY KEY, Отдел TEXT, Руководитель TEXT);"
      "INSERT INTO Отделы (Отдел,Руководитель) SELECT 'ИТ','Иванов' WHERE NOT EXISTS (SELECT 1 FROM Отделы);"
      "CREATE TABLE IF NOT EXISTS Сотрудники ([Код сотрудника] INTEGER PRIMARY KEY, Фамилия TEXT, Оклад INTEGER);"
      "INSERT INTO Сотрудники (Фамилия,Оклад) SELECT 'Иванов',20000 WHERE NOT EXISTS (SELECT 1 FROM Сотрудники);";
    char *e = NULL;
    if (sqlite3_exec(db, init, NULL, NULL, &e) != SQLITE_OK) {
        g_snprintf(str, sizeof(str), "Ошибка инициализации: %s", e ? e : "");
        sqlite3_free(e);
        ShowError();
    }
}
void EndDB(void) { if (db) sqlite3_close(db); db = NULL; }
static void click_departments(GtkButton *b, gpointer d) { (void)b;(void)d; ShowOtdeli(window1); }
static void click_staff(GtkButton *b, gpointer d) { (void)b;(void)d; ShowSotrudniki(window1); }
static gboolean ShowMainWindow(void) {
    GError *error = NULL;
    builder1 = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder1, "main.glade", &error)) {
        g_snprintf(str, sizeof(str), "Ошибка Glade: %s", error ? error->message : "");
        if (error) g_error_free(error);
        return FALSE;
    }
    window1 = GTK_WIDGET(gtk_builder_get_object(builder1, "window1"));
    GtkWidget *departments = GTK_WIDGET(gtk_builder_get_object(builder1, "button1"));
    GtkWidget *staff = GTK_WIDGET(gtk_builder_get_object(builder1, "button2"));
    GtkWidget *exit_btn = GTK_WIDGET(gtk_builder_get_object(builder1, "button3"));
    if (!window1 || !departments || !staff || !exit_btn) return FALSE;
    gtk_window_set_default_size(GTK_WINDOW(window1), 500, 300);
    g_signal_connect(departments, "clicked", G_CALLBACK(click_departments), NULL);
    g_signal_connect(staff, "clicked", G_CALLBACK(click_staff), NULL);
    g_signal_connect_swapped(exit_btn, "clicked", G_CALLBACK(gtk_widget_destroy), window1);
    g_signal_connect(window1, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window1);
    return TRUE;
}
int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    StartDB();
    if (!ShowMainWindow()) {
        ShowError(); EndDB(); return 1;
    }
    gtk_main(); EndDB(); return 0;
}
