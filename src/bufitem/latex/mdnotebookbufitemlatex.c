#include "bufitem/latex/mdnotebookbufitemlatex.h"

#define _ __attribute__((unused))

static void mdnotebook_bufitem_latex_bufitem_iface_init(MdNotebookBufItemInterface* iface);

typedef struct {
	MdNotebookView* view;
} MdNotebookBufItemLatexPrivate;

G_DEFINE_TYPE_WITH_CODE(MdNotebookBufItemLatex, mdnotebook_bufitem_latex, G_TYPE_OBJECT,
	G_ADD_PRIVATE (MdNotebookBufItemLatex)
	G_IMPLEMENT_INTERFACE(MDNOTEBOOK_TYPE_BUFITEM, mdnotebook_bufitem_latex_bufitem_iface_init))

enum {
	PROP_TEXTVIEW = 1,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_bufitem_latex_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookBufItemLatex* self = MDNOTEBOOK_BUFITEM_LATEX(object);

	switch (prop_id) {
		case PROP_TEXTVIEW:
			g_value_set_object(value, mdnotebook_bufitem_latex_get_textview(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

}

static void mdnotebook_bufitem_latex_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookBufItemLatex* self = MDNOTEBOOK_BUFITEM_LATEX(object);

	switch (prop_id) {
		case PROP_TEXTVIEW:
			mdnotebook_bufitem_latex_set_textview(self, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}
static void mdnotebook_bufitem_latex_dispose(GObject* object) {
	MdNotebookBufItemLatexPrivate* priv = mdnotebook_bufitem_latex_get_instance_private(MDNOTEBOOK_BUFITEM_LATEX(object));

	if (priv->view)
		g_object_unref(priv->view);

	G_OBJECT_CLASS(mdnotebook_bufitem_latex_parent_class)->dispose(object);
}

static void mdnotebook_bufitem_latex_bufitem_cursor_changed(_ MdNotebookBufItem* /* if used, you need update mdnotebook_bufitem_latex_queue_child_anchor_cb as this calls this method with iface set to NULL */,
															MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter cursor;
	GtkTextTag *latextag,*hiddentag;
	GtkTextIter active = *start;

	gtk_text_buffer_get_iter_at_mark(buf, &cursor, gtk_text_buffer_get_insert(buf));

	latextag = gtk_text_tag_table_lookup(tagtable, "mdlatex");
	if (!latextag)
		return;

	hiddentag = gtk_text_tag_table_lookup(tagtable, "mdlatexhidden");
	if (!hiddentag)
		hiddentag = gtk_text_buffer_create_tag(buf, "mdlatexhidden",
			//"background", "red",
			"invisible", TRUE,
		NULL);

	gtk_text_buffer_remove_tag(buf, hiddentag, start, end);

	while (gtk_text_iter_forward_to_tag_toggle(&active, latextag)) {
		if (gtk_text_iter_starts_tag(&active, latextag)) {
			GtkTextIter anchor = active, begin = active;
			gtk_text_iter_backward_char(&anchor);

			if (!gtk_text_iter_forward_to_tag_toggle(&active, latextag))
				break;

			GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&anchor);

			if (!anch)
				continue;
			guint num;
			GtkWidget** widgets = gtk_text_child_anchor_get_widgets(anch, &num);
			if (!num || !MDNOTEBOOK_IS_LATEX_EQUATION(widgets[0]))
				continue;

			if (gtk_text_iter_in_range(&cursor, &anchor, &active)) {
				gtk_widget_hide(widgets[0]);
			} else {
				gtk_widget_show(widgets[0]);
				gtk_text_buffer_apply_tag(buf, hiddentag, &begin, &active);
			}
		}
	}
}

static bool mdnotebook_bufitem_latex_test_escaped(const GtkTextIter* ch) {
	GtkTextIter active = *ch;
	g_return_val_if_fail(ch, FALSE);

	gtk_text_iter_backward_char(&active);
	return gtk_text_iter_get_char(&active) == '\\';
}

static gboolean mdnotebook_bufitem_latex_queue_child_anchor_cb(GtkTextMark* mark) {
	GtkTextBuffer* buf = gtk_text_mark_get_buffer(mark);
	GtkTextIter start;
	gtk_text_buffer_get_iter_at_mark(buf, &start, mark);

	if (!gtk_text_iter_get_child_anchor(&start))
		gtk_text_buffer_create_child_anchor(buf, &start);

	gtk_text_buffer_delete_mark(buf, mark);

	GtkTextIter bufstart,bufend;
	gtk_text_buffer_get_start_iter(buf, &bufstart);
	gtk_text_buffer_get_end_iter(buf, &bufend);
	mdnotebook_bufitem_latex_bufitem_cursor_changed(NULL, MDNOTEBOOK_BUFFER(buf), &bufstart, &bufend);

	return G_SOURCE_REMOVE;
}

static void mdnotebook_bufitem_latex_queue_child_anchor(MdNotebookBuffer* self, GtkTextMark* mark) {
	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_bufitem_latex_queue_child_anchor_cb), mark, NULL);
	g_source_attach(s, g_main_context_default());
}

const char _gtk_text_unknown_char_utf8[] = { '\xEF', '\xBF', '\xBC', '\0' };

static gboolean mdnotebook_bufitem_latex_queue_child_removal_cb(GtkTextMark* mark) {
	GtkTextBuffer* buf = gtk_text_mark_get_buffer(mark);
	GtkTextIter start,end;
	gtk_text_buffer_get_iter_at_mark(buf, &start, mark);
	end = start;
	gtk_text_iter_forward_char(&end);

	if (gtk_text_iter_get_child_anchor(&start))
		gtk_text_buffer_delete(buf, &start, &end);

	gtk_text_buffer_delete_mark(buf, mark);

	return G_SOURCE_REMOVE;
}
static void mdnotebook_bufitem_latex_queue_child_removal(MdNotebookBuffer* self, GtkTextMark* mark) {
	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_bufitem_latex_queue_child_removal_cb), mark, NULL);
	g_source_attach(s, g_main_context_default());
}

