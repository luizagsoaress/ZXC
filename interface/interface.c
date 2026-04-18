#include <gtk/gtk.h>
#include <vte/vte.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <math.h>
#include <curl/curl.h>
#include <termios.h>

GtkWidget *nb;
GtkWidget *terminal;

void escapesCleaner(char *src, char *destiny) {
	int i = 0, j = 0;
	if (src[0] == '$' && src[1] == '\'') {
		i = 2;
		int len = strlen(src);
		if (src[len - 1] == '\'')
			src[len - 1] = '\0';
	}
	while (src[i]) {
		if (src[i] == '\\' && src[i + 1]) {
			i++;
			if (src[i] == 'e' || src[i] == 'E')
				destiny[j++] = '\033';
			else if (src[i] == '0' && src[i+1] == '3' && src[i+2] == '3') {
				destiny[j++] = '\033';
				i += 2;
			} else if (src[i] == 'n')
				destiny[j++] = '\n';
			else if (src[i] == 't')
				destiny[j++] = '\t';
			else {
				destiny[j++] = '\\';
				destiny[j++] = src[i];
			}
		} else
			destiny[j++] = src[i];
		i++;
	}
	destiny[j] = '\0';
}

void loadConf() {
	char path[1024];
	snprintf(path, sizeof(path), "%s/.local/share/zxc/conf", getenv("HOME"));
	FILE *conf = fopen(path, "r+");
	char line[1024];
	if (conf) {
		while (fgets(line, sizeof(line), conf)) {
			line[strcspn(line, "\n")] = '\0';
			if (line[0] == '\0' || line[0] == '#') continue;
			char *x = strchr(line, '=');
			if (!x) continue;
			*x = '\0';
			char processed[1024];
			escapesCleaner(x + 1, processed);
			setenv(line, processed, 1);
		}
		fclose(conf);
	}
}

static GtkWidget *createTabTerminal();
void addPage(GtkNotebook *nb, const char *title);
void initialPage(GtkNotebook *nb);
static void activate(GtkApplication *app, gpointer user_data);
void clickButtonAdd(GtkWidget *widget, gpointer data);
void clickButtonClose(GtkWidget *widget, gpointer data);
static gboolean functionCopyPasteOut(GtkWidget *widget, GdkEventKey *event, gpointer data);
void tabFocus();

static GtkWidget *createTabTerminal(void) {
    terminal = vte_terminal_new();
    GdkRGBA bg = {0.1, 0.1, 0.1, 1.0};
    GdkRGBA fg = {1.0, 1.0, 1.0, 1.0};
    vte_terminal_set_color_background(VTE_TERMINAL(terminal), &bg);
    vte_terminal_set_color_foreground(VTE_TERMINAL(terminal), &fg);

    PangoFontDescription *font = pango_font_description_from_string("JetBrains Mono Light 12");
    vte_terminal_set_font(VTE_TERMINAL(terminal), font);
    pango_font_description_free(font);

    vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), 10000);
    vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(terminal), VTE_CURSOR_BLINK_ON);
    
    char* dirHome = getenv("HOME");
	char shellPath[1024];
	snprintf(shellPath, sizeof(shellPath), "%s/.local/bin/main", dirHome);
	
	gchar *shell[] = {shellPath, "-i", NULL};
	
    vte_terminal_spawn_async(
        VTE_TERMINAL(terminal),
        VTE_PTY_DEFAULT,
        g_get_home_dir(),           
        shell,          
        NULL,           
        G_SPAWN_DEFAULT,
        NULL, NULL,     
        NULL,           
        -1,             
        NULL,           
        NULL,          
        NULL            
    );

    g_signal_connect(terminal, "key-press-event", G_CALLBACK(functionCopyPasteOut), terminal);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), terminal);

    return scrolled; 
}

