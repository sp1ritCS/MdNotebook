#define MDNOTEBOOK_BUFFER_EXPOSE_INTERNAS
#include "bufitem/mdnotebookproxbufitem.h"

#define _ __attribute__((unused))

typedef struct {
	GtkTextMark* last_position;
	GtkTextTag* invisible;
	gboolean* bufchangeptr;
} MdNotebookProxBufItemPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (MdNotebookProxBufItem, mdnotebook_proxbufitem, MDNOTEBOOK_TYPE_BUFITEM)

static void mdnotebook_proxbufitem_dispose(GObject* object) {
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(MDNOTEBOOK_PROXBUFITEM(object));

	if (priv->last_position)
		g_object_unref(priv->last_position);

	G_OBJECT_CLASS(mdnotebook_proxbufitem_parent_class)->dispose(object);
}

static void mdnotebook_proxbufitem_bufitem_registered(MdNotebookBufItem* self, MdNotebookBuffer* buffer) {
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(MDNOTEBOOK_PROXBUFITEM(self));
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(buffer);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);

	GtkTextIter cursor;
	gtk_text_buffer_get_iter_at_mark(buf, &cursor, gtk_text_buffer_get_insert(buf));
	priv->last_position = gtk_text_buffer_create_mark(buf, NULL, &cursor, true);

	GtkTextTag* invisible = gtk_text_tag_table_lookup(tagtable, "mdnb:prox:invisible");
	if (invisible)
		g_object_ref(invisible);
	else
		invisible = gtk_text_buffer_create_tag(buf, "mdnb:prox:invisible",
			"invisible", TRUE,
		NULL);
	priv->invisible = invisible;

	/*GtkTextTag* proximity = gtk_text_tag_table_lookup(tagtable, "mdnb:prox:proximity");
	if (proximity)
		g_object_ref(proximity);
	else
		proximity = gtk_text_buffer_create_tag(buf, "mdnb:prox:proximity", NULL);
	priv->proximity = proximity;*/

	if (!mdnotebook_bufitem_get_textview(self))
		g_critical("%s has no MdNotebook.View attached. This will cause issues.\n", g_type_name(G_OBJECT_TYPE(self)));
	else
		priv->bufchangeptr = mdnotebook_buffer_get_bufchange_ptr(mdnotebook_bufitem_get_buffer(self));
}

typedef struct {
	MdNotebookProxBufItem* item;
	GtkTextMark* mark;
} QueuedChildAnchor;
static void mdnotebook_proxbufitem_render_widget(MdNotebookProxBufItem* self, const GtkTextIter* start, const GtkTextIter* end);
static gboolean mdnotebook_proxbufitem_bufitem_shift_iter_accomodate_anchor(GtkTextIter* i);
static gboolean mdnotebook_proxbufitem_queue_child_anchor_cb(QueuedChildAnchor* queue) {
	MdNotebookProxBufItemClass* class = MDNOTEBOOK_PROXBUFITEM_GET_CLASS(queue->item);
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(queue->item);
	GtkTextBuffer* buf = gtk_text_mark_get_buffer(queue->mark);
	GtkTextIter start;
	gtk_text_buffer_get_iter_at_mark(buf, &start, queue->mark);

	if (!gtk_text_iter_get_child_anchor(&start)) {
		*priv->bufchangeptr = TRUE;
		gtk_text_buffer_create_child_anchor(buf, &start);
		*priv->bufchangeptr = FALSE;

		gtk_text_buffer_get_iter_at_mark(buf, &start, queue->mark);
		gtk_text_iter_forward_char(&start);
		GtkTextIter new_left,new_right;
		gboolean valid = mdnotebook_bufitem_get_tag_extends(&start, class->tag(queue->item), &new_left, &new_right);
		if (valid) {
			mdnotebook_proxbufitem_bufitem_shift_iter_accomodate_anchor(&new_left);
			mdnotebook_proxbufitem_render_widget(queue->item, &new_left, &new_right);
		}
	}

	gtk_text_buffer_delete_mark(buf, queue->mark);

	g_object_unref(queue->item);
	g_object_unref(queue->mark);
	g_free(queue);

	return G_SOURCE_REMOVE;
}

