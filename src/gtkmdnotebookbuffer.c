#include "gtkmdnotebookbuffer.h"
#include "bufitem/mdnotebookbufitem.h"
#include "bufitem/mdnotebookbufitemheading.h"

typedef struct {
	GListStore* bufitems;
} MdNotebookBufferPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBuffer, mdnotebook_buffer, GTK_TYPE_TEXT_BUFFER)

static void mdnotebook_buffer_changed(GtkTextBuffer* buf) {
	MdNotebookBuffer* self = MDNOTEBOOK_BUFFER(buf);
	MdNotebookBufferPrivate* priv = mdnotebook_buffer_get_instance_private(self);

	GtkTextIter active,end;
	gtk_text_buffer_get_start_iter(buf, &active);
	gtk_text_buffer_get_end_iter(buf, &end);

	guint bufitems = g_list_model_get_n_items(G_LIST_MODEL(priv->bufitems));
	for (guint i = 0; i<bufitems; i++) {
		MdNotebookBufItem* bufitem = MDNOTEBOOK_BUFITEM(g_list_model_get_object(G_LIST_MODEL(priv->bufitems), i));
		mdnotebook_bufitem_changed(bufitem, buf, active, end);
	}
}

static void mdnotebook_buffer_dispose(GObject* object) {
	MdNotebookBuffer* self = MDNOTEBOOK_BUFFER(object);
	MdNotebookBufferPrivate* priv = mdnotebook_buffer_get_instance_private(self);

	g_object_unref(priv->bufitems);
}

static void mdnotebook_buffer_class_init(MdNotebookBufferClass* class) {
	GtkTextBufferClass* text_buffer = GTK_TEXT_BUFFER_CLASS(class);
	GObjectClass* object = G_OBJECT_CLASS(class);

	text_buffer->changed = mdnotebook_buffer_changed;

	object->dispose = mdnotebook_buffer_dispose;
}

static void mdnotebook_buffer_init(MdNotebookBuffer* self) {
	MdNotebookBufferPrivate* priv = mdnotebook_buffer_get_instance_private(self);

	priv->bufitems = g_list_store_new(MDNOTEBOOK_TYPE_BUFITEM);
	// Add basic markdown elements
	MdNotebookBufItem* title = mdnotebook_bufitem_heading_new();
	g_list_store_append(priv->bufitems, title);
	g_object_unref(title);
}

GtkTextBuffer* mdnotebook_buffer_new(GtkTextTagTable* table) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFFER, "tag-table", table, NULL);
}

void mdnotebook_buffer_add_bufitem(MdNotebookBuffer* self, MdNotebookBufItem* item) {
	MdNotebookBufferPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFFER(self));
	priv = mdnotebook_buffer_get_instance_private(self);

	g_list_store_append(priv->bufitems, item);
}
