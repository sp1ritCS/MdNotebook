#include "bufitem/mdnotebookbufitemdynblock.h"

#define _ __attribute__((unused))

G_DEFINE_TYPE(MdNotebookBufItemDynBlock, mdnotebook_bufitem_dynblock, MDNOTEBOOK_TYPE_BUFITEM)

typedef struct {
	gint linenum;
	GtkTextIter gt;
} QuoteNode;

static void mdnotebook_bufitem_dynblock_bufitem_buffer_changed(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter active = *start;

	GtkTextTag* quotetag = gtk_text_tag_table_lookup(tagtable, "mdquotetag");
	if (!quotetag)
		quotetag = gtk_text_buffer_create_tag(buf, "mdquotetag",
				"paragraph-background", "#E2E2E2",
				"left-margin", 32,
				"right-margin", 64,
				"pixels-above-lines", 4,
				"pixels-below-lines", 4,
			NULL);
	GtkTextTag* quotelntag = gtk_text_tag_table_lookup(tagtable, "mdquotelntag");
	if (!quotelntag)
		quotelntag = gtk_text_buffer_create_tag(buf, "mdquotelntag",
				"background", "#808595",
				"foreground", "#808595",
				"size", (4 * PANGO_SCALE),
			NULL);

	gtk_text_buffer_remove_tag(buf, quotetag, start, end);
	gtk_text_buffer_remove_tag(buf, quotelntag, start, end);

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_check_char, (gpointer)'>', end)) {
		if (mdnotebook_bufitem_is_iter_in_private(self, &active))
			continue;
		if (!mdnotebook_bufitem_check_backward_whitespace(&active))
			continue;

		GtkTextIter aftr_quote = active;
		gtk_text_iter_forward_char(&aftr_quote);
		gtk_text_buffer_apply_tag(buf, quotelntag, &active, &aftr_quote);

		GtkTextIter
			beginqt = active,
			endqt = active;
		gtk_text_iter_set_line_offset(&beginqt, 0);
		gtk_text_iter_forward_to_line_end(&endqt);

		gtk_text_buffer_apply_tag(buf, quotetag, &beginqt, &endqt);
	}
}

static gboolean insert_handle_fwd(gunichar c, gpointer clause) {
	if (clause) {
		return (c != ' ' && c != '\t') || c == '\n';
	} else {
		return c == ' ' || c == '\t' || c == '\n';
	}
}

static void mdnotebook_bufitem_dynblock_bufitem_on_insert(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, GtkTextMark* location, gchar* text, gint) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_mark(buf, &iter, location);
	if(g_strcmp0(text, "\n") == 0) {
		if (mdnotebook_bufitem_is_iter_in_private(self, &iter)) return;
		// TODO: implement reference to MdNotebook.View
		//if(self->modifier_keys & GDK_SHIFT_MASK) return;
		if(!gtk_text_iter_ends_line(&iter)) return;

		/* extract previous line's first word and indentation preceding it */
		GtkTextIter start = iter; gtk_text_iter_backward_line(&start);
		GtkTextIter end = start;
		gunichar end_char = gtk_text_iter_get_char(&end);
		if(end_char == ' ' || end_char == '\t') gtk_text_iter_forward_find_char(&end, insert_handle_fwd, (gpointer)true, NULL);
		gtk_text_iter_forward_find_char(&end, insert_handle_fwd, (gpointer)false, NULL);
		gtk_text_iter_forward_char(&end);
		gchar* str = gtk_text_buffer_get_text(buf, &start, &end, true);
		gtk_text_iter_forward_char(&end);

		int num=0,pad=0,len=0;
		//printf("word: %s\n",str);

		/* count indentation spaces, then eat them */
		sscanf(str," %n",&pad);
		str = g_utf8_substring(str, pad, g_utf8_strlen(str, -1));

		//printf("word: %s\n",str);

		/* try to see if we have any valid markdown enumeration we could extend */
		sscanf(str,"%d.%*1[ \t]%n",&num,&len);
		char sbuf[512];
		if(len==g_utf8_strlen(str, -1) && num>0) {
			if(gtk_text_iter_compare(&end, &iter) >= 0) {
				gtk_text_buffer_delete(buf, &start, &iter);
				return;
			}
			sprintf(sbuf,"%*s%d. ",pad,"",num+1);
			gtk_text_buffer_insert(buf, &iter, sbuf, -1);
		} else if(g_strcmp0(str, "* ") == 0) {
			if(gtk_text_iter_compare(&end, &iter) >= 0) {
				gtk_text_buffer_delete(buf, &start, &iter);
				return;
			}
			sprintf(sbuf,"%*s* ",pad,"");
			gtk_text_buffer_insert(buf, &iter, sbuf, -1);
		} else if(g_strcmp0(str, "- ") == 0) {
			if(gtk_text_iter_compare(&end, &iter) >= 0) {
				gtk_text_buffer_delete(buf, &start, &iter);
				return;
			}
			sprintf(sbuf,"%*s- ",pad,"");
			gtk_text_buffer_insert(buf, &iter, sbuf, -1);
		} else if(g_strcmp0(str, "> ") == 0) {
			if(gtk_text_iter_compare(&end, &iter) == 0) {
				gtk_text_buffer_delete(buf, &start, &iter);
				return;
			}
			sprintf(sbuf,"%*s> ",pad,"");
			gtk_text_buffer_insert(buf, &iter, sbuf, -1);
		}
	}
}

static void mdnotebook_bufitem_dynblock_class_init(_ MdNotebookBufItemDynBlockClass* class) {
	MdNotebookBufItemClass* bufitem = MDNOTEBOOK_BUFITEM_CLASS(class);

	bufitem->buffer_changed = mdnotebook_bufitem_dynblock_bufitem_buffer_changed;
	bufitem->on_insert = mdnotebook_bufitem_dynblock_bufitem_on_insert;
}

static void mdnotebook_bufitem_dynblock_init(_ MdNotebookBufItemDynBlock* self) {}

MdNotebookBufItem* mdnotebook_bufitem_dynblock_new() {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_DYNBLOCK, NULL);
}