static void mdnotebook_proxbufitem_queue_child_anchor(MdNotebookProxBufItem* self, GtkTextMark* mark) {
	QueuedChildAnchor* queue = g_new(QueuedChildAnchor, 1);
	queue->item = g_object_ref(self);
	queue->mark = g_object_ref(mark);

	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_proxbufitem_queue_child_anchor_cb), queue, NULL);
	g_source_attach(s, g_main_context_default());
}

static void mdnotebook_proxbufitem_render_widget(MdNotebookProxBufItem* self, const GtkTextIter* start, const GtkTextIter* end) {
	MdNotebookProxBufItemClass* class = MDNOTEBOOK_PROXBUFITEM_GET_CLASS(self);
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(self);
	GtkTextView* view = GTK_TEXT_VIEW(mdnotebook_bufitem_get_textview(MDNOTEBOOK_BUFITEM(self)));
	GtkTextBuffer* buf = gtk_text_view_get_buffer(view);

	GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(start);
	if (!anch) {
		GtkTextMark* mstart = gtk_text_buffer_create_mark(buf, NULL, start, true);
		mdnotebook_proxbufitem_queue_child_anchor(self, mstart);
	} else {
		GtkTextIter j = *start;
		gtk_text_iter_forward_char(&j);
		gtk_text_buffer_remove_tag(buf, priv->invisible, start, &j);
		gtk_text_buffer_apply_tag(buf, priv->invisible, &j, end);

		guint num;
		GtkWidget** ws = gtk_text_child_anchor_get_widgets(anch, &num);
		if (num) {
			for (guint i = 0; i<num; i++) {
				class->update(self, ws[i], &j, end);
				gtk_widget_show(ws[i]);
				GtkTextTag* baseline = mdnotebook_buffer_get_baseline_tag(MDNOTEBOOK_BUFFER(buf), class->get_baseline(self, ws[i]));
				if (baseline)
					gtk_text_buffer_apply_tag(buf, baseline, start, &j);
			}
		} else {
			GtkWidget* b = class->render(self, &j, end);
			gtk_text_view_add_child_at_anchor(view, b, anch);
			GtkTextTag* baseline = mdnotebook_buffer_get_baseline_tag(MDNOTEBOOK_BUFFER(buf), class->get_baseline(self, b));
			if (baseline)
				gtk_text_buffer_apply_tag(buf, baseline, start, &j);
		}
	}
}
static void mdnotebook_proxbufitem_unrender_widgets(MdNotebookProxBufItem* self, const GtkTextIter* start, const GtkTextIter* end) {
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(self);
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(mdnotebook_bufitem_get_buffer(MDNOTEBOOK_BUFITEM(self)));
	GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(start);
	if (anch) {
		guint num;
		GtkWidget** ws = gtk_text_child_anchor_get_widgets(anch, &num);
		for (guint i = 0; i<num; i++) {
			gtk_widget_hide(ws[i]);
		}
		GtkTextIter j = *start;
		gtk_text_iter_forward_char(&j);
		gtk_text_buffer_apply_tag(buf, priv->invisible, start, &j);
		gtk_text_buffer_remove_tag(buf, priv->invisible, &j, end);
	}
}

