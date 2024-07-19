#include <gtk/gtk.h>

int num1 = 0;	//1つめの数値
int num2 = 0;	//2つめの数値
char* sym = "";	//記号

static void click_num(GtkButton *btn ,GtkButton *typing){
	g_print("%s",gtk_button_get_label(btn));
}

static void click_symbol(GtkButton *btn,GtkButton *symbol){
	gtk_button_set_label (symbol, gtk_button_get_label (btn));
}

static void click_display(GtkButton *btn){

}

static void app_activate(GApplication *app, gpointer *user_data) {
	char * numlabel[] = {"7","8","9","4","5","6","3","2","1","0"};	//数字ボタン
	size_t nll = sizeof(numlabel) / sizeof(numlabel[0]);	//numlabel[]の長さ

	char * symbollabel[] = {"+","-","*","/","="};	//記号ボタン
	size_t sll = sizeof(symbollabel) / sizeof(symbollabel[0]);	//symbollabel[]の長さ

	GtkWidget *win;	//ウィンドウ
	GtkWidget *grid;
	GtkWidget *nbtn[nll]; //ボタン
	GtkWidget *sbtn[sll]; //ボタン

	GtkWidget *entered; //入力された数値
	GtkWidget *Typing; //入力中の数値
	GtkWidget *symbol; //記号

	win = gtk_application_window_new(GTK_APPLICATION (app));	//ウィンドウを作成
	gtk_window_set_title(GTK_WINDOW (win), "Calculator");	//ウィンドウのタイトル
  	gtk_window_set_default_size(GTK_WINDOW (win), 400, 300);	//ウィンドウサイズ

	//gridを作成
	grid = gtk_grid_new();
	//gridをウィンドウに配置
	gtk_window_set_child(GTK_WINDOW (win), grid);

	entered = gtk_button_new_with_label("");
	g_signal_connect(entered, "clicked", G_CALLBACK (click_display), NULL);
	gtk_grid_attach(GTK_GRID (grid), entered, 0, 0, 10, 1);

	Typing = gtk_button_new_with_label("");
	g_signal_connect(Typing, "clicked", G_CALLBACK (click_display), NULL);
	gtk_grid_attach(GTK_GRID (grid), Typing, 0, 1, 10, 1);

	symbol = gtk_button_new_with_label(sym);
	g_signal_connect(symbol, "clicked", G_CALLBACK (click_display), NULL);
	gtk_grid_attach(GTK_GRID (grid), symbol, 11, 1, 1, 1);

	int j = 0;
	int k = 2;
	//数字ボタン表示
	for (int i = 0; i < nll; i++){
		nbtn[i] = gtk_button_new_with_label(numlabel[i]);
		g_signal_connect(nbtn[i], "clicked", G_CALLBACK (click_num), Typing);
		gtk_grid_attach(GTK_GRID (grid), nbtn[i], j, k, 1, 1);
		j++;
		if(j == 3){
			j = 0;
			k++;
		}
	}
	//記号ボタン表示
	for (int i = 0; i < sll; i++){
		sbtn[i] = gtk_button_new_with_label(symbollabel[i]);
		g_signal_connect(sbtn[i], "clicked", G_CALLBACK (click_symbol), symbol);
		gtk_grid_attach(GTK_GRID (grid), sbtn[i], j, k, 1, 1);
		j++;
		if(j == 3){
			j = 0;
			k++;
		}
	}
	gtk_window_present(GTK_WINDOW (win));
}

int main(int argc, char **argv) {
	GtkApplication *app;
	int stat;

	app = gtk_application_new("hoge.test.calc", G_APPLICATION_DEFAULT_FLAGS);

	g_signal_connect(app, "activate", G_CALLBACK (app_activate), NULL);
	//ウィジェットなどのインスタンス,イベント,G_CALLBACK(呼びたい関数), ハンドラー(呼びたい関数)に渡したい変数(ない場合NULL)

	stat =g_application_run(G_APPLICATION (app), argc, argv);
	g_object_unref(app);
	return stat;
}