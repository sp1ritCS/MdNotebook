#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "booktool/mdnotebookbooktoolselect.h"

#define _ __attribute__((unused))

typedef struct {
	MdNotebookBoundDrawingSelectionNode* nodes;
	gsize num_nodes;
	gsize alloc_nodes;

	gboolean in_gesture;
} MdNotebookBookToolSelectPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MdNotebookBookToolSelect, mdnotebook_booktool_select, MDNOTEBOOK_TYPE_BOOKTOOL)

void mdnotebook_booktool_select_append_selection_node(MdNotebookBookToolSelect* self, gdouble x, gdouble y) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(self);

	if (priv->num_nodes == priv->alloc_nodes) {
		priv->nodes = g_renew(MdNotebookBoundDrawingSelectionNode, priv->nodes, priv->alloc_nodes + 0x80);
		priv->alloc_nodes += 0x80;
	}

	MdNotebookBoundDrawingSelectionNode node = { .x = x, .y = y };
	priv->nodes[priv->num_nodes] = node;

	priv->num_nodes++;
}

static void mdnotebook_booktool_get_area_extends(MdNotebookBookToolSelect* self, gdouble* x0, gdouble* y0, gdouble* x1, gdouble* y1) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(self);
	*x0 = NAN; *y0 = NAN; *x1 = NAN; *y1 = NAN;
	for (gsize i = 0; i < priv->num_nodes; i++) {
		MdNotebookBoundDrawingSelectionNode node = priv->nodes[i];

		if (isnan(*x0) || node.x < *x0) *x0 = node.x;
		if (isnan(*y0) || node.y < *y0) *y0 = node.y;
		if (isnan(*x1) || node.x > *x1) *x1 = node.x;
		if (isnan(*y1) || node.y > *y1) *y1 = node.y;
	}
}
static void mdnotebook_booktool_select_commit_selection(MdNotebookBookToolSelect* self) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(self);
	MdNotebookView* view = mdnotebook_booktool_get_textview(MDNOTEBOOK_BOOKTOOL(self));
	GtkTextView* gview = (GtkTextView*)view;

	gdouble x0,y0,x1,y1;
	mdnotebook_booktool_get_area_extends(self, &x0, &y0, &x1, &y1);

	GtkTextIter i,j;
	gtk_text_view_get_iter_at_position(gview, &i, NULL, x0, y0);
	gtk_text_view_get_iter_at_position(gview, &j, NULL, x1, y1);

	GPtrArray* seen = g_ptr_array_new();

	do {
		GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&i);
		if (anch) {
			guint len;
			GtkWidget** w = gtk_text_child_anchor_get_widgets(anch, &len);
			if (len) {
				if (MDNOTEBOOK_IS_BOUNDDRAWING(w[0])) {
					GtkAllocation alloc;
					gtk_widget_get_allocation(w[0], &alloc);
					mdnotebook_bounddrawing_select_area(MDNOTEBOOK_BOUNDDRAWING(w[0]), priv->nodes, priv->num_nodes, -alloc.x, -alloc.y);

					g_ptr_array_add(seen, w[0]);
				}
			}
		}

		if (gtk_text_iter_compare(&i, &j) == 0) break;
	} while (gtk_text_iter_forward_char(&i));

	GListModel* children = gtk_widget_observe_children(GTK_WIDGET(view));
	for (guint i  = 0; i < g_list_model_get_n_items(children); i++) {
		GtkWidget* child = GTK_WIDGET(g_list_model_get_object(children, i));
		if (MDNOTEBOOK_IS_BOUNDDRAWING(child)) {
			if (!g_ptr_array_find(seen, child, NULL)) {
				mdnotebook_bounddrawing_unselect((MdNotebookBoundDrawing*)child);
			}
		}
	}
}

static void mdnotebook_booktool_select_object_dispose(GObject* object) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(MDNOTEBOOK_BOOKTOOL_SELECT(object));

	g_free(g_steal_pointer(&priv->nodes));

	G_OBJECT_CLASS(mdnotebook_booktool_select_parent_class)->dispose(object);
}

static const gchar* mdnotebook_booktool_select_booktool_icon_name(MdNotebookBookTool*) {
	return "edit-select-all-symbolic";
}

static void mdnotebook_booktool_select_booktool_activated(MdNotebookBookTool*, MdNotebookView* view) {
	mdnotebook_view_set_stylus_gesture_state(view, FALSE);
	mdnotebook_view_set_cursor_from_name(view, "crosshair");
}
static void mdnotebook_booktool_select_booktool_deactivated(MdNotebookBookTool*, MdNotebookView* view) {
	mdnotebook_view_set_stylus_gesture_state(view, TRUE);
	mdnotebook_view_set_cursor(view, NULL);
}