const gunichar _gtk_text_unknown_char = 0xFFFC;
static gboolean mdnotebook_proxbufitem_bufitem_shift_iter_accomodate_anchor(GtkTextIter* i) {
	GtkTextIter active = *i;
	gtk_text_iter_backward_char(&active);

	if (gtk_text_iter_get_char(&active) == _gtk_text_unknown_char) {
		gtk_text_iter_backward_char(i);
		return TRUE;
	}
	return FALSE;
}
static gboolean mdnotebook_proxbufitem_bufitem_reverse_shift_iter_accomodate_anchor(GtkTextIter* i) {
	if (gtk_text_iter_get_char(i) == _gtk_text_unknown_char) {
		gtk_text_iter_forward_char(i);
		return TRUE;
	}
	return FALSE;
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
static void mdnotebook_proxbufitem_cursor_changed_remove_selection(GtkTextBuffer* buf, GParamSpec*, MdNotebookProxBufItem* self) {
	GtkTextIter ins,sb;
	gtk_text_buffer_get_iter_at_mark(buf, &ins, gtk_text_buffer_get_insert(buf));
	gtk_text_buffer_get_iter_at_mark(buf, &sb, gtk_text_buffer_get_selection_bound(buf));

	printf("(I%d|S%d)\n", gtk_text_iter_get_offset(&ins), gtk_text_iter_get_offset(&sb));

	gtk_text_buffer_move_mark(buf, gtk_text_buffer_get_selection_bound(buf), &ins);

	g_signal_handlers_disconnect_by_func(buf, (gpointer)mdnotebook_proxbufitem_cursor_changed_remove_selection, self);
	g_object_unref(self);
}

static gboolean mdnotebook_proxbufitem_bufitem_cursor_changed_cb(MdNotebookProxBufItem* self) {
	MdNotebookProxBufItemClass* class = MDNOTEBOOK_PROXBUFITEM_GET_CLASS(self);
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(self);
	MdNotebookBufItem* bufitem = MDNOTEBOOK_BUFITEM(self);
	MdNotebookView* view = mdnotebook_bufitem_get_textview(MDNOTEBOOK_BUFITEM(self));
	GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	GtkTextIter new_iter,old_iter;
	gtk_text_buffer_get_iter_at_mark(buf, &new_iter, gtk_text_buffer_get_insert(buf));
	gtk_text_buffer_get_iter_at_mark(buf, &old_iter, priv->last_position);
	GtkTextIter cursor = new_iter;

	GtkTextTag* tag = class->tag(self);

	guint keyval = mdnotebook_view_get_latest_keyval(view);

	// TODO: this is currently somehow broken when there is a single
	// char behind an equation (\$eq\$.$). I'm not in the right headspace
	// rn to debug this.

	mdnotebook_proxbufitem_bufitem_reverse_shift_iter_accomodate_anchor(&old_iter);
	mdnotebook_proxbufitem_bufitem_reverse_shift_iter_accomodate_anchor(&new_iter);

	GtkTextIter old_left,old_right,new_left,new_right;
	gboolean old_valid = mdnotebook_bufitem_get_tag_extends(&old_iter, tag, &old_left, &old_right);
	gboolean new_valid = mdnotebook_bufitem_get_tag_extends(&new_iter, tag, &new_left, &new_right);

	if (old_valid)
		mdnotebook_proxbufitem_bufitem_shift_iter_accomodate_anchor(&old_left);
	if (new_valid)
		mdnotebook_proxbufitem_bufitem_shift_iter_accomodate_anchor(&new_left);

	gboolean from_right = new_valid && gtk_text_iter_compare(&new_iter, &new_left) > 0
		&& (!keyval || keyval == GDK_KEY_Left || keyval == GDK_KEY_Up || keyval == GDK_KEY_Down);
	GtkTextMark* move_to = NULL;

	// I still don't like this; at it's current state this is very buggy
	// and broken (cannot select from first & last char in equation,
	// `<Shift>Home/End` doesn't work when behind an equation, etc.) Will
	// definitively have to implement a better solution. Just don't know
	// how yet ._.
	/*if ((old_valid && new_valid && !(gtk_text_iter_in_range(&new_iter, &old_left, &old_right)))
		 || (!old_valid && new_valid)) {
		g_signal_connect(buf, "notify::has-selection", G_CALLBACK(mdnotebook_proxbufitem_cursor_changed_remove_selection), g_object_ref(self));
	}*/

	if (old_valid) {
		if (new_valid) {
			if(gtk_text_iter_compare(&new_iter, &old_left) < 0 || gtk_text_iter_compare(&new_iter, &old_right) > 0) {
				if (from_right) {
					if (gtk_text_iter_has_tag(&cursor, priv->invisible)) {
						gtk_text_iter_forward_to_tag_toggle(&new_iter, tag);
						move_to = gtk_text_buffer_create_mark(buf, NULL, &new_iter, FALSE);
					}
				}
				/* moved into a different prox region. signal both. */
				mdnotebook_bufitem_push_iter(bufitem, &new_left);
				mdnotebook_bufitem_push_iter(bufitem, &new_right);
				mdnotebook_proxbufitem_render_widget(self, &old_left, &old_right);
				mdnotebook_bufitem_pop_iter(bufitem, &new_right);
				mdnotebook_bufitem_pop_iter(bufitem, &new_left);
				mdnotebook_proxbufitem_unrender_widgets(self, &new_left, &new_right);
			} else if (gtk_text_iter_compare(&new_iter, &old_iter) == 0) {
				/* we may have deleted right into a prox region's boundary. signal to be safe. */
				g_info("deletion enter\n");
				mdnotebook_proxbufitem_unrender_widgets(self, &new_left, &new_right);
			}
			/* otherwise, we stayed in the same one. no signals needed */
		} else {
			mdnotebook_proxbufitem_render_widget(self, &old_left, &old_right);
		}
	} else if (new_valid) {
		if (from_right) {
			if (gtk_text_iter_has_tag(&cursor, priv->invisible)) {
				gtk_text_iter_forward_to_tag_toggle(&new_iter, tag);
				move_to = gtk_text_buffer_create_mark(buf, NULL, &new_iter, FALSE);
			}
		}
		mdnotebook_proxbufitem_unrender_widgets(self, &new_left, &new_right);
	}

	if (move_to) {
		GtkTextMark
			*insert = gtk_text_buffer_get_insert(buf),
			*selection = gtk_text_buffer_get_selection_bound(buf);
		GtkTextIter t,ins,sb;
		gtk_text_buffer_get_iter_at_mark(buf, &t, move_to);
		gtk_text_buffer_get_iter_at_mark(buf, &ins, insert);
		gtk_text_buffer_get_iter_at_mark(buf, &sb, selection);

		gtk_text_buffer_move_mark(buf, insert, &t);
		if (gtk_text_iter_compare(&sb, &ins) == 0)
			gtk_text_buffer_move_mark(buf, selection, &t);

		gtk_text_buffer_delete_mark(buf, move_to);
	}

	gtk_text_buffer_move_mark(buf, priv->last_position, &new_iter);
	g_object_unref(self);
	return G_SOURCE_REMOVE;
}

static void mdnotebook_proxbufitem_bufitem_cursor_changed(MdNotebookBufItem* self, MdNotebookBuffer* buffer, _ const GtkTextIter* start, _ const GtkTextIter* end) {
	if (gtk_text_buffer_get_has_selection(GTK_TEXT_BUFFER(buffer)))
		return;

	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_proxbufitem_bufitem_cursor_changed_cb), g_object_ref(self), NULL);
	g_source_attach(s, g_main_context_default());
}

