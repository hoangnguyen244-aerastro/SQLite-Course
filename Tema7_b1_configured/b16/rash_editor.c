/* Тема 7: редактор расходной накладной и ее строк.
   Дата и контрагенты сохраняются в Накладные, движение товаров атомарно.
   GTK+ 3, SQLite3, C (Code::Blocks). */
#include "main.h"
#include <stdio.h>
#include <time.h>

typedef struct {
    int doc_id;
    GtkWidget *dialog, *number, *date, *supplier, *buyer, *total;
    GtkWidget *goods, *quantity, *tree;
    GtkListStore *lines;
} RashUi;

static void warn_user(RashUi *u, const char *msg) {
    GtkWidget *d=gtk_message_dialog_new(GTK_WINDOW(u->dialog),GTK_DIALOG_MODAL,
             GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"%s",msg);
    gtk_dialog_run(GTK_DIALOG(d)); gtk_widget_destroy(d);
}
static int prepare(sqlite3_stmt **st,const char *sql){
    return sqlite3_prepare_v2(db,sql,-1,st,NULL)==SQLITE_OK;
}
static int sql_run(const char *sql){
    char *error=NULL;
    int ok=(sqlite3_exec(db,sql,NULL,NULL,&error)==SQLITE_OK);
    if(error)sqlite3_free(error);
    return ok;
}
static void totals(RashUi *u){
    sqlite3_stmt *st=NULL;
    if(prepare(&st,"SELECT COALESCE(SUM(s.Расход * COALESCE(t.[Цена розничная],0)),0) "
                   "FROM [Строки накладной] s JOIN Товары t "
                   "ON t.[Код товара]=s.[Код товара] "
                   "WHERE s.[Код накладной]=?")){
        sqlite3_bind_int(st,1,u->doc_id);
        if(sqlite3_step(st)==SQLITE_ROW){
            char text[80]; g_snprintf(text,sizeof(text),"%.2f",sqlite3_column_double(st,0));
            gtk_label_set_text(GTK_LABEL(u->total),text);
        }
    }
    sqlite3_finalize(st);
}
static void refresh_lines(RashUi *u){
    GtkTreeIter it;sqlite3_stmt *st=NULL;
    gtk_list_store_clear(u->lines);
    if(prepare(&st,"SELECT s.[Код строки накладной],t.Товар,s.Расход,"
                   "COALESCE(t.[Цена розничная],0) "
                   "FROM [Строки накладной] s JOIN Товары t "
                   "ON t.[Код товара]=s.[Код товара] "
                   "WHERE s.[Код накладной]=? ORDER BY s.[Код строки накладной]")){
        sqlite3_bind_int(st,1,u->doc_id);
        while(sqlite3_step(st)==SQLITE_ROW){
            const char *name=(const char*)sqlite3_column_text(st,1);
            gtk_list_store_append(u->lines,&it);
            gtk_list_store_set(u->lines,&it,0,sqlite3_column_int(st,0),
               1,name?name:"",2,sqlite3_column_int(st,2),
               3,sqlite3_column_double(st,3),-1);
        }
    }
    sqlite3_finalize(st);
    totals(u);
}
static int save_doc(RashUi *u){
    const char *dt=gtk_entry_get_text(GTK_ENTRY(u->date));
    int y,m,d;char extra;
    if(sscanf(dt,"%d-%d-%d%c",&y,&m,&d,&extra)!=3 ||
       y<1 || y>9999 || !g_date_valid_dmy((GDateDay)d,(GDateMonth)m,(GDateYear)y)){
        warn_user(u,"Дата должна быть действительной в формате YYYY-MM-DD");
        return 0;
    }
    sqlite3_stmt *st=NULL;
    if(!prepare(&st,"UPDATE Накладные SET Номер=?,Дата=?,Поставщик=?,Покупатель=? "
                    "WHERE [Код накладной]=? AND Тип=0")){
        warn_user(u,sqlite3_errmsg(db));return 0;
    }
    sqlite3_bind_int(st,1,gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(u->number)));
    sqlite3_bind_text(st,2,dt,-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(st,3,gtk_entry_get_text(GTK_ENTRY(u->supplier)),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(st,4,gtk_entry_get_text(GTK_ENTRY(u->buyer)),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(st,5,u->doc_id);
    int ok=sqlite3_step(st)==SQLITE_DONE;
    sqlite3_finalize(st);
    if(!ok)warn_user(u,sqlite3_errmsg(db));
    return ok;
}
static void on_save(GtkButton *button,gpointer user_data){
    (void)button;save_doc((RashUi*)user_data);
}
static void update_total_db(int id){
    sqlite3_stmt *st=NULL;
    if(prepare(&st,"UPDATE Накладные SET Сумма=("
                  "SELECT COALESCE(SUM(s.Расход*COALESCE(t.[Цена розничная],0)),0) "
                  "FROM [Строки накладной] s JOIN Товары t "
                  "ON t.[Код товара]=s.[Код товара] WHERE s.[Код накладной]=?) "
                  "WHERE [Код накладной]=?")){
        sqlite3_bind_int(st,1,id);sqlite3_bind_int(st,2,id);
        sqlite3_step(st);
    }
    sqlite3_finalize(st);
}
static void on_add(GtkButton *button,gpointer user_data){
    (void)button;
    RashUi *u=(RashUi*)user_data;
    const char *prod=gtk_combo_box_get_active_id(GTK_COMBO_BOX(u->goods));
    int qty=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(u->quantity));
    if(!prod || qty<=0){warn_user(u,"Выберите товар и положительное количество");return;}
    if(!save_doc(u))return;
    int id=atoi(prod);
    if(!sql_run("BEGIN IMMEDIATE")){warn_user(u,sqlite3_errmsg(db));return;}
    sqlite3_stmt *st=NULL;
    int ok=prepare(&st,"UPDATE Товары SET Количество=Количество-? "
                       "WHERE [Код товара]=? AND COALESCE(Количество,0)>=?");
    if(ok){
        sqlite3_bind_int(st,1,qty);sqlite3_bind_int(st,2,id);
        sqlite3_bind_int(st,3,qty);
        ok=(sqlite3_step(st)==SQLITE_DONE && sqlite3_changes(db)==1);
    }
    sqlite3_finalize(st);
    if(ok && prepare(&st,"INSERT INTO [Строки накладной] "
            "([Код товара],[Код накладной],Приход,Расход) VALUES (?,?,0,?)")){
        sqlite3_bind_int(st,1,id);sqlite3_bind_int(st,2,u->doc_id);
        sqlite3_bind_int(st,3,qty);
        ok=sqlite3_step(st)==SQLITE_DONE;
    }else ok=0;
    sqlite3_finalize(st);
    if(ok)update_total_db(u->doc_id);
    if(!sql_run(ok?"COMMIT":"ROLLBACK"))ok=0;
    if(!ok)warn_user(u,"Не удалось добавить товар. Проверьте остаток на складе.");
    refresh_lines(u);
}
static void on_remove(GtkButton *button,gpointer user_data){
    (void)button;RashUi *u=(RashUi*)user_data;
    GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(u->tree));
    GtkTreeIter iter;GtkTreeModel *model=NULL;
    if(!gtk_tree_selection_get_selected(sel,&model,&iter))return;
    int lineid=0;gtk_tree_model_get(model,&iter,0,&lineid,-1);
    if(!sql_run("BEGIN IMMEDIATE")){warn_user(u,sqlite3_errmsg(db));return;}
    sqlite3_stmt *st=NULL;
    int ok=prepare(&st,"UPDATE Товары SET Количество=COALESCE(Количество,0)+("
       "SELECT Расход FROM [Строки накладной] WHERE [Код строки накладной]=?) "
       "WHERE [Код товара]=(SELECT [Код товара] FROM [Строки накладной] "
       "WHERE [Код строки накладной]=?)");
    if(ok){
        sqlite3_bind_int(st,1,lineid);sqlite3_bind_int(st,2,lineid);
        ok=(sqlite3_step(st)==SQLITE_DONE && sqlite3_changes(db)==1);
    }
    sqlite3_finalize(st);
    if(ok && prepare(&st,"DELETE FROM [Строки накладной] "
        "WHERE [Код строки накладной]=? AND [Код накладной]=?")){
        sqlite3_bind_int(st,1,lineid);sqlite3_bind_int(st,2,u->doc_id);
        ok=(sqlite3_step(st)==SQLITE_DONE && sqlite3_changes(db)==1);
    }else ok=0;
    sqlite3_finalize(st);
    if(ok)update_total_db(u->doc_id);
    if(!sql_run(ok?"COMMIT":"ROLLBACK"))ok=0;
    if(!ok)warn_user(u,"Не удалось удалить строку");
    refresh_lines(u);
}
static void input_row(GtkWidget *grid,int row,const char *label,GtkWidget *widget){
    GtkWidget *lbl=gtk_label_new(label);
    gtk_widget_set_halign(lbl,GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid),lbl,0,row,1,1);
    gtk_grid_attach(GTK_GRID(grid),widget,1,row,1,1);
}
void OpenRashDocument(int id){
    RashUi u={0};u.doc_id=id;
    u.dialog=gtk_dialog_new_with_buttons("Расходная накладная",GTK_WINDOW(wRash),
                  GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                  "Закрыть",GTK_RESPONSE_CLOSE,NULL);
    gtk_window_set_default_size(GTK_WINDOW(u.dialog),680,570);
    GtkWidget *box=gtk_dialog_get_content_area(GTK_DIALOG(u.dialog));
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),6);
    gtk_grid_set_column_spacing(GTK_GRID(grid),12);
    gtk_container_set_border_width(GTK_CONTAINER(grid),12);
    gtk_box_pack_start(GTK_BOX(box),grid,FALSE,FALSE,4);
    u.number=gtk_spin_button_new_with_range(0,999999,1);
    u.date=gtk_entry_new();u.supplier=gtk_entry_new();u.buyer=gtk_entry_new();
    input_row(grid,0,"Номер",u.number);
    input_row(grid,1,"Дата (YYYY-MM-DD)",u.date);
    input_row(grid,2,"Поставщик",u.supplier);
    input_row(grid,3,"Покупатель",u.buyer);
    GtkWidget *save=gtk_button_new_with_label("Сохранить поля накладной");
    gtk_grid_attach(GTK_GRID(grid),save,0,4,2,1);
    g_signal_connect(save,"clicked",G_CALLBACK(on_save),&u);
    sqlite3_stmt *st=NULL;
    if(prepare(&st,"SELECT Номер,Дата,Поставщик,Покупатель FROM Накладные "
                   "WHERE [Код накладной]=? AND Тип=0")){
        sqlite3_bind_int(st,1,id);
        if(sqlite3_step(st)==SQLITE_ROW){
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(u.number),sqlite3_column_int(st,0));
            GtkWidget *fields[]={u.date,u.supplier,u.buyer};
            for(int k=0;k<3;k++){
                const char *v=(const char*)sqlite3_column_text(st,k+1);
                gtk_entry_set_text(GTK_ENTRY(fields[k]),v?v:"");
            }
        }
    }
    sqlite3_finalize(st);
    u.lines=gtk_list_store_new(4,G_TYPE_INT,G_TYPE_STRING,G_TYPE_INT,G_TYPE_DOUBLE);
    u.tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(u.lines));
    const char *headers[]={"Код","Товар","Расход","Цена"};
    for(int k=0;k<4;k++){
        GtkCellRenderer *render=gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col=gtk_tree_view_column_new_with_attributes(
            headers[k],render,k==3?"text":"text",k,-1);
        /* Double is formatted by the renderer using default conversion. */
        gtk_tree_view_append_column(GTK_TREE_VIEW(u.tree),col);
    }
    GtkWidget *sc=gtk_scrolled_window_new(NULL,NULL);
    gtk_widget_set_size_request(sc,-1,230);
    gtk_container_add(GTK_CONTAINER(sc),u.tree);
    gtk_box_pack_start(GTK_BOX(box),sc,TRUE,TRUE,4);
    GtkWidget *edit=gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(edit),8);
    gtk_grid_set_row_spacing(GTK_GRID(edit),8);
    gtk_container_set_border_width(GTK_CONTAINER(edit),10);
    gtk_box_pack_start(GTK_BOX(box),edit,FALSE,FALSE,3);
    u.goods=gtk_combo_box_text_new();
    u.quantity=gtk_spin_button_new_with_range(1,1000000,1);
    input_row(edit,0,"Товар",u.goods);
    input_row(edit,1,"Расход (шт.)",u.quantity);
    if(prepare(&st,"SELECT [Код товара],Товар,COALESCE(Количество,0) FROM Товары ORDER BY Товар")){
        int first=1;
        while(sqlite3_step(st)==SQLITE_ROW){
            gchar key[40],label[512];
            g_snprintf(key,sizeof(key),"%d",sqlite3_column_int(st,0));
            const char *name=(const char*)sqlite3_column_text(st,1);
            g_snprintf(label,sizeof(label),"%s (остаток: %d)",
                       name?name:"",sqlite3_column_int(st,2));
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(u.goods),key,label);
            if(first){gtk_combo_box_set_active(GTK_COMBO_BOX(u.goods),0);first=0;}
        }
    }
    sqlite3_finalize(st);
    GtkWidget *add=gtk_button_new_with_label("Добавить строку");
    GtkWidget *remove=gtk_button_new_with_label("Удалить строку");
    gtk_grid_attach(GTK_GRID(edit),add,0,2,1,1);
    gtk_grid_attach(GTK_GRID(edit),remove,1,2,1,1);
    g_signal_connect(add,"clicked",G_CALLBACK(on_add),&u);
    g_signal_connect(remove,"clicked",G_CALLBACK(on_remove),&u);
    u.total=gtk_label_new("0.00");
    input_row(edit,3,"Сумма накладной",u.total);
    refresh_lines(&u);
    gtk_widget_show_all(u.dialog);
    gtk_dialog_run(GTK_DIALOG(u.dialog));
    gtk_widget_destroy(u.dialog);
    g_object_unref(u.lines);
}
int DeleteRashDocument(int id){
    if(!sql_run("BEGIN IMMEDIATE"))return 0;
    sqlite3_stmt *st=NULL;
    int ok=prepare(&st,"UPDATE Товары SET Количество=COALESCE(Количество,0)+"
            "COALESCE((SELECT SUM(Расход) FROM [Строки накладной] "
            "WHERE [Код товара]=Товары.[Код товара] AND [Код накладной]=?),0) "
            "WHERE [Код товара] IN (SELECT [Код товара] FROM [Строки накладной] WHERE [Код накладной]=?)");
    if(ok){
        sqlite3_bind_int(st,1,id);sqlite3_bind_int(st,2,id);
        ok=sqlite3_step(st)==SQLITE_DONE;
    }sqlite3_finalize(st);
    if(ok && prepare(&st,"DELETE FROM [Строки накладной] WHERE [Код накладной]=?")){
        sqlite3_bind_int(st,1,id);ok=sqlite3_step(st)==SQLITE_DONE;
    }else ok=0;
    sqlite3_finalize(st);
    if(ok && prepare(&st,"DELETE FROM Накладные WHERE [Код накладной]=? AND Тип=0")){
        sqlite3_bind_int(st,1,id);
        ok=(sqlite3_step(st)==SQLITE_DONE && sqlite3_changes(db)==1);
    }else ok=0;
    sqlite3_finalize(st);
    if(!sql_run(ok?"COMMIT":"ROLLBACK"))ok=0;
    return ok;
}
