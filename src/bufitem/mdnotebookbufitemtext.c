#include "bufitem/mdnotebookbufitemtext.h"

#define _ __attribute__((unused))

static void mdnotebook_bufitem_text_bufitem_iface_init(MdNotebookBufItemInterface* iface);

G_DEFINE_TYPE_WITH_CODE(MdNotebookBufItemText, mdnotebook_bufitem_text, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(MDNOTEBOOK_TYPE_BUFITEM, mdnotebook_bufitem_text_bufitem_iface_init))

typedef struct {
	char* tagname;
	guint8 trvl;
} TextTagNode;
#define TEXTTAG_NODES_NUM 5
TextTagNode texttag_nodes[TEXTTAG_NODES_NUM] = {
	// ASTERISK (*) Nodes
	{.tagname = "mdtextitalic", .trvl = 1},
	{.tagname = "mdtextbold", .trvl = 2},
	{.tagname = "mdtextbolditalic", .trvl = 3},
	// UNDERSCORE (_) Nodes
	{.tagname = "mdtextunderline", .trvl = 2},
	// TILDE (~) Nodes
	{.tagname = "mdtextstrikethrough", .trvl = 2}
};

static void mdnotebook_bufitem_text_bufitem_cursor_changed(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter cursor;
	GtkTextTag *showntag,*hiddentag;

	gtk_text_buffer_get_iter_at_mark(buf, &cursor, gtk_text_buffer_get_insert(buf));

	showntag = gtk_text_tag_table_lookup(tagtable, "mdtextshown");
	if (!showntag)
		showntag = gtk_text_buffer_create_tag(buf, "mdtextshown",
			"foreground", "#A0A8C0",
		NULL);

	hiddentag = gtk_text_tag_table_lookup(tagtable, "mdtexthidden");
	if (!hiddentag)
		hiddentag = gtk_text_buffer_create_tag(buf, "mdtexthidden",
			//"background", "red",
			"invisible", TRUE,
		NULL);

	gtk_text_buffer_remove_tag(buf, showntag, start, end);
	gtk_text_buffer_remove_tag(buf, hiddentag, start, end);

	for (gsize i = 0; i < TEXTTAG_NODES_NUM; i++) {
		GtkTextIter beginstart, beginend, endend, active = *start;
		GtkTextTag* tag = gtk_text_tag_table_lookup(tagtable, texttag_nodes[i].tagname);
		if (!tag)
			continue;
		while (gtk_text_iter_forward_to_tag_toggle(&active, tag)) {
			beginstart = active;
			gtk_text_iter_forward_chars(&active, texttag_nodes[i].trvl);
			beginend = active;
			if (!gtk_text_iter_forward_to_tag_toggle(&active, tag))
				goto next_textitem;
			endend = active;
			gtk_text_iter_backward_chars(&active, texttag_nodes[i].trvl);

			GtkTextTag* markertag;
			if (gtk_text_iter_in_range(&cursor, &beginstart, &endend))
				markertag = showntag;
			else
				markertag = hiddentag;

			gtk_text_buffer_apply_tag(buf, markertag, &beginstart, &beginend);
			gtk_text_buffer_apply_tag(buf, markertag, &active, &endend);

			gtk_text_iter_forward_chars(&active, texttag_nodes[i].trvl);
		}
next_textitem:
		(void)0;
	}
}

static void strip_texttags(GtkTextBuffer* buf, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);

	for (gsize i = 0; i < TEXTTAG_NODES_NUM; i++) {
		GtkTextTag* titletag = gtk_text_tag_table_lookup(tagtable, texttag_nodes[i].tagname);
		if (titletag)
			gtk_text_buffer_remove_tag(buf, titletag, start, end);
	}
}

gboolean mdnotebook_bufitem_text_check_char(gunichar ch, _ gpointer user_data) {
	return ch == (gsize)user_data;
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
} ApplyTextNodeUserdata;
#define CAST_GFUNC(f) ((GFunc) (f))
static void apply_tag_to_node(TextNode* node, ApplyTextNodeUserdata* tag) {
	if (!node)
		return;
	if (!node->completed)
		return;

	gtk_text_iter_backward_chars(&node->start, tag->backtrvl);
	gtk_text_buffer_apply_tag(tag->buffer, tag->tag, &node->start, &node->end);
}

static bool mdnotebook_bufitem_text_test_escaped(const GtkTextIter* ch) {
	GtkTextIter active = *ch;
	g_return_val_if_fail(ch, FALSE);

	gtk_text_iter_backward_char(&active);
	return gtk_text_iter_get_char(&active) == '\\';
}

