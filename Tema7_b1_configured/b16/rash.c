#include "main.h"
#include <time.h>
/* Таблица расходных накладных и доступ к форме документа. */
static GtkListStore *buffer3;
static GtkTreeView *table3;
enum {C_ID_NAKL,C_NOMER,C_DATE};
static void rash_error(void){
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(wRash),GTK_DIALOG_MODAL,
       GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"%s",sqlite3_errmsg(db));
  gtk_dialog_run(GTK_DIALOG(dlg));gtk_widget_destroy(dlg);
}
static void refresh_rash(void){
  sqlite3_stmt *st=NULL;
  gtk_list_store_clear(buffer3);
  if(sqlite3_prepare_v2(db,"SELECT [Код накладной],Номер,Дата "
       "FROM [Расходные накладные] ORDER BY [Код накладной]",-1,&st,NULL)!=SQLITE_OK){
      rash_error();return;
  }
  GtkTreeIter it;
  while(sqlite3_step(st)==SQLITE_ROW){
     const char *date=(const char*)sqlite3_column_text(st,2);
     gtk_list_store_append(buffer3,&it);
     gtk_list_store_set(buffer3,&it,0,sqlite3_column_int(st,0),
         1,sqlite3_column_int(st,1),2,date?date:"",-1);
  }
  sqlite3_finalize(st);
}
G_MODULE_EXPORT void on_button13_clicked(GtkButton *btn,gpointer user){
  (void)btn;(void)user;
  GtkTreeModel *model=NULL;GtkTreeIter it;
  GtkTreeSelection *sel=gtk_tree_view_get_selection(table3);
  if(!gtk_tree_selection_get_selected(sel,&model,&it))return;
  int id=0;gtk_tree_model_get(model,&it,C_ID_NAKL,&id,-1);
  OpenRashDocument(id);refresh_rash();
}
G_MODULE_EXPORT void on_button17_clicked(GtkButton *btn,gpointer user){
  (void)btn;(void)user;
  sqlite3_stmt *st=NULL;
  if(sqlite3_prepare_v2(db,"INSERT INTO Накладные(Тип,Дата,Номер,Сумма) "
      "VALUES(0,date('now'),0,0)",-1,&st,NULL)!=SQLITE_OK){rash_error();return;}
  int ok=sqlite3_step(st)==SQLITE_DONE;
  sqlite3_finalize(st);
  if(!ok){rash_error();return;}
  int id=(int)sqlite3_last_insert_rowid(db);
  OpenRashDocument(id);refresh_rash();
}
G_MODULE_EXPORT void on_button18_clicked(GtkButton *btn,gpointer user){
  (void)btn;(void)user;
  GtkTreeModel *model=NULL;GtkTreeIter it;
  GtkTreeSelection *sel=gtk_tree_view_get_selection(table3);
  if(!gtk_tree_selection_get_selected(sel,&model,&it))return;
  int id=0;gtk_tree_model_get(model,&it,C_ID_NAKL,&id,-1);
  if(!DeleteRashDocument(id))rash_error();
  refresh_rash();
}
G_MODULE_EXPORT void on_button19_clicked(GtkButton *btn,gpointer user){
 (void)btn;(void)user;gtk_widget_destroy(wRash);
}
void ShowRash(void){
  builder_rash=gtk_builder_new();
  if(!gtk_builder_add_from_file(builder_rash,"main.glade",NULL)){ShowGladeError();return;}
  wRash=GTK_WIDGET(gtk_builder_get_object(builder_rash,"w_rash"));
  buffer3=GTK_LIST_STORE(gtk_builder_get_object(builder_rash,"liststore3"));
  table3=GTK_TREE_VIEW(gtk_builder_get_object(builder_rash,"treeview3"));
  gtk_window_set_default_size(GTK_WINDOW(wRash),750,420);
  gtk_window_set_position(GTK_WINDOW(wRash),GTK_WIN_POS_CENTER);
  gtk_builder_connect_signals(builder_rash,NULL);
  refresh_rash();
  gtk_widget_show_all(wRash);
}