void mdnotebook_latex_equation_view_resized(MdNotebookView* view, gint width, MdNotebookLatexEquation* eq);

static void mdnotebook_bufitem_latex_apply_dollar_items(MdNotebookBufItemLatex* latex, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextTag* privatetag = mdnotebook_bufitem_get_private_tag(self);
	GtkTextIter active = *start;

	GtkTextTag* latextag = gtk_text_tag_table_lookup(tagtable, "mdlatex");
	if (!latextag)
		latextag = gtk_text_buffer_create_tag(buf, "mdlatex",
			"foreground", "#A0A8C0",
		NULL);

	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_check_char, (gpointer)'$', end)) {
		if (mdnotebook_bufitem_is_iter_in_private(self, &active))
			continue;
		if (mdnotebook_bufitem_latex_test_escaped(&active))
			continue;

		GtkTextIter latex_begin = active;

		guint level = 0;
		while (gtk_text_iter_get_char(&active) == '$' && level < 2) {
			level++; gtk_text_iter_forward_char(&active);
		}

		gint valid_latex = FALSE;

		while (mdnotebook_bufitem_is_iter_in_private(self, &active) || !valid_latex) {
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

			GtkTextIter anchor = latex_begin;
			gtk_text_iter_backward_char(&anchor);
			GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&anchor);
			if (!anch) {
				GtkTextMark* mark = gtk_text_buffer_create_mark(buf, NULL, &latex_begin, TRUE);
				mdnotebook_bufitem_latex_queue_child_anchor(self, mark);
			} else {
				gchar* equation = gtk_text_iter_get_text(&latex_begin, &active);
				guint num;
				GtkWidget** widgets = gtk_text_child_anchor_get_widgets(anch, &num);
				if (num) {
					if (MDNOTEBOOK_IS_LATEX_EQUATION(widgets[0]))
						mdnotebook_latex_equation_set_equation(MDNOTEBOOK_LATEX_EQUATION(widgets[0]), equation);
				} else {
					MdNotebookView* view = mdnotebook_bufitem_latex_get_textview(latex);
					GtkWidget* b = mdnotebook_latex_equation_new(view);
					g_signal_connect(view, "horizontal-resize", G_CALLBACK(mdnotebook_latex_equation_view_resized), b);
					gtk_widget_hide(b);
					gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(view), b, anch);
					mdnotebook_latex_equation_set_equation(MDNOTEBOOK_LATEX_EQUATION(b), equation);
				}
				g_free(equation);
			}
		}
	}

	active = *start;
	GtkTextIter before;
	while (gtk_text_iter_forward_search(&active, _gtk_text_unknown_char_utf8, GTK_TEXT_SEARCH_VISIBLE_ONLY, &before, &active, end)) {
		if (gtk_text_iter_starts_tag(&active, latextag))
			continue;

		// if anchor is irrelevant to LaTeX -> continue
		GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&before);
		if (!anch)
			continue;
		guint num;
		GtkWidget** widgets = gtk_text_child_anchor_get_widgets(anch, &num);
		if (!num)
			continue;
		if (num && !MDNOTEBOOK_IS_LATEX_EQUATION(widgets[0]))
			continue;

		GtkTextMark* mark = gtk_text_buffer_create_mark(buf, NULL, &before, TRUE);
		mdnotebook_bufitem_latex_queue_child_removal(self, mark);

		// if there are (for whatever reason) multiple invalid nodes, the next one will be
		// cleared once the first one has been removed (which will trigger this function again,
		// and so forth).
		break;
	}
}