static void mdnotebook_proxbufitem_class_init(MdNotebookProxBufItemClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	MdNotebookBufItemClass* bufitem_class= MDNOTEBOOK_BUFITEM_CLASS(class);

	object_class->dispose = mdnotebook_proxbufitem_dispose;
	bufitem_class->registered = mdnotebook_proxbufitem_bufitem_registered;
	bufitem_class->cursor_changed = mdnotebook_proxbufitem_bufitem_cursor_changed;
}

static void mdnotebook_proxbufitem_init(MdNotebookProxBufItem* self) {
	MdNotebookProxBufItemPrivate* priv = mdnotebook_proxbufitem_get_instance_private(self);
	priv->last_position = NULL;
	priv->bufchangeptr = NULL;
}


static gboolean mdnotebook_proxbufitem_queue_child_removal_cb(GtkTextMark* mark) {
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
static void mdnotebook_proxbufitem_queue_child_removal(GtkTextMark* mark) {
	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_proxbufitem_queue_child_removal_cb), mark, NULL);
	g_source_attach(s, g_main_context_default());
}
static void mdnotebook_proxbufitem_cleanup(MdNotebookProxBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	g_return_if_fail(MDNOTEBOOK_IS_PROXBUFITEM(self));
	MdNotebookProxBufItemClass* class = MDNOTEBOOK_PROXBUFITEM_GET_CLASS(self);

	GtkTextIter active = *start;
	while (gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_check_char, (gpointer)0xFFFC, end)) {
		GtkTextIter fwd = active;
		gtk_text_iter_forward_char(&fwd);
		if (gtk_text_iter_starts_tag(&fwd, class->tag(self)))
			continue;

		// if anchor is irrelevant to LaTeX -> continue
		GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&active);
		if (!anch)
			continue;
		guint num;
		GtkWidget** widgets = gtk_text_child_anchor_get_widgets(anch, &num);
		if (!num)
			continue;
		gboolean valid = FALSE;
		for (guint i = 0; i<num; i++) {
			valid = valid || class->test_widget(self, widgets[0]);
		}
		if (!valid)
			continue;


		GtkTextMark* mark = gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(buffer), NULL, &active, TRUE);
		mdnotebook_proxbufitem_queue_child_removal(mark);

		// if there are (for whatever reason) multiple invalid nodes, the next one will be
		// cleared once the first one has been removed (which will trigger this function again,
		// and so forth).
		//break;
	}
}