void addPage(GtkNotebook *nb, const char *title) {
    int pos = gtk_notebook_get_n_pages(nb);
    int totalPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));
    gint currentPos = gtk_notebook_get_current_page(GTK_NOTEBOOK(nb));

    GtkWidget * page = createTabTerminal();
    gtk_notebook_insert_page(GTK_NOTEBOOK(nb), page, NULL, pos);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *label = gtk_label_new(title);
    GtkWidget *button = gtk_button_new();
     
    char closeIconPath[1024];
	char addIconPath[1024];
	snprintf(closeIconPath, sizeof(closeIconPath), "%s/.local/share/zxc/images/close.png", g_get_home_dir());
	snprintf(addIconPath, sizeof(addIconPath), "%s/.local/share/zxc/images/add.png", g_get_home_dir());
	
    GdkPixbuf *imageClosePix = gdk_pixbuf_new_from_file_at_scale(closeIconPath, 15, 15, TRUE, NULL);
    GtkWidget *image = gtk_image_new_from_pixbuf(imageClosePix);

    gtk_style_context_add_class(gtk_widget_get_style_context(button), "button");

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        ".button { background: none; border-radius: none; border-color: transparent; border-width: 1px; border-style: solid; box-shadow: none;}", -1, NULL);

    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_widget_set_size_request(button, 20, 20); 
    gtk_container_add(GTK_CONTAINER(hbox), label);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    gtk_widget_show_all(hbox);

    gtk_style_context_add_provider(
        gtk_widget_get_style_context(button),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb), page, hbox);

    g_signal_connect(button, "clicked", G_CALLBACK(clickButtonClose), page);

    tabFocus();
}

void initialPage(GtkNotebook *nb) {
    char title[1024];
    sprintf(title, "%d: ~", 1);
    addPage(nb, title);
}

void clickButtonAdd(GtkWidget *widget, gpointer data) {
    int currentPos = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb)) + 1;
    if(currentPos > 10) {
        return;
    }
    char title[1024];
    sprintf(title, "%d: ~", currentPos);
    addPage(GTK_NOTEBOOK(nb), title);
    gtk_widget_show_all(nb); 
    gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb)));
}

void clickButtonClose(GtkWidget *widget, gpointer data) {
    GtkWidget *pageRemove = (GtkWidget *)data;
    
    gint position = gtk_notebook_page_num(GTK_NOTEBOOK(nb), pageRemove);

    if (position == 0) {
        return;
    }
    
    int totalPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));

    if(totalPages > 1) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(nb), position);
    } 

    totalPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));

    for(int i = 1; i < totalPages; i++) {
        GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(nb), i);
        char title[1024];
        snprintf(title, sizeof(title), "%d: ~", i + 1);

        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        GtkWidget *label = gtk_label_new(title);
        GtkWidget *button = gtk_button_new(); 
        char closeIconPath[1024];
		char addIconPath[1024];
		snprintf(closeIconPath, sizeof(closeIconPath), "%s/.local/share/zxc/images/close.png", g_get_home_dir());
		snprintf(addIconPath, sizeof(addIconPath), "%s/.local/share/zxc/images/add.png", g_get_home_dir());
	
        GdkPixbuf *imageClosePix = gdk_pixbuf_new_from_file_at_scale(closeIconPath, 15, 15, TRUE, NULL);
        GtkWidget *image = gtk_image_new_from_pixbuf(imageClosePix);

        gtk_style_context_add_class(gtk_widget_get_style_context(button), "button");

        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(provider,
            ".button { background: none; border-radius: none; border-color: transparent; border-width: 1px; border-style: solid; box-shadow: none;}", -1, NULL);

        gtk_button_set_image(GTK_BUTTON(button), image);
        gtk_widget_set_size_request(button, 20, 20); 
        gtk_container_add(GTK_CONTAINER(hbox), label);
        gtk_container_add(GTK_CONTAINER(hbox), button);
        gtk_widget_show_all(hbox);

        gtk_style_context_add_provider(
            gtk_widget_get_style_context(button),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );

        gtk_notebook_set_tab_label(GTK_NOTEBOOK(nb), page, hbox);

        g_signal_connect(button, "clicked", G_CALLBACK(clickButtonClose), page);
    }
}

static gboolean functionCopyPasteOut(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    VteTerminal *terminal = VTE_TERMINAL(data);
    guint modf = event->state & gtk_accelerator_get_default_mod_mask();
    gboolean shiftKey = (modf == (GDK_CONTROL_MASK | GDK_SHIFT_MASK));
    gboolean controlKey = (modf == (GDK_CONTROL_MASK));
    
    if (shiftKey) {
        if (event->keyval == GDK_KEY_C || event->keyval == GDK_KEY_c) {
            vte_terminal_copy_clipboard_format(terminal, VTE_FORMAT_TEXT);
            return TRUE;
        } else if (event->keyval == GDK_KEY_V || event->keyval == GDK_KEY_v) {
            GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
            gchar *clip = gtk_clipboard_wait_for_text(clipboard);
            if (clip) {
                vte_terminal_feed_child(terminal, clip, -1);
                g_free(clip);
            }
            return TRUE;
        }
    }

    if(controlKey) {
        if (event->keyval == GDK_KEY_C || event->keyval == GDK_KEY_c) {
            vte_terminal_unselect_all(terminal);
            vte_terminal_feed_child(terminal, "\x03", 1);
            return TRUE;
        }
    }
    
    return FALSE;
}

