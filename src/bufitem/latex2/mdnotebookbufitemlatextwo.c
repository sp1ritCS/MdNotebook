#include "bufitem/latex2/mdnotebookbufitemlatextwo.h"

#define _ __attribute__((unused))


G_DEFINE_TYPE (MdNotebookBufItemLatexTwo, mdnotebook_bufitem_latex_two, MDNOTEBOOK_TYPE_PROXBUFITEM)

static void mdnotebook_bufitem_latex_two_bufitem_registered(MdNotebookBufItem* self, MdNotebookBuffer* buffer) {
	mdnotebook_latex_equation_init_microtex();

	MDNOTEBOOK_BUFITEM_CLASS(mdnotebook_bufitem_latex_two_parent_class)->registered(self, buffer);
}

static bool mdnotebook_bufitem_latex_two_test_escaped(const GtkTextIter* ch) {
	GtkTextIter active = *ch;
	g_return_val_if_fail(ch, FALSE);

	gtk_text_iter_backward_char(&active);
	return gtk_text_iter_get_char(&active) == '\\';
}

static void mdnotebook_bufitem_latex_two_apply_dollar_items(_ MdNotebookBufItemLatexTwo* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(buffer);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextTag* privatetag = mdnotebook_bufitem_get_private_tag(buffer);
	GtkTextIter active = *start;

	GtkTextTag* latextag = gtk_text_tag_table_lookup(tagtable, "mdlatextwo");
	if (!latextag)
		latextag = gtk_text_buffer_create_tag(buf, "mdlatextwo",
			"foreground", "#A0A8C0",
			"weight", PANGO_WEIGHT_BOLD,
		NULL);

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_check_char, (gpointer)'$', end)) {
		if (mdnotebook_bufitem_is_iter_in_private(buffer, &active))
			continue;
		if (mdnotebook_bufitem_latex_two_test_escaped(&active))
			continue;

		GtkTextIter latex_begin = active;

		guint level = 0;
		while (gtk_text_iter_get_char(&active) == '$' && level < 2) {
			level++; gtk_text_iter_forward_char(&active);
		}

		gint valid_latex = FALSE;

		while (mdnotebook_bufitem_is_iter_in_private(buffer, &active) || !valid_latex) {
			if (!gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_check_char, (gpointer)'$', end)) {
				valid_latex = FALSE;
				break;
			}

			guint endlevel = 0;
			while (gtk_text_iter_get_char(&active) == '$' && endlevel < 2) {
				endlevel++; gtk_text_iter_forward_char(&active);
			}

			if (level == endlevel) {
				valid_latex = TRUE;
				break;
			}
		}

		if (valid_latex) {
			gtk_text_buffer_apply_tag(buf, latextag, &latex_begin, &active);
			gtk_text_buffer_apply_tag(buf, privatetag, &latex_begin, &active);
		}
	}
}

static void mdnotebook_bufitem_latex_two_bufitem_buffer_changed(MdNotebookBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(buffer);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextTag* latextag = gtk_text_tag_table_lookup(tagtable, "mdlatextwo");
	if (latextag)
		gtk_text_buffer_remove_tag(buf, latextag, start, end);

	mdnotebook_bufitem_latex_two_apply_dollar_items(MDNOTEBOOK_BUFITEM_LATEX_TWO(self), buffer, start, end);

	MDNOTEBOOK_BUFITEM_CLASS(mdnotebook_bufitem_latex_two_parent_class)->buffer_changed(self, buffer, start, end);
}

static GtkTextTag* mdnotebook_bufitem_latex_two_proxbufitem_tag(MdNotebookProxBufItem* self) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(mdnotebook_bufitem_get_buffer(MDNOTEBOOK_BUFITEM(self)));
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextTag* latextag = gtk_text_tag_table_lookup(tagtable, "mdlatextwo");
	if (!latextag)
		latextag = gtk_text_buffer_create_tag(buf, "mdlatextwo",
			"foreground", "#A0A8C0",
			"weight", PANGO_WEIGHT_BOLD,
		NULL);

	return latextag;
}

static GtkWidget* mdnotebook_bufitem_latex_two_proxbufitem_render(MdNotebookProxBufItem* self, const GtkTextIter* begin, const GtkTextIter* end) {
	MdNotebookView* view = mdnotebook_bufitem_get_textview(MDNOTEBOOK_BUFITEM(self));
	gchar* str = gtk_text_iter_get_text(begin, end);
	GtkWidget* eq = mdnotebook_latex_equation_new(view);
	mdnotebook_latex_equation_set_equation(MDNOTEBOOK_LATEX_EQUATION(eq), str);
	g_free(str);

	return eq;
}

static void mdnotebook_bufitem_latex_two_proxbufitem_update(_ MdNotebookProxBufItem* self, GtkWidget* render, const GtkTextIter* begin, const GtkTextIter* end) {
	g_return_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(render));
	gchar* str = gtk_text_iter_get_text(begin, end);
	mdnotebook_latex_equation_set_equation(MDNOTEBOOK_LATEX_EQUATION(render), str);
	g_free(str);
}

static void mdnotebook_bufitem_latex_two_class_init(MdNotebookBufItemLatexTwoClass* class) {
	MdNotebookBufItemClass* bufitem_class = MDNOTEBOOK_BUFITEM_CLASS(class);
	MdNotebookProxBufItemClass* proxbufitem_class = MDNOTEBOOK_PROXBUFITEM_CLASS(class);

	bufitem_class->registered = mdnotebook_bufitem_latex_two_bufitem_registered;
	bufitem_class->buffer_changed = mdnotebook_bufitem_latex_two_bufitem_buffer_changed;

	proxbufitem_class->tag = mdnotebook_bufitem_latex_two_proxbufitem_tag;
	proxbufitem_class->render = mdnotebook_bufitem_latex_two_proxbufitem_render;
	proxbufitem_class->update = mdnotebook_bufitem_latex_two_proxbufitem_update;
}

static void mdnotebook_bufitem_latex_two_init(_ MdNotebookBufItemLatexTwo* self) {}

MdNotebookBufItem* mdnotebook_bufitem_latex_two_new(MdNotebookView* textview) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_LATEX_TWO, "textview", textview, NULL);
}
