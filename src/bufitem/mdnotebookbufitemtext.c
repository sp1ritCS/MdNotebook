#include "bufitem/mdnotebookbufitemtext.h"

#define _ __attribute__((unused))

static void mdnotebook_bufitem_text_bufitem_iface_init(MdNotebookBufItemInterface* iface);

G_DEFINE_TYPE_WITH_CODE(MdNotebookBufItemText, mdnotebook_bufitem_text, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(MDNOTEBOOK_TYPE_BUFITEM, mdnotebook_bufitem_text_bufitem_iface_init))

gchar valid_texttags[5][20] = {"mdtextitalic", "mdtextbold", "mdtextbolditalic", "mdtextshown", "mdtexthidden"};
static void strip_texttags(GtkTextBuffer* buf, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);

	for (size_t i = 0; i<6; i++) {
		GtkTextTag* titletag = gtk_text_tag_table_lookup(tagtable, valid_texttags[i]);
		if (titletag)
			gtk_text_buffer_remove_tag(buf, titletag, start, end);
	}
}

gboolean mdnotebook_bufitem_text_is_italic(gunichar ch, _ gpointer user_data) {
	return ch == '*';
}

gboolean mdnotebook_bufitem_text_check_asterisk(gunichar ch, _ gpointer user_data) {
	return ch == '*';
}

typedef struct {
	GtkTextIter start;
	GtkTextIter end;
	gboolean completed;
} TextNode;

static void insert_iter_at_active(GSList** active, const GtkTextIter* iter) {
	if (!(*active)->data) {
		(*active)->data = g_new(TextNode, 1);
		((TextNode*)((*active)->data))->start = *iter;
		((TextNode*)((*active)->data))->completed = FALSE;
	} else {
		((TextNode*)((*active)->data))->end = *iter;
		((TextNode*)((*active)->data))->completed = TRUE;
		GSList* node = g_slist_alloc();
		(*active)->next = node;
		*active = node;
	}
}

typedef struct {
	GtkTextBuffer* buffer;
	GtkTextTag* tag;
	guint8 backtrvl;
	const GtkTextIter* cursor;
} ApplyTextNodeUserdata;
static void apply_tag_to_node(gpointer nodeptr, gpointer tagptr) {
	ApplyTextNodeUserdata* tag = (ApplyTextNodeUserdata*)tagptr;
	TextNode* node = (TextNode*)nodeptr;
	if (!node)
		return;
	if (!node->completed)
		return;

	gtk_text_iter_backward_chars(&node->start, tag->backtrvl);

	gtk_text_buffer_apply_tag(tag->buffer, tag->tag, &node->start, &node->end);

	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(tag->buffer);
	GtkTextTag* markertag;
	// test if cursor is inside the tagarea, ensuring that if it is not, the marker is hidden
	if (gtk_text_iter_in_range(tag->cursor, &node->start, &node->end)) {
		markertag = gtk_text_tag_table_lookup(tagtable, "mdtextshown");
		if (!markertag)
			markertag = gtk_text_buffer_create_tag(tag->buffer, "mdtextshown",
				"foreground", "#A0A8C0",
			NULL);
	} else {
		markertag = gtk_text_tag_table_lookup(tagtable, "mdtexthidden");
		if (!markertag)
			markertag = gtk_text_buffer_create_tag(tag->buffer, "mdtexthidden",
				"invisible", TRUE,
			NULL);
	}

	GtkTextIter
		frontend = node->start,
		backstart = node->end;

	gtk_text_iter_forward_chars(&frontend, tag->backtrvl);
	gtk_text_iter_backward_chars(&backstart, tag->backtrvl);

	gtk_text_buffer_apply_tag(tag->buffer, markertag, &node->start, &frontend);
	gtk_text_buffer_apply_tag(tag->buffer, markertag, &backstart, &node->end);
}

static bool mdnotebook_bufitem_text_test_escaped(const GtkTextIter* ch) {
	GtkTextIter active = *ch;
	g_return_val_if_fail(ch, FALSE);

	gtk_text_iter_backward_char(&active);
	return gtk_text_iter_get_char(&active) == '\\';
}