static void mdnotebook_bufitem_text_apply_asterisk_items(MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
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

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_text_check_char, (gpointer)'*', end)) {
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

	ApplyTextNodeUserdata italic_ud = { .buffer = buf, .tag = italictag, .backtrvl = 1 };
	g_slist_foreach(italic_nodes, CAST_GFUNC(apply_tag_to_node), &italic_ud);
	ApplyTextNodeUserdata bold_ud = { .buffer = buf, .tag = boldtag, .backtrvl = 2 };
	g_slist_foreach(bold_nodes, CAST_GFUNC(apply_tag_to_node), &bold_ud);
	ApplyTextNodeUserdata bolditalic_ud = { .buffer = buf, .tag = bolditalictag, .backtrvl = 3 };
	g_slist_foreach(bolditalic_nodes, CAST_GFUNC(apply_tag_to_node), &bolditalic_ud);

	g_slist_free_full(italic_nodes, g_free);
	g_slist_free_full(bold_nodes, g_free);
	g_slist_free_full(bolditalic_nodes, g_free);
}

static void mdnotebook_bufitem_text_apply_underscore_items(MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter active = *start;

	GtkTextTag* underlinetag = gtk_text_tag_table_lookup(tagtable, "mdtextunderline");
	if (!underlinetag)
		underlinetag = gtk_text_buffer_create_tag(buf, "mdtextunderline",
			"underline-set", TRUE,
			"underline", PANGO_UNDERLINE_SINGLE,
		NULL);

	GSList* underline_nodes = g_slist_alloc();
	GSList* underline_node_active = underline_nodes;

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_text_check_char, (gpointer)'_', end)) {
		if (mdnotebook_bufitem_text_test_escaped(&active))
			continue;

		guint level = 0;
		while (gtk_text_iter_get_char(&active) == '_' && level < 2) {
			level++; gtk_text_iter_forward_char(&active);
		}

		switch (level) {
			case 2:
				insert_iter_at_active(&underline_node_active, &active);
				break;
		}
	}

	ApplyTextNodeUserdata underline_ud = { .buffer = buf, .tag = underlinetag, .backtrvl = 2 };
	g_slist_foreach(underline_nodes, CAST_GFUNC(apply_tag_to_node), &underline_ud);

	g_slist_free_full(underline_nodes, g_free);
}

static void mdnotebook_bufitem_text_apply_tilde_items(MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter active = *start;

	GtkTextTag* strikethroughtag = gtk_text_tag_table_lookup(tagtable, "mdtextstrikethrough");
	if (!strikethroughtag)
		strikethroughtag = gtk_text_buffer_create_tag(buf, "mdtextstrikethrough",
			"strikethrough-set", TRUE,
			"strikethrough", TRUE,
		NULL);

	GSList* strikethrough_nodes = g_slist_alloc();
	GSList* strikethrough_node_active = strikethrough_nodes;

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_text_check_char, (gpointer)'~', end)) {
		if (mdnotebook_bufitem_text_test_escaped(&active))
			continue;

		guint level = 0;
		while (gtk_text_iter_get_char(&active) == '~' && level < 2) {
			level++; gtk_text_iter_forward_char(&active);
		}

		switch (level) {
			case 2:
				insert_iter_at_active(&strikethrough_node_active, &active);
				break;
		}
	}

	ApplyTextNodeUserdata strikethrough_ud = { .buffer = buf, .tag = strikethroughtag, .backtrvl = 2 };
	g_slist_foreach(strikethrough_nodes, CAST_GFUNC(apply_tag_to_node), &strikethrough_ud);

	g_slist_free_full(strikethrough_nodes, g_free);
}

static void mdnotebook_bufitem_text_bufitem_buffer_changed(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);

	strip_texttags(buf, start, end);
	mdnotebook_bufitem_text_apply_asterisk_items(self, start, end);
	mdnotebook_bufitem_text_apply_underscore_items(self, start, end);
	mdnotebook_bufitem_text_apply_tilde_items(self, start, end);
}

static void mdnotebook_bufitem_text_class_init(_ MdNotebookBufItemTextClass* class) {}
static void mdnotebook_bufitem_text_bufitem_iface_init(MdNotebookBufItemInterface* iface) {
	iface->cursor_changed = mdnotebook_bufitem_text_bufitem_cursor_changed;
	iface->buffer_changed = mdnotebook_bufitem_text_bufitem_buffer_changed;
}

static void mdnotebook_bufitem_text_init(_ MdNotebookBufItemText* self) {}

MdNotebookBufItem* mdnotebook_bufitem_text_new() {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_TEXT, NULL);
}
