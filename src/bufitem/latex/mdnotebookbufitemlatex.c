#include "bufitem/latex/mdnotebookbufitemlatex.h"

#define _ __attribute__((unused))

static void mdnotebook_bufitem_latex_bufitem_iface_init(MdNotebookBufItemInterface* iface);

typedef struct {
	MdNotebookView* view;
	GtkTextMark* last_position;
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

static void mdnotebook_bufitem_latex_bufitem_init(MdNotebookBufItem* iface, MdNotebookBuffer* self) {
	MdNotebookBufItemLatexPrivate* priv = mdnotebook_bufitem_latex_get_instance_private(MDNOTEBOOK_BUFITEM_LATEX(iface));
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextIter cursor;
	gtk_text_buffer_get_iter_at_mark(buf, &cursor, gtk_text_buffer_get_insert(buf));
	GtkTextMark* last_pos = gtk_text_buffer_create_mark(buf, "mdlatexlast_position", &cursor, true);

	priv->last_position = last_pos;

	mdnotebook_latex_equation_init_microtex();
}
// work arround the equation code being partially selected after expanding the
// widget, but only after the cursor_changed method has completed. This
// function is connected to "notify::selection-changed" if the new cursor
// position is inside the `mdlatex` tag. Once the signal fires, the following
// will be executed, removing the selection by setting it to the insert
// location. Unlike NoteKit, this means that the cursor position will ultimatly
// be where the user clicked and not at the beginning or end of the equation
// code.
// Once the selection was remove, it automatally disconnects itself again.
static void mdnotebook_bufitem_latex_cursor_changed_remove_selection(GtkTextBuffer* buf, GParamSpec*, GtkTextTag* tag) {
	GtkTextIter ins,sb;
	gtk_text_buffer_get_iter_at_mark(buf, &ins, gtk_text_buffer_get_insert(buf));
	gtk_text_buffer_get_iter_at_mark(buf, &sb, gtk_text_buffer_get_selection_bound(buf));

	if (gtk_text_iter_has_tag(&sb, tag))
		goto unref;

	gtk_text_buffer_move_mark(buf, gtk_text_buffer_get_selection_bound(buf), &ins);
unref:
	g_signal_handlers_disconnect_by_func(buf, (gpointer)mdnotebook_bufitem_latex_cursor_changed_remove_selection, tag);
	g_object_unref(tag);
}
static void mdnotebook_bufitem_latex_bufitem_cursor_changed(MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
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

			// ensure that this also shows the equation code and since gtk_text_iter_in_range
			// does not return true when the iter is at the end of the range, and I cannot
			// forward iters over the end of the buffer, I have to work arround this like so.
			gint delta = gtk_text_iter_get_offset(&cursor) - gtk_text_iter_get_offset(&active);
			if (gtk_text_iter_in_range(&cursor, &anchor, &active) || delta == 0) {
				gtk_widget_hide(widgets[0]);
				gtk_text_buffer_apply_tag(buf, hiddentag, &anchor, &begin);
			} else {
				gtk_widget_show(widgets[0]);
				gtk_text_buffer_apply_tag(buf, hiddentag, &begin, &active);
			}
		}
	}

	MdNotebookBufItemLatexPrivate* priv = mdnotebook_bufitem_latex_get_instance_private(MDNOTEBOOK_BUFITEM_LATEX(iface));
	GtkTextIter old_iter,new_iter;
	gtk_text_buffer_get_iter_at_mark(buf, &old_iter, priv->last_position);
	gtk_text_buffer_get_iter_at_mark(buf, &new_iter, gtk_text_buffer_get_insert(buf));

	GtkTextIter new_left,new_right;
	gboolean old_has_latex = gtk_text_iter_has_tag(&old_iter, latextag) || gtk_text_iter_ends_tag(&old_iter, latextag);
	gboolean new_valid = mdnotebook_bufitem_get_tag_extends(&new_iter, latextag, &new_left, &new_right);
	gboolean from_right = new_valid && !old_has_latex && gtk_text_iter_compare(&old_iter, &new_iter) > 0;

	if (from_right) {
		GtkTextIter ins,sb;

		// wierd workarround to the Gtk.TextBuffer moving the cursor to the wrong spot; this might be my bug intorduced above
		gint delta = gtk_text_iter_get_offset(&new_iter) - gtk_text_iter_get_offset(&new_left);
		gtk_text_iter_forward_to_tag_toggle(&new_iter, latextag);
		gtk_text_iter_forward_chars(&new_iter, delta);

		gtk_text_buffer_get_iter_at_mark(buf, &ins, gtk_text_buffer_get_insert(buf));
		gtk_text_buffer_get_iter_at_mark(buf, &sb, gtk_text_buffer_get_selection_bound(buf));

		gtk_text_buffer_move_mark_by_name(buf, "insert", &new_iter);
		if (gtk_text_iter_compare(&sb, &ins) == 0)
			gtk_text_buffer_move_mark_by_name(buf, "selection_bound", &new_iter);
	}

	// I still don't like this; at it's current state this is very buggy
	// and broken (cannot select from last char in equation, `<Shift>Home`
	// doesn't work when behind an equation, etc.) Will definitively have
	// to implement a better solution. Just don't know how yet ._.
	if (new_valid)
		g_signal_connect(self, "notify::has-selection", G_CALLBACK(mdnotebook_bufitem_latex_cursor_changed_remove_selection), g_object_ref(latextag));

	gtk_text_buffer_move_mark(buf, priv->last_position, &cursor);
}

