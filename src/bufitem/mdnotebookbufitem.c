#include "bufitem/mdnotebookbufitem.h"

#define _ __attribute__((unused))

G_DEFINE_ABSTRACT_TYPE (MdNotebookBufItem, mdnotebook_bufitem, G_TYPE_OBJECT)

static void mdnotebook_bufitem_class_init(MdNotebookBufItemClass* class) {
	class->registered = NULL;
	class->cursor_changed = NULL;
	class->buffer_changed = NULL;
	class->on_insert = NULL;
}

static void mdnotebook_bufitem_init(_ MdNotebookBufItem* self) {}

void mdnotebook_bufitem_registered(MdNotebookBufItem* self, MdNotebookBuffer* buffer) {
	MdNotebookBufItemClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM(self));

	class = MDNOTEBOOK_BUFITEM_GET_CLASS(self);
	if (class->registered != NULL)
		class->registered(self, buffer);
}

void mdnotebook_bufitem_cursor_changed(MdNotebookBufItem *self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	MdNotebookBufItemClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM(self));

	class = MDNOTEBOOK_BUFITEM_GET_CLASS(self);
	if (class->cursor_changed != NULL)
		class->cursor_changed(self, buffer, start, end);
}

void mdnotebook_bufitem_buffer_changed(MdNotebookBufItem *self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	MdNotebookBufItemClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM(self));

	class = MDNOTEBOOK_BUFITEM_GET_CLASS(self);
	g_return_if_fail (class->buffer_changed != NULL);
	class->buffer_changed(self, buffer, start, end);
}

void mdnotebook_bufitem_on_insert(MdNotebookBufItem* self, MdNotebookBuffer* buffer, GtkTextMark* location, gchar* text, gint len) {
	MdNotebookBufItemClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM(self));

	class = MDNOTEBOOK_BUFITEM_GET_CLASS(self);
	if (class->on_insert)
		class->on_insert(self, buffer, location, text, len);
}


// Common functions implementors might need

GtkTextTag* mdnotebook_bufitem_get_private_tag(MdNotebookBuffer* self) {
	GtkTextBuffer* buf;
	GtkTextTagTable* tagtable;
	GtkTextTag* private;

	g_return_val_if_fail(MDNOTEBOOK_IS_BUFFER(self), NULL);

	buf = GTK_TEXT_BUFFER(self);
	tagtable = gtk_text_buffer_get_tag_table(buf);

	private = gtk_text_tag_table_lookup(tagtable, "mdprivate");
	if (!private)
		private = gtk_text_buffer_create_tag(buf, "mdprivate", NULL);

	return private;
}

gboolean mdnotebook_bufitem_is_iter_in_private(MdNotebookBuffer* self, const GtkTextIter* it) {
	GtkTextBuffer* buf;
	GtkTextTagTable* tagtable;
	GtkTextTag* private;

	g_return_val_if_fail(MDNOTEBOOK_IS_BUFFER(self), FALSE);

	buf = GTK_TEXT_BUFFER(self);
	tagtable = gtk_text_buffer_get_tag_table(buf);

	private = gtk_text_tag_table_lookup(tagtable, "mdprivate");
	if (private)
		return gtk_text_iter_has_tag(it, private);

	return FALSE;
}

void mdnotebook_butitem_strip_private(MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(self));

	GtkTextTag* privatetag = gtk_text_tag_table_lookup(tagtable, "mdprivate");
	if (privatetag)
		gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(self), privatetag, start, end);
}

gboolean mdnotebook_bufitem_check_char(gunichar ch, gpointer user_data) {
	return ch == (gsize)user_data;
}

gboolean mdnotebook_bufitem_check_backward_whitespace(const GtkTextIter* ch) {
	GtkTextIter active = *ch;
	gtk_text_iter_set_line_offset(&active, 0);

	while (gtk_text_iter_compare(&active, ch) < 0) {
		gunichar c = gtk_text_iter_get_char(&active);
		if (c != ' ' && c != '\t')
			return FALSE;
		gtk_text_iter_forward_char(&active);
	}

	return TRUE;
}

gboolean mdnotebook_bufitem_get_tag_extends(const GtkTextIter* t, GtkTextTag* tag, GtkTextIter* left, GtkTextIter* right) {
	GtkTextIter active = *t;
	if (gtk_text_iter_starts_tag(&active, tag)) {
		*left = active;
		gtk_text_iter_forward_to_tag_toggle(&active, tag);
		*right = active;
		return TRUE;
	}
	if (gtk_text_iter_ends_tag(&active, tag)) {
		*right = active;
		gtk_text_iter_backward_to_tag_toggle(&active, tag);
		*left = active;
		return TRUE;
	}
	if (gtk_text_iter_has_tag(&active, tag)) {
		*left = active;
		gtk_text_iter_backward_to_tag_toggle(left, tag);
		*right = active;
		gtk_text_iter_forward_to_tag_toggle(right, tag);
		return TRUE;
	}
	return FALSE;
}
