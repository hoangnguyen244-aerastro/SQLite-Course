/* Редактирование дополнительных полей таблицы Товары.
   Подключается к форме wTovar без изменения исходного Glade. */
#include "main.h"

void OpenExtraTovar(int id,GtkWindow *parent){
    sqlite3_stmt *st=NULL;
    GtkWidget *dlg=gtk_dialog_new_with_buttons("Дополнительные поля товара",
        parent,GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
        "Сохранить",GTK_RESPONSE_OK,"Отмена",GTK_RESPONSE_CANCEL,NULL);
    gtk_window_set_default_size(GTK_WINDOW(dlg),460,290);
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),10);
    gtk_grid_set_column_spacing(GTK_GRID(grid),10);
    gtk_container_set_border_width(GTK_CONTAINER(grid),14);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg))),grid,TRUE,TRUE,0);
    const char *labels[]={"Цена закупки","Цена розничная","Место хранения","Стеллаж","Полка"};
    GtkWidget *fields[5];
    for(int i=0;i<5;i++){
        GtkWidget *label=gtk_label_new(labels[i]);
        gtk_widget_set_halign(label,GTK_ALIGN_START);
        fields[i]=i<2?gtk_spin_button_new_with_range(0,100000000,0.01):gtk_entry_new();
        if(i<2)gtk_spin_button_set_digits(GTK_SPIN_BUTTON(fields[i]),2);
        gtk_grid_attach(GTK_GRID(grid),label,0,i,1,1);
        gtk_grid_attach(GTK_GRID(grid),fields[i],1,i,1,1);
    }
    const char *sql="SELECT [Цена закупки],[Цена розничная],"
        "[Место хранения],Стеллаж,Полка FROM Товары WHERE [Код товара]=?";
    if(sqlite3_prepare_v2(db,sql,-1,&st,NULL)==SQLITE_OK){
        sqlite3_bind_int(st,1,id);
        if(sqlite3_step(st)==SQLITE_ROW){
            for(int i=0;i<5;i++){
                if(i<2)gtk_spin_button_set_value(GTK_SPIN_BUTTON(fields[i]),
                   sqlite3_column_double(st,i));
                else {
                   const char *txt=(const char*)sqlite3_column_text(st,i);
                   gtk_entry_set_text(GTK_ENTRY(fields[i]),txt?txt:"");
                }
            }
        }
    }
    sqlite3_finalize(st);
    gtk_widget_show_all(dlg);
    if(gtk_dialog_run(GTK_DIALOG(dlg))==GTK_RESPONSE_OK){
        const char *upd="UPDATE Товары SET [Цена закупки]=?,[Цена розничная]=?,"
           "[Место хранения]=?,Стеллаж=?,Полка=? WHERE [Код товара]=?";
        if(sqlite3_prepare_v2(db,upd,-1,&st,NULL)==SQLITE_OK){
            sqlite3_bind_double(st,1,gtk_spin_button_get_value(GTK_SPIN_BUTTON(fields[0])));
            sqlite3_bind_double(st,2,gtk_spin_button_get_value(GTK_SPIN_BUTTON(fields[1])));
            for(int i=2;i<5;i++)sqlite3_bind_text(st,i+1,
                  gtk_entry_get_text(GTK_ENTRY(fields[i])),-1,SQLITE_TRANSIENT);
            sqlite3_bind_int(st,6,id);
            if(sqlite3_step(st)!=SQLITE_DONE){
                GtkWidget *msg=gtk_message_dialog_new(GTK_WINDOW(dlg),GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"%s",sqlite3_errmsg(db));
                gtk_dialog_run(GTK_DIALOG(msg));gtk_widget_destroy(msg);
            }
        }
        sqlite3_finalize(st);
    }
    gtk_widget_destroy(dlg);
}
