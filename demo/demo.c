#include <gtk/gtk.h>
#include <gtkmdnotebook.h>

static void activate(GtkApplication* app, gpointer) {
	GtkWidget *window,*shell,*scroll,*view,*bufwgt,*innerwgt,*btmbar,*zoom_slider;
	MdNotebookBuffer* buf;
	GtkTextIter iter;
	GtkTextChildAnchor* anch;
	GtkAdjustment* zoom_adj;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "MdNotebook Demo");
	gtk_window_set_default_size(GTK_WINDOW(window), 853, 480);

	shell = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	scroll = gtk_scrolled_window_new();
	gtk_widget_set_hexpand(scroll, TRUE);
	gtk_widget_set_vexpand(scroll, TRUE);

	view = mdnotebook_zoomview_new();
	buf = MDNOTEBOOK_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mdnotebook_zoomview_get_textview(MDNOTEBOOK_ZOOMVIEW(view)))));

	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buf), &iter);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buf), &iter, "Hello World!\n  - MdNotebook Demo", -1);

	btmbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_halign(btmbar, GTK_ALIGN_END);

	zoom_adj = gtk_adjustment_new(1., 0., 5., 1., 0., 0.);
	zoom_slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, zoom_adj);
	gtk_widget_set_size_request(zoom_slider, 200, -1);
	gtk_scale_add_mark(GTK_SCALE(zoom_slider), 1., GTK_POS_BOTTOM, "100%");
	gtk_scale_add_mark(GTK_SCALE(zoom_slider), 2.5, GTK_POS_BOTTOM, "250%");

	g_object_bind_property(zoom_adj, "value", view, "zoom", G_BINDING_BIDIRECTIONAL);

	gtk_box_append(GTK_BOX(btmbar), zoom_slider);

	bufwgt = mdnotebook_bufwidget_new();
	innerwgt = gtk_button_new_with_label("in View");
	mdnotebook_bufwidget_set_child(MDNOTEBOOK_BUFWIDGET(bufwgt), innerwgt);

	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buf), &iter);
	anch = gtk_text_buffer_create_child_anchor(GTK_TEXT_BUFFER(buf), &iter);
	gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(mdnotebook_zoomview_get_textview(MDNOTEBOOK_ZOOMVIEW(view))), bufwgt, anch);



	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), view);

	gtk_box_append(GTK_BOX(shell), scroll);
	gtk_box_append(GTK_BOX(shell), btmbar);

	gtk_window_set_child(GTK_WINDOW(window), shell);

	gtk_widget_show(window);
}

int main(int argc, char** argv) {
	GtkApplication* app;
	int status;

	app = gtk_application_new("arpa.sp1rit.MdNotebookDemo", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