static void mdnotebook_bufitem_text_bufitem_cursor_changed(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter active = *start;

	GtkTextTag* italictag = gtk_text_tag_table_lookup(tagtable, "mdtextitalic");
	if (!italictag)
		italictag = gtk_text_buffer_create_tag(buf, "mdtextitalic",
			"style", PANGO_STYLE_ITALIC,
		NULL);
	GtkTextTag* boldtag = gtk_text_tag_table_lookup(tagtable, "mdtextbold");
	if (!boldtag)
		boldtag = gtk_text_buffer_create_tag(buf, "mdtextbold",
			"weight", PANGO_WEIGHT_BOLD,
		NULL);
	GtkTextTag* bolditalictag = gtk_text_tag_table_lookup(tagtable, "mdtextbolditalic");
	if (!bolditalictag)
		bolditalictag = gtk_text_buffer_create_tag(buf, "mdtextbolditalic",
			"weight", PANGO_WEIGHT_BOLD,
			"style", PANGO_STYLE_ITALIC,
		NULL);

	GSList* italic_nodes = g_slist_alloc(), *bold_nodes = g_slist_alloc(), *bolditalic_nodes = g_slist_alloc();
	GSList* italic_node_active = italic_nodes, *bold_node_active = bold_nodes, *bolditalic_node_active = bolditalic_nodes;

	strip_texttags(buf, start, end);

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_text_check_asterisk, NULL, end)) {
		if (mdnotebook_bufitem_text_test_escaped(&active))
			continue;

		guint level = 0;
		while (gtk_text_iter_get_char(&active) == '*' && level < 3) {
			level++; gtk_text_iter_forward_char(&active);
		}

		switch (level) {
			case 1:
				insert_iter_at_active(&italic_node_active, &active);
				break;
			case 2:
				insert_iter_at_active(&bold_node_active, &active);
				break;
			case 3:
				insert_iter_at_active(&bolditalic_node_active, &active);
				break;
		}
	}

	GtkTextIter cursor;
	gtk_text_buffer_get_iter_at_mark(buf, &cursor, gtk_text_buffer_get_insert(buf));

	ApplyTextNodeUserdata italic_ud = { .buffer = buf, .tag = italictag, .backtrvl = 1, .cursor = &cursor };
	g_slist_foreach(italic_nodes, apply_tag_to_node, &italic_ud);
	ApplyTextNodeUserdata bold_ud = { .buffer = buf, .tag = boldtag, .backtrvl = 2, .cursor = &cursor };
	g_slist_foreach(bold_nodes, apply_tag_to_node, &bold_ud);
	ApplyTextNodeUserdata bolditalic_ud = { .buffer = buf, .tag = bolditalictag, .backtrvl = 3, .cursor = &cursor };
	g_slist_foreach(bolditalic_nodes, apply_tag_to_node, &bolditalic_ud);

	g_slist_free_full(italic_nodes, g_free);
	g_slist_free_full(bold_nodes, g_free);
	g_slist_free_full(bolditalic_nodes, g_free);
}

static void mdnotebook_bufitem_text_bufitem_buffer_changed(_ MdNotebookBufItem* iface, _ MdNotebookBuffer* self, _ const GtkTextIter* start, _ const GtkTextIter* end) {}

static void mdnotebook_bufitem_text_class_init(_ MdNotebookBufItemTextClass* class) {}
static void mdnotebook_bufitem_text_bufitem_iface_init(MdNotebookBufItemInterface* iface) {
	iface->cursor_changed = mdnotebook_bufitem_text_bufitem_cursor_changed;
	iface->buffer_changed = mdnotebook_bufitem_text_bufitem_buffer_changed;
}

static void mdnotebook_bufitem_text_init(_ MdNotebookBufItemText* self) {}

MdNotebookBufItem* mdnotebook_bufitem_text_new() {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_TEXT, NULL);
}