typedef struct {
	GtkTextMark* begin;
	GtkTextMark* end;
} MarkerCombo;

typedef struct {
	MdNotebookProxBufItem* self;
	GSList* renderlist;
} ChangedQueue;

#define CAST_GFUNC(f) ((GFunc) (f))
static void mdnotebook_proxbufitem_render_outstanding(MarkerCombo* cmb, MdNotebookProxBufItem* self) {
	if (!cmb)
		return;
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(mdnotebook_bufitem_get_buffer(MDNOTEBOOK_BUFITEM(self)));
	GtkTextIter left,right;
	gtk_text_buffer_get_iter_at_mark(buf, &left, cmb->begin);
	gtk_text_buffer_get_iter_at_mark(buf, &right, cmb->end);

	mdnotebook_proxbufitem_render_widget(self, &left, &right);

	gtk_text_buffer_delete_mark(buf, cmb->begin);
	gtk_text_buffer_delete_mark(buf, cmb->end);

	g_free(cmb);
}


static gboolean mdnotebook_proxbufitem_bufitem_items_changed_cb(ChangedQueue* queue) {
	g_slist_foreach(queue->renderlist, CAST_GFUNC(mdnotebook_proxbufitem_render_outstanding), queue->self);
	g_slist_free(g_steal_pointer(&queue->renderlist));
	g_object_unref(queue->self);
	g_free(queue);
	return G_SOURCE_REMOVE;
}

void mdnotebook_proxbufitem_bufitem_items_changed(MdNotebookProxBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	g_return_if_fail(MDNOTEBOOK_IS_PROXBUFITEM(self));
	MdNotebookProxBufItemClass* class = MDNOTEBOOK_PROXBUFITEM_GET_CLASS(self);
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(buffer);

	GtkTextTag* tag = class->tag(MDNOTEBOOK_PROXBUFITEM(self));
	if (!tag)
		return;

	mdnotebook_proxbufitem_cleanup(self, buffer, start, end);

	GSList* renderlist = g_slist_alloc();
	GSList* active_renderlist_node = renderlist;

	GtkTextIter i = *start,next;
	while (gtk_text_iter_forward_to_tag_toggle(&i, tag)) {
		if (gtk_text_iter_compare(&i, end) > 0) break;

		next = i;
		if (!gtk_text_iter_forward_to_tag_toggle(&next, tag)) next = *end;

		GtkTextIter cursor;
		gtk_text_buffer_get_iter_at_mark(buf, &cursor, gtk_text_buffer_get_insert(buf));
		if (gtk_text_iter_compare(&cursor, &i) < 0 || gtk_text_iter_compare(&cursor, &next) > 0) {
			if (mdnotebook_proxbufitem_bufitem_shift_iter_accomodate_anchor(&i))
				goto next_tag;

			MarkerCombo* cmb = g_new(MarkerCombo, 1);
			cmb->begin = gtk_text_buffer_create_mark(buf, NULL, &i, TRUE);
			cmb->end = gtk_text_buffer_create_mark(buf, NULL, &next, FALSE);

			active_renderlist_node->data = cmb;
			GSList* new = g_slist_alloc();
			active_renderlist_node->next = new;
			active_renderlist_node = new;
		}

next_tag:
		i = next;
	}

	ChangedQueue* ud = g_new(ChangedQueue, 1);
	ud->self = g_object_ref(MDNOTEBOOK_PROXBUFITEM(self));
	ud->renderlist = renderlist;

	GSource* s = g_idle_source_new();
	g_source_set_callback(s, G_SOURCE_FUNC(mdnotebook_proxbufitem_bufitem_items_changed_cb), ud, NULL);
	g_source_attach(s, g_main_context_default());
}

gboolean mdnotebook_proxbufitem_test_iter_has_widget(const GtkTextIter* i) {
	GtkTextIter active = *i;
	gtk_text_iter_backward_char(&active);
	return gtk_text_iter_get_char(&active) == _gtk_text_unknown_char;
}