static bool mdnotebook_bufitem_latex_test_escaped(const GtkTextIter* ch) {
	GtkTextIter active = *ch;
	g_return_val_if_fail(ch, FALSE);

	gtk_text_iter_backward_char(&active);
	return gtk_text_iter_get_char(&active) == '\\';
}

typedef struct {
	MdNotebookBufItemLatex* item;
	GtkTextMark* mark;
} QueuedChildAnchor;

static gboolean mdnotebook_bufitem_latex_queue_child_anchor_cb(QueuedChildAnchor* queue) {
	GtkTextBuffer* buf = gtk_text_mark_get_buffer(queue->mark);
	GtkTextIter start;
	gtk_text_buffer_get_iter_at_mark(buf, &start, queue->mark);

	if (!gtk_text_iter_get_child_anchor(&start))
		gtk_text_buffer_create_child_anchor(buf, &start);

	gtk_text_buffer_delete_mark(buf, queue->mark);

	GtkTextIter bufstart,bufend;
	gtk_text_buffer_get_start_iter(buf, &bufstart);
	gtk_text_buffer_get_end_iter(buf, &bufend);
	mdnotebook_bufitem_latex_bufitem_cursor_changed(MDNOTEBOOK_BUFITEM(queue->item), MDNOTEBOOK_BUFFER(buf), &bufstart, &bufend);

	g_object_unref(queue->mark);
	g_object_unref(queue->item);
	g_free(queue);

	return G_SOURCE_REMOVE;
}

static void mdnotebook_bufitem_latex_queue_child_anchor(MdNotebookBufItemLatex* item, GtkTextMark* mark) {
	QueuedChildAnchor* queue = g_new(QueuedChildAnchor, 1);
	queue->item = g_object_ref(item);
	queue->mark = g_object_ref(mark);

	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_bufitem_latex_queue_child_anchor_cb), queue, NULL);
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
static void mdnotebook_bufitem_latex_queue_child_removal(GtkTextMark* mark) {
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
				mdnotebook_bufitem_latex_queue_child_anchor(latex, mark);
				return;
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
		mdnotebook_bufitem_latex_queue_child_removal(mark);

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
	iface->init = mdnotebook_bufitem_latex_bufitem_init;
	iface->cursor_changed = mdnotebook_bufitem_latex_bufitem_cursor_changed;
	iface->buffer_changed = mdnotebook_bufitem_latex_bufitem_buffer_changed;
}

static void mdnotebook_bufitem_latex_init(MdNotebookBufItemLatex* self) {
	MdNotebookBufItemLatexPrivate* priv = mdnotebook_bufitem_latex_get_instance_private(self);

	priv->view = NULL;
	priv->last_position = NULL;
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