static void mdnotebook_bufitem_latex_bufitem_buffer_changed(MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextTag* latextag = gtk_text_tag_table_lookup(tagtable, "mdlatex");
	if (latextag)
		gtk_text_buffer_remove_tag(buf, latextag, start, end);

	mdnotebook_bufitem_latex_apply_dollar_items(MDNOTEBOOK_BUFITEM_LATEX(iface), self, start, end);
}

static void mdnotebook_bufitem_latex_class_init(MdNotebookBufItemLatexClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	object_class->get_property = mdnotebook_bufitem_latex_get_property;
	object_class->set_property = mdnotebook_bufitem_latex_set_property;
	object_class->dispose = mdnotebook_bufitem_latex_dispose;

	obj_properties[PROP_TEXTVIEW] = g_param_spec_object("textview", "TextView", "The Gtk.TextView this BufItem will be rendering in", MDNOTEBOOK_TYPE_VIEW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}
static void mdnotebook_bufitem_latex_bufitem_iface_init(MdNotebookBufItemInterface* iface) {
	iface->cursor_changed = mdnotebook_bufitem_latex_bufitem_cursor_changed;
	iface->buffer_changed = mdnotebook_bufitem_latex_bufitem_buffer_changed;
}

static void mdnotebook_bufitem_latex_init(MdNotebookBufItemLatex* self) {
	MdNotebookBufItemLatexPrivate* priv = mdnotebook_bufitem_latex_get_instance_private(self);

	priv->view = NULL;

	// TODO: move to iface init
	mdnotebook_latex_equation_init_microtex();
}

MdNotebookBufItem* mdnotebook_bufitem_latex_new(MdNotebookView* textview) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_LATEX, "textview", textview, NULL);
}


MdNotebookView* mdnotebook_bufitem_latex_get_textview(MdNotebookBufItemLatex* self) {
	MdNotebookBufItemLatexPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_BUFITEM_LATEX(self), NULL);

	priv = mdnotebook_bufitem_latex_get_instance_private(self);

	return priv->view;
}

void mdnotebook_bufitem_latex_set_textview(MdNotebookBufItemLatex* self, MdNotebookView* view) {
	MdNotebookBufItemLatexPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM_LATEX(self));
	g_return_if_fail(view == NULL || MDNOTEBOOK_IS_VIEW(view));

	priv = mdnotebook_bufitem_latex_get_instance_private(self);

	if (priv->view == view)
		return;

	priv->view = g_object_ref(view);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXTVIEW]);
}
