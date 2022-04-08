#include "gtkmdnotebookbuffer.h"

typedef struct {
	gdouble zoom;
} MdNotebookBufferPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBuffer, mdnotebook_buffer, GTK_TYPE_TEXT_BUFFER)

static void mdnotebook_buffer_class_init(MdNotebookBufferClass* class) {

}

static void mdnotebook_buffer_init(MdNotebookBuffer* self) {
	MdNotebookBufferPrivate* priv = mdnotebook_buffer_get_instance_private(self);

	priv->zoom = 1;
}

GtkTextBuffer* mdnotebook_buffer_new(GtkTextTagTable* table) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFFER, "tag-table", table, NULL);
}
