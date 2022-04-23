#include "mdnotebookbuffer.h"
#define MDNOTEBOOK_EXPOSE_INTERNAS
#include "bufitem/mdnotebookbufitem.h"

#define _ __attribute__((unused))

typedef struct {
	GListStore* bufitems;
} MdNotebookBufferPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBuffer, mdnotebook_buffer, GTK_TYPE_TEXT_BUFFER)

static void mdnotebook_buffer_cursor_changed(MdNotebookBuffer* self, _ GParamSpec* pspec, _ gpointer user_data) {
	MdNotebookBufferPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFFER(self));
	priv = mdnotebook_buffer_get_instance_private(self);

	/* selecting text is mostly frustrating if the text layout
	 * changes due to hiding/unhiding while doing it */
	if (gtk_text_buffer_get_has_selection(GTK_TEXT_BUFFER(self))) return;

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

	mdnotebook_butitem_strip_private(self, &start, &end);

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

	GtkTextMark* location_mark = gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(self), NULL, location, true);
	guint bufitems = g_list_model_get_n_items(G_LIST_MODEL(priv->bufitems));
	for (guint i = 0; i<bufitems; i++) {
		MdNotebookBufItem* bufitem = MDNOTEBOOK_BUFITEM(g_list_model_get_object(G_LIST_MODEL(priv->bufitems), i));
		mdnotebook_bufitem_on_insert(bufitem, self, location_mark, text, len);
	}
	gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(self), location_mark);

	/* --FFR--
	 * If there is ever a signal handler on self->insert-text, that gets
	 * called after this handler, there will be warnings (and likely crashes)
	 * because location is invalid if one of the BufItems decieds to insert
	 * text into the Buffer during with it's on_insert handler.
	 *
	 * I work arround this here by creating a Mark at `location` and passing
	 * that Mark down to the seperate BufItems, fron which they get the
	 * corresponding Iter seperatly. If they decide to insert something into
	 * the Buffer, it is their reposibility to make sure to not reuse the old
	 * iter during their execution. After that it doesn't matter, since the
	 * next BufItem will re-query the Iter for itself.
	 *
	 * The issue is just, that this will not be happening for a potential
	 * second signal handler, if any such would be connected to this
	 * signal, as that will receive the initial `location` Iter, which is
	 * invalid if text was inserted.
	 * NoteKit works arround this, by "fixing up" the `location` Iter, by
	 * simply disregarding the fact that it is `const` and thus should not be
	 * modified and overwriting it with a new iter queried from a mark using a
	 * similar process as described above. @blackhole89 called the Iter
	 * "erroneously set [...] to const", which sadly isn't correct (anymore).
	 * While one seems to get away with overwriting it, there is risk about
	 * binary incompatabilties that lead to crashes on optimized builds if GTK,
	 * since compilers tend to optimize code with `const` pointers for better
	 * performance.
	 *
	 * But should I ever run into an issue as described at the beginning, I'll
	 * probably just do the same as NoteKit ._. (this is likely to occur when
	 * implementing spellcheck [I ran into this with NoteKit when implementing
	 * a Spellchecker using gspell], however it might not, depending on how
	 * GTK will handle GNOME/gtk#3814)
	 */
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
}

GtkTextBuffer* mdnotebook_buffer_new(GtkTextTagTable* table) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFFER, "tag-table", table, NULL);
}

static gboolean mdnotebook_buffer_cmp_bufitem_type(gconstpointer lhs, gconstpointer rhs) {
	return G_OBJECT_TYPE(lhs) == G_OBJECT_TYPE(rhs);
}

void mdnotebook_buffer_add_bufitem(MdNotebookBuffer* self, MdNotebookBufItem* item) {
	MdNotebookBufferPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFFER(self));
	priv = mdnotebook_buffer_get_instance_private(self);

	if (g_list_store_find_with_equal_func(priv->bufitems, item, mdnotebook_buffer_cmp_bufitem_type, NULL)) {
		g_warning("%s is already registered in the MdNotebook.Buffer\n", g_type_name(G_OBJECT_TYPE(item)));
	} else {
		g_list_store_append(priv->bufitems, item);
		mdnotebook_bufitem_registered(item, self);
	}

	g_object_unref(item);
}
