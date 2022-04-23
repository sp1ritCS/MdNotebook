#include "bufitem/mdnotebookbufitemcodeblock.h"

#define _ __attribute__((unused))

G_DEFINE_TYPE(MdNotebookBufItemCodeblock, mdnotebook_bufitem_codeblock, MDNOTEBOOK_TYPE_BUFITEM)

static void strip_codeblocktag(GtkTextBuffer* buf, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);

	GtkTextTag* codeblocktag = gtk_text_tag_table_lookup(tagtable, "mdcodeblock");
	if (codeblocktag)
		gtk_text_buffer_remove_tag(buf, codeblocktag, start, end);
}

static void mdnotebook_bufitem_codeblock_bufitem_buffer_changed(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter active = *start;

	GtkTextTag* codeblocktag = gtk_text_tag_table_lookup(tagtable, "mdcodeblock");
	if (!codeblocktag)
		codeblocktag = gtk_text_buffer_create_tag(buf, "mdcodeblock",
			"family", "monospace",
		NULL);

	GtkTextTag* privatetag = mdnotebook_bufitem_get_private_tag(self);

	strip_codeblocktag(buf, start, end);

	GtkTextIter beginblock, languagestart;
	while (gtk_text_iter_forward_search(&active, "```", GTK_TEXT_SEARCH_TEXT_ONLY, &beginblock, &languagestart, end)) {
		if (mdnotebook_bufitem_is_iter_in_private(self, &languagestart))
			continue;
		if (!gtk_text_iter_ends_line(&languagestart)) {
			GtkTextIter languageend = languagestart;
			gtk_text_iter_forward_to_line_end(&languageend);
			gchar* language = gtk_text_iter_get_text(&languagestart, &languageend);
			g_info("MdNotebookBufItemCodeblock at %d has language %s\n", gtk_text_iter_get_line(&beginblock), language);
			g_free(language);
		}

		GtkTextIter endblock = languagestart;
		while (gtk_text_iter_compare(&languagestart, &endblock) == 0 || mdnotebook_bufitem_is_iter_in_private(self, &endblock)) {
			if (!gtk_text_iter_forward_search(&endblock, "```", GTK_TEXT_SEARCH_TEXT_ONLY, NULL, &endblock, end)) {
				endblock = *end;
				break;
			}

		}

		gtk_text_buffer_apply_tag(buf, codeblocktag, &beginblock, &endblock);
		gtk_text_buffer_apply_tag(buf, privatetag, &beginblock, &endblock);

		active = endblock;
	}
}

static void mdnotebook_bufitem_codeblock_bufitem_on_insert(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, GtkTextMark* location, gchar* text, gint) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_mark(buf, &iter, location);
	if(g_strcmp0(text, "\n") == 0) {
		GtkTextIter endln;
		GtkTextTagTable* tagtable;
		GtkTextTag* codeblocktag;

		if (mdnotebook_bufitem_is_iter_in_private(self, &iter)) return;
		// TODO: implement reference to MdNotebook.View
		//if(self->modifier_keys & GDK_SHIFT_MASK) return;
		if(!gtk_text_iter_ends_line(&iter)) return;

		gtk_text_iter_backward_line(&iter);

		endln = iter;
		gtk_text_iter_forward_to_line_end(&endln);
		if (!gtk_text_iter_forward_search(&iter, "```", GTK_TEXT_SEARCH_TEXT_ONLY, NULL, NULL, &endln))
			return;

		tagtable = gtk_text_buffer_get_tag_table(buf);
		codeblocktag = gtk_text_tag_table_lookup(tagtable, "mdcodeblock");
		if (codeblocktag && gtk_text_iter_ends_tag(&endln, codeblocktag))
			return;

		gtk_text_buffer_get_iter_at_mark(buf, &iter, location);
		gtk_text_iter_forward_line(&iter);
		endln = iter;
		gtk_text_iter_forward_to_line_end(&endln);

		if (gtk_text_iter_forward_search(&iter, "```", GTK_TEXT_SEARCH_TEXT_ONLY, NULL, NULL, &endln))
			return;

		//GtkTextMark* cursor = gtk_text_buffer_create_mark(buf, NULL, location, FALSE);
		gtk_text_buffer_get_iter_at_mark(buf, &iter, location);
		gtk_text_buffer_insert(buf, &iter, "\n```", -1);
		gtk_text_buffer_get_iter_at_mark(buf, &iter, location);
		gtk_text_buffer_place_cursor(buf, &iter);
	}
}

static void mdnotebook_bufitem_codeblock_class_init(MdNotebookBufItemCodeblockClass* class) {
	MdNotebookBufItemClass* bufitem = MDNOTEBOOK_BUFITEM_CLASS(class);
	bufitem->buffer_changed = mdnotebook_bufitem_codeblock_bufitem_buffer_changed;
	bufitem->on_insert = mdnotebook_bufitem_codeblock_bufitem_on_insert;
}

static void mdnotebook_bufitem_codeblock_init(_ MdNotebookBufItemCodeblock* self) {}

MdNotebookBufItem* mdnotebook_bufitem_codeblock_new() {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_CODEBLOCK, NULL);
}