void tabFocus() {
    GdkRGBA bg = {0.18, 0.18, 0.18, 1.0};
    GdkRGBA bg_active = {0.1, 0.1, 0.1, 1.0};

    gchar *bg_str = gdk_rgba_to_string(&bg);
    gchar *bg_active_str = gdk_rgba_to_string(&bg_active);

    gchar *css = g_strdup_printf(
        ".nb, .nb > *, .nb notebook, .nb header, .nb header tab, .nb header tab label, .nb stack { "
        "  background: %s; "
        "  background-color: %s; "
        "  border-color: transparent; "
        "  color: white; "
        "  border-bottom: 1px solid transparent; "
        "  box-shadow: none; "
        "} "

        ".nb header tab { "
        "  background-color: %s; "
        "} "

        ".nb header tab:checked { "
        "  background-color: %s; "
        "  border-bottom: 1px solid transparent; "
        "  box-shadow: none; "
        "  border-top-left-radius: 10px; "
        "  border-top-right-radius: 10px; "
        "} ",
        bg_str, bg_str, bg_str, bg_active_str
    );

    GtkCssProvider *providerNB = gtk_css_provider_new();
    gtk_css_provider_load_from_data(providerNB, css, -1, NULL);

    gtk_style_context_add_provider(
        gtk_widget_get_style_context(nb),
        GTK_STYLE_PROVIDER(providerNB),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_free(bg_str);
    g_free(bg_active_str);
    g_free(css);

    g_object_unref(providerNB);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "ZXC");
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 500);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(win), overlay);

    nb = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(overlay), nb);

    GtkWidget *buttonAdd = gtk_button_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(buttonAdd), "buttonAdd");

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        ".buttonAdd { background: none; border-radius: none; border-color: transparent; border-width: 1px; border-style: solid; box-shadow: none;}", -1, NULL);

    gtk_style_context_add_class(gtk_widget_get_style_context(nb), "nb");

    GdkRGBA bg = {0.1, 0.1, 0.1, 1.0}; 

    gchar *bg_str = gdk_rgba_to_string(&bg);

    gchar *css = g_strdup_printf(
    ".nb, .nb > *, .nb notebook, .nb header, .nb header tab, .nb header tab label, .nb stack  { "
    "  background: %s; "
    "  background-color: %s; "
    "  border-color: transparent; "
    "  color: white; "
    "}", bg_str, bg_str);

    GtkCssProvider *providerNB = gtk_css_provider_new();
    gtk_css_provider_load_from_data(providerNB, css, -1, NULL);

    gtk_style_context_add_provider(
        gtk_widget_get_style_context(buttonAdd),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    gtk_style_context_add_provider(
        gtk_widget_get_style_context(nb),
        GTK_STYLE_PROVIDER(providerNB),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
    
    char closeIconPath[1024];
	char addIconPath[1024];
	snprintf(closeIconPath, sizeof(closeIconPath), "%s/.local/share/zxc/images/close.png", g_get_home_dir());
	snprintf(addIconPath, sizeof(addIconPath), "%s/.local/share/zxc/images/add.png", g_get_home_dir());

    GdkPixbuf *imagePlusPix = gdk_pixbuf_new_from_file_at_scale(addIconPath, 12, 12, TRUE, NULL);
    GtkWidget *image = gtk_image_new_from_pixbuf(imagePlusPix);
    gtk_button_set_image(GTK_BUTTON(buttonAdd), image);
    gtk_widget_set_size_request(buttonAdd, 20, 20); 
    gtk_widget_set_halign(buttonAdd, GTK_ALIGN_END);   
    gtk_widget_set_valign(buttonAdd, GTK_ALIGN_START); 
    gtk_widget_set_margin_top(buttonAdd, 4);
    gtk_widget_set_margin_end(buttonAdd, 4);

    initialPage(GTK_NOTEBOOK(nb));


    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), buttonAdd);

    g_signal_connect(buttonAdd, "clicked", G_CALLBACK(clickButtonAdd), NULL);
    
    gtk_widget_show_all(win);
    g_object_unref(imagePlusPix);
}

int main(int argc, char **argv) {
    loadConf();
    gtk_init(&argc, &argv);
    GtkApplication *app = gtk_application_new("org.example.nb", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
