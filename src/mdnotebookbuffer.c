#include "mdnotebookbuffer.h"
#include "bufitem/mdnotebookbufitem.h"
#include "bufitem/mdnotebookbufitemdynblock.h"
#include "bufitem/mdnotebookbufitemheading.h"
#include "bufitem/mdnotebookbufitemtext.h"

#define _ __attribute__((unused))

typedef struct {
	GListStore* bufitems;
} MdNotebookBufferPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBuffer, mdnotebook_buffer, GTK_TYPE_TEXT_BUFFER)

static void mdnotebook_buffer_cursor_changed(MdNotebookBuffer* self, _ GParamSpec* pspec, _ gpointer user_data) {
	MdNotebookBufferPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFFER(self));
	priv = mdnotebook_buffer_get_instance_private(self);

	GtkTextIter start,end;
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(self), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(self), &end);

	guint bufitems = g_list_model_get_n_items(G_LIST_MODEL(priv->bufitems));
	for (guint i = 0; i<bufitems; i++) {
		MdNotebookBufItem* bufitem = MDNOTEBOOK_BUFITEM(g_list_model_get_object(G_LIST_MODEL(priv->bufitems), i));
		mdnotebook_bufitem_cursor_changed(bufitem, self, &start, &end);
	}
}

static void mdnotebook_buffer_changed(GtkTextBuffer* buf) {
	MdNotebookBuffer* self = MDNOTEBOOK_BUFFER(buf);
	MdNotebookBufferPrivate* priv = mdnotebook_buffer_get_instance_private(self);

	GtkTextIter start,end;
	gtk_text_buffer_get_start_iter(buf, &start);
	gtk_text_buffer_get_end_iter(buf, &end);

	guint bufitems = g_list_model_get_n_items(G_LIST_MODEL(priv->bufitems));
	for (guint i = 0; i<bufitems; i++) {
		MdNotebookBufItem* bufitem = MDNOTEBOOK_BUFITEM(g_list_model_get_object(G_LIST_MODEL(priv->bufitems), i));
		mdnotebook_bufitem_buffer_changed(bufitem, MDNOTEBOOK_BUFFER(buf), &start, &end);
	}
}

static void mdnotebook_buffer_on_insert(MdNotebookBuffer* self, const GtkTextIter* location, gchar* text, gint len, _ gpointer user_data) {
	MdNotebookBufferPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFFER(self));
	priv = mdnotebook_buffer_get_instance_private(self);

	GtkTextIter start,end;
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(self), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(self), &end);

	guint bufitems = g_list_model_get_n_items(G_LIST_MODEL(priv->bufitems));
	for (guint i = 0; i<bufitems; i++) {
		MdNotebookBufItem* bufitem = MDNOTEBOOK_BUFITEM(g_list_model_get_object(G_LIST_MODEL(priv->bufitems), i));
		mdnotebook_bufitem_on_insert(bufitem, self, location, text, len);
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

	g_signal_connect(self, "notify::cursor-position", G_CALLBACK(mdnotebook_buffer_cursor_changed), NULL);
	g_signal_connect_object(self, "insert-text", G_CALLBACK(mdnotebook_buffer_on_insert), NULL, G_CONNECT_AFTER);

	priv->bufitems = g_list_store_new(MDNOTEBOOK_TYPE_BUFITEM);
	// Add basic markdown elements
	MdNotebookBufItem* title = mdnotebook_bufitem_heading_new();
	MdNotebookBufItem* dynblock = mdnotebook_bufitem_dynblock_new();
	MdNotebookBufItem* text = mdnotebook_bufitem_text_new();
	g_list_store_append(priv->bufitems, title);
	g_list_store_append(priv->bufitems, dynblock);
	g_list_store_append(priv->bufitems, text);
	g_object_unref(title);
	g_object_unref(dynblock);
	g_object_unref(text);
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