static gboolean mdnotebook_booktool_select_booktool_gesture_start(MdNotebookBookTool* tool, gdouble x, gdouble y, _ gdouble pressure) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(MDNOTEBOOK_BOOKTOOL_SELECT(tool));
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);

	priv->in_gesture = TRUE;

	priv->num_nodes = 0;
	// potentional reset the allocated stroke area back to 0x80?
	mdnotebook_booktool_select_append_selection_node(MDNOTEBOOK_BOOKTOOL_SELECT(tool), x, y);
	gtk_widget_queue_draw(stroke_proxy->overlay);

	return TRUE;
}
static gboolean mdnotebook_booktool_select_booktool_gesture_move(MdNotebookBookTool* tool, gdouble x, gdouble y, _ gdouble pressure) {
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);
	mdnotebook_booktool_select_append_selection_node(MDNOTEBOOK_BOOKTOOL_SELECT(tool), x, y);
	gtk_widget_queue_draw(stroke_proxy->overlay);

	return TRUE;
}
static gboolean mdnotebook_booktool_select_booktool_gesture_end(MdNotebookBookTool* tool, gdouble x, gdouble y, _ gdouble pressure) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(MDNOTEBOOK_BOOKTOOL_SELECT(tool));
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);

	priv->in_gesture = FALSE;

	// potentional reset the allocated stroke area back to 0x80?
	mdnotebook_booktool_select_append_selection_node(MDNOTEBOOK_BOOKTOOL_SELECT(tool), x, y);
	gtk_widget_queue_draw(stroke_proxy->overlay);

	mdnotebook_booktool_select_commit_selection(MDNOTEBOOK_BOOKTOOL_SELECT(tool));

	return TRUE;
}

static void mdnotebook_booktool_select_booktool_render_surface(MdNotebookBookTool* tool, cairo_t* ctx, gdouble, gdouble) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(MDNOTEBOOK_BOOKTOOL_SELECT(tool));

	if (!priv->in_gesture)
		return;
	if (priv->num_nodes == 0)
		return;

	cairo_set_line_width(ctx, 2.0);
	cairo_set_source_rgba(ctx, .627, .659, .75, 1.0);

	MdNotebookBoundDrawingSelectionNode start = priv->nodes[0];
	cairo_move_to(ctx, start.x, start.y);
	for (gsize i = 1; i < priv->num_nodes; i++) {
		MdNotebookBoundDrawingSelectionNode node = priv->nodes[i];
		cairo_line_to(ctx, node.x, node.y);
	}
	cairo_line_to(ctx, start.x, start.y);

	const double dashes[1] = {8.};
	cairo_set_dash(ctx, dashes, 1, 0.);
	cairo_stroke_preserve(ctx);

	cairo_set_source_rgba(ctx, .627, .659, .75, .15);
	cairo_fill(ctx);
}

static void mdnotebook_booktool_select_class_init(MdNotebookBookToolSelectClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	MdNotebookBookToolClass* booktool_class = MDNOTEBOOK_BOOKTOOL_CLASS(class);

	object_class->dispose = mdnotebook_booktool_select_object_dispose;

	booktool_class->icon_name = mdnotebook_booktool_select_booktool_icon_name;
	booktool_class->activated = mdnotebook_booktool_select_booktool_activated;
	booktool_class->deactivated = mdnotebook_booktool_select_booktool_deactivated;
	booktool_class->gesture_start = mdnotebook_booktool_select_booktool_gesture_start;
	booktool_class->gesture_end = mdnotebook_booktool_select_booktool_gesture_end;
	booktool_class->gesture_move = mdnotebook_booktool_select_booktool_gesture_move;
	booktool_class->render_surface = mdnotebook_booktool_select_booktool_render_surface;
}

static void mdnotebook_booktool_select_init(MdNotebookBookToolSelect* self) {
	MdNotebookBookToolSelectPrivate* priv = mdnotebook_booktool_select_get_instance_private(self);

	priv->num_nodes = 0;
	priv->nodes = g_new(MdNotebookBoundDrawingSelectionNode, 0x80);
	priv->alloc_nodes = 0x80;
}

MdNotebookBookTool* mdnotebook_booktool_select_new(MdNotebookView* view) {
	return g_object_new(MDNOTEBOOK_TYPE_BOOKTOOL_SELECT, "textview", view, NULL);
}
