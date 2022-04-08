#include "gtkmdnotebookview.h"

static GtkTextBuffer* mdnotebook_view_create_buffer(GtkTextView*) {
	return mdnotebook_buffer_new(NULL);
}


typedef struct {
	guint8 none;
} MdNotebookViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookView, mdnotebook_view, GTK_TYPE_TEXT_VIEW)

static void mdnotebook_view_class_init(MdNotebookViewClass* class) {
	GTK_TEXT_VIEW_CLASS(class)->create_buffer = mdnotebook_view_create_buffer;
}

static void mdnotebook_view_init(MdNotebookView* self) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);
}

GtkWidget* mdnotebook_view_new(void) {
	return g_object_new(MDNOTEBOOK_TYPE_VIEW, NULL);
}

GtkWidget* mdnotebook_view_new_with_buffer(MdNotebookBuffer* buffer) {
	GtkWidget* view = mdnotebook_view_new();
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), GTK_TEXT_BUFFER(buffer));

	return view;
}
