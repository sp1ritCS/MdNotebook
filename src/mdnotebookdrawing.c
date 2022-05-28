#include "mdnotebookdrawing.h"

#define _ __attribute__((unused))

MdNotebookStroke* mdnotebook_stroke_new(guint32 color) {
	MdNotebookStroke* stroke = g_new(MdNotebookStroke, 1);
	stroke->color = color;
	stroke->num_nodes = 0;
	stroke->nodes = g_new(MdNotebookStrokeNode, 0x80);
	stroke->alloc_nodes = 0x80;

	return stroke;
}

void mdnotebook_stroke_free(gpointer strokeptr) {
	if (!strokeptr)
		return;

	MdNotebookStroke* stroke = (MdNotebookStroke*)strokeptr;
	g_free(stroke->nodes);
	g_free(stroke);
}

void mdnotebook_stroke_append_node(MdNotebookStroke* stroke, gdouble x, gdouble y, gdouble pressure) {
	if (!stroke)
		return;

	if (stroke->num_nodes == stroke->alloc_nodes) {
		stroke->nodes = g_renew(MdNotebookStrokeNode, stroke->nodes, stroke->alloc_nodes + 0x80);
		stroke->alloc_nodes += 0x80;
	}

	MdNotebookStrokeNode node = { .x = x, .y = y, .p = pressure };
	stroke->nodes[stroke->num_nodes] = node;

	stroke->num_nodes++;
}

void mdnotebook_stroke_force_min_xy(MdNotebookStroke* stroke, gdouble x, gdouble y) {
	if (!stroke)
		return;

	for (gsize i = 0; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode* node = &stroke->nodes[i];
		if (node->x - node->p < x) node->x = x + node->p;
		if (node->y - node->p < y) node->y = y + node->p;
	}
}

gboolean mdnotebook_stroke_get_bbox(MdNotebookStroke* stroke, gdouble* x0, gdouble* x1, gdouble* y0, gdouble* y1) {
	if (!stroke || !stroke->num_nodes)
		return FALSE;

	{
		MdNotebookStrokeNode* firstnode = &stroke->nodes[0];
		*x0 = firstnode->x; *x1 = firstnode->x;
		*y0 = firstnode->y; *y1 = firstnode->y;
	}
	for (gsize i = 0; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode* node = &stroke->nodes[i];
		if (node->x - node->p < *x0) *x0 = node->x - node->p;
		if (node->x + node->p > *x1) *x1 = node->x + node->p;
		if (node->y - node->p < *y0) *y0 = node->y - node->p;
		if (node->y + node->p > *y1) *y1 = node->y + node->p;
	}

	return TRUE;
}

/*
 * Test if rectangle defined by (x0|y0)@(x1|y1) contains a stroke
 */
gboolean mdnotebook_stroke_test_rectangle(MdNotebookStroke* stroke, gdouble x0, gdouble y0, gdouble x1, gdouble y1) {
	if (!stroke)
		return FALSE;

	for (gsize i = 0; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode node = stroke->nodes[i];
		if ((x0 < node.x) && (y0 < node.y) &&
			(x1 > node.x) && (y1 > node.y))
			return TRUE;
	}
	return FALSE;
}

void mdnotebook_stroke_render(MdNotebookStroke* stroke, cairo_t* ctx, gboolean debug_mode) {
	if (!stroke)
		return;

	cairo_set_source_rgba(ctx,
		(gdouble)((stroke->color >> 16) & 0xFF) / (gdouble)0xFF,
		(gdouble)((stroke->color >>  8) & 0xFF) / (gdouble)0xFF,
		(gdouble)((stroke->color >>  0) & 0xFF) / (gdouble)0xFF,
		(gdouble)((stroke->color >> 24) & 0xFF) / (gdouble)0xFF
	);
	cairo_set_line_cap(ctx, CAIRO_LINE_CAP_ROUND);
	for (gsize i = 1; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode prevnode = stroke->nodes[i-1];
		MdNotebookStrokeNode node = stroke->nodes[i];
		cairo_move_to(ctx, prevnode.x, prevnode.y);
		cairo_line_to(ctx, node.x, node.y);
		cairo_set_line_width(ctx, prevnode.p * MDNOTEBOOK_STROKE_SIZE_MULTIPLIER);
		cairo_stroke(ctx);
	}

	if (debug_mode)
		for (gsize i = 1; i < stroke->num_nodes; i++) {
			MdNotebookStrokeNode node = stroke->nodes[i];
			MdNotebookStrokeNode prevnode = stroke->nodes[i-1];

			cairo_set_line_width(ctx, 0.5);
			cairo_set_source_rgba(ctx, 0.301960784, 0.760784314, 0.941176471, 1.0);
			cairo_move_to(ctx, prevnode.x, prevnode.y);
			cairo_line_to(ctx, node.x, node.y);
			cairo_status(ctx);

			cairo_set_line_width(ctx, 0.75);
			cairo_set_source_rgba(ctx, 0.937254902, 0.301960784, 0.941176471, 1.0);
			cairo_move_to(ctx, node.x - 2.5, node.y - 2.5);
			cairo_line_to(ctx, node.x + 2.5, node.y + 2.5);
			cairo_move_to(ctx, node.x + 2.5, node.y - 2.5);
			cairo_line_to(ctx, node.x - 2.5, node.y + 2.5);
			cairo_stroke(ctx);
		}
}

void mdnotebook_stroke_set_color(MdNotebookStroke* stroke, guint32 color) {
	if (!stroke)
		return;

	stroke->color = color;
}

// TODO: implement more efficient algorithm that keeps track of the tip of the stroke
typedef struct {
	gint width, height;
	GSList* strokes;
	//GSList* stroke_head;

	gboolean debug;
} MdNotebookBoundDrawingPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBoundDrawing, mdnotebook_bounddrawing, GTK_TYPE_WIDGET)

enum {
	PROP_DEBUG = 1,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_bounddrawing_object_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookBoundDrawing* self = MDNOTEBOOK_BOUNDDRAWING(object);

	switch (prop_id) {
		case PROP_DEBUG:
			g_value_set_boolean(value, mdnotebook_bounddrawing_get_debug(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

}
static void mdnotebook_bounddrawing_object_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookBoundDrawing* self = MDNOTEBOOK_BOUNDDRAWING(object);

	switch (prop_id) {
		case PROP_DEBUG:
			mdnotebook_bounddrawing_set_debug(self, g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}
static void mdnotebook_bounddrawing_object_dispose(GObject* object) {
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(MDNOTEBOOK_BOUNDDRAWING(object));

	if (priv->strokes)
		g_slist_free_full(g_steal_pointer(&priv->strokes), mdnotebook_stroke_free);
}

static GtkSizeRequestMode mdnotebook_bounddrawing_widget_get_request_mode(_ GtkWidget* widget) {
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}
static void mdnotebook_bounddrawing_widget_measure(GtkWidget* widget, GtkOrientation orientation, _ gint for_size, gint* min, gint* nat, gint* min_baseline, gint* nat_baseline) {
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(MDNOTEBOOK_BOUNDDRAWING(widget));

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		*min = priv->width;
		*nat = priv->width;
	}
	if (orientation == GTK_ORIENTATION_VERTICAL) {
		*min = priv->height;
		*nat = priv->height;

		*min_baseline = 0;
		*nat_baseline = 0;
	}
}

typedef struct {
	MdNotebookBoundDrawing* self;
	cairo_t* ctx;
} MdNotebookBoundDrawingStrokeIterUd;
static void mdnotebook_bounddrawing_stroke_iter(MdNotebookStroke* stroke, MdNotebookBoundDrawingStrokeIterUd* ud) {
	if (!stroke)
		return;
	mdnotebook_stroke_render(stroke, ud->ctx, mdnotebook_bounddrawing_get_debug(ud->self));
}
static void mdnotebook_bounddrawing_widget_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(MDNOTEBOOK_BOUNDDRAWING(widget));

	gdouble
		width = gtk_widget_get_allocated_width(widget),
		height = gtk_widget_get_allocated_height(widget);

	cairo_t* ctx = gtk_snapshot_append_cairo(snapshot, &GRAPHENE_RECT_INIT(0, 0, width, height));
	MdNotebookBoundDrawingStrokeIterUd stroke_iter_ud = { .self = MDNOTEBOOK_BOUNDDRAWING(widget), .ctx = ctx };
	g_slist_foreach(priv->strokes, (GFunc)mdnotebook_bounddrawing_stroke_iter, &stroke_iter_ud);

	cairo_destroy(ctx);
}

static void mdnotebook_bounddrawing_class_init(MdNotebookBoundDrawingClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);

	object_class->get_property = mdnotebook_bounddrawing_object_get_property;
	object_class->set_property = mdnotebook_bounddrawing_object_set_property;
	object_class->dispose = mdnotebook_bounddrawing_object_dispose;

	widget_class->get_request_mode = mdnotebook_bounddrawing_widget_get_request_mode;
	widget_class->measure = mdnotebook_bounddrawing_widget_measure;
	widget_class->snapshot = mdnotebook_bounddrawing_widget_snapshot;

	obj_properties[PROP_DEBUG] = g_param_spec_boolean("debug", "Debug", "Render debug nodes", FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void mdnotebook_bounddrawing_init(MdNotebookBoundDrawing* self) {
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);

	priv->width = -1;
	priv->height = -1;
	priv->strokes = g_slist_alloc();
	//priv->stroke_head = priv->strokes;

	priv->debug = FALSE;
}

GtkWidget* mdnotebook_bounddrawing_new() {
	return g_object_new(MDNOTEBOOK_TYPE_BOUNDDRAWING, NULL);
}

MdNotebookBoundDrawing* mdnotebook_bounddrawing_try_upcast(GtkWidget* w) {
	if (MDNOTEBOOK_IS_BOUNDDRAWING(w))
		return (MdNotebookBoundDrawing*)w;
	else
		return NULL;
}

gboolean mdnotebook_bounddrawing_get_debug(MdNotebookBoundDrawing* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_BOUNDDRAWING(self), FALSE);
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);

	return priv->debug;
}
void mdnotebook_bounddrawing_set_debug(MdNotebookBoundDrawing* self, gboolean debug) {
	g_return_if_fail(MDNOTEBOOK_IS_BOUNDDRAWING(self));
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);

	if (priv->debug == debug)
		return;
	priv->debug = debug;
	gtk_widget_queue_draw(GTK_WIDGET(self));

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_DEBUG]);
}

gboolean mdnotebook_bounddrawing_get_size(MdNotebookBoundDrawing* self, gint* width, gint* height) {
	g_return_val_if_fail(MDNOTEBOOK_IS_BOUNDDRAWING(self), FALSE);
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);

	*width = priv->width;
	*height = priv->height;
	return TRUE;
}

typedef struct {
	gint dx;
	gint dy;
} MdNotebookBoundDrawingUpdateSizeIterUD;
void mdnotebook_bounddrawing_update_size_update_stroke_iter(MdNotebookStroke* stroke, MdNotebookBoundDrawingUpdateSizeIterUD* ud) {
	if (!stroke)
		return;
	for (gsize i = 0; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode* node = &stroke->nodes[i];
		node->x -= ud->dx;
		node->y -= ud->dy;
	}
}

/* Change the drawing's size, possibly resizing the internal buffer in the process */
/* dx, dy move the top left corner; neww, newh are relative to the OLD top left corner */
/* returns false if the resizing would result in some strokes falling off the edge */
gboolean mdnotebook_bounddrawing_update_size(MdNotebookBoundDrawing* self, gint neww, int newh, int dx, int dy) {
	g_return_val_if_fail(MDNOTEBOOK_IS_BOUNDDRAWING(self), FALSE);
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);
	neww -= dx; newh -= dy;

	if (dx != 0 || dy != 0) {
		// check that the new size is positive and that no strokes fell off the left
		if (neww < 0 || newh < 0)
			return FALSE;
		GSList* stroke = priv->strokes;
		while (stroke) {
			MdNotebookStroke* data = (MdNotebookStroke*)stroke->data;
			for (gsize i = 0; i < data->num_nodes; i++) {
				MdNotebookStrokeNode* node = &data->nodes[i];
				if (node->x < dx || node->y < dy)
					return FALSE;
			}
			stroke = stroke->next;
		}
	}

	MdNotebookBoundDrawingUpdateSizeIterUD ud = { .dx = dx, .dy = dy };
	g_slist_foreach(priv->strokes, (GFunc)mdnotebook_bounddrawing_update_size_update_stroke_iter, &ud);

	if (priv->width != neww || priv->height != newh) {
		priv->width = neww;
		priv->height = newh;

		gtk_widget_queue_resize(GTK_WIDGET(self));
	} else {
		gtk_widget_queue_draw(GTK_WIDGET(self));
	}

	return TRUE;
}

static void mdnotebook_bounddrawing_add_stroke_append_stroke(GSList** head, MdNotebookStroke* stroke) {
	if ((*head)->data)
		*head = g_slist_append(*head, stroke);
	else
		(*head)->data = stroke;
}

/* push and draw a new stroke, shifting it by (dx,dy) to accommodate the local coordinate system */
/* if force is false, then return false if some of the new stroke would wind up to the left of the left boundary */
gboolean mdnotebook_bounddrawing_add_stroke(MdNotebookBoundDrawing* self, MdNotebookStroke* stroke /* takes ownership */, gdouble dx, gdouble dy, gboolean force) {
	g_return_val_if_fail(MDNOTEBOOK_IS_BOUNDDRAWING(self), FALSE);
	if (!stroke)
		return FALSE;

	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);

	// validate that incoming stroke is entirely in positive coordinate space, either by rejecting it or by changing offending entries
	if (!force) for (gsize i = 0; i < stroke->num_nodes; i++) {
		if (stroke->nodes[i].x < -dx || stroke->nodes[i].y < -dy)
			return FALSE;
	} else for (gsize i = 0; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode* node = &stroke->nodes[i];
		if (node->x < -dx) node->x = -dx;
		if (node->y < -dy) node->y = -dy;
	}

	gint neww = priv->width, newh = priv->height;
	mdnotebook_bounddrawing_add_stroke_append_stroke(&priv->strokes, stroke);
	for (gsize i = 0; i < stroke->num_nodes; i++) {
		MdNotebookStrokeNode* node = &stroke->nodes[i];

		node->x += dx;
		node->y += dy;

		gint newx = node->x,
		     newy = node->y,
		     newp = node->p + 1; // add 1 as safety margin

		if (newx + newp > neww) neww = newx + newp;
		if (newy + newp > newh) newh = newy + newp;
	}

	mdnotebook_bounddrawing_update_size(self, neww, newh, 0, 0);

	return TRUE;
}

void mdnotebook_bounddrawing_erase_sqare_area(MdNotebookBoundDrawing* self, gdouble cx, gdouble cy, gdouble padding) {
	g_return_if_fail(MDNOTEBOOK_IS_BOUNDDRAWING(self));
	MdNotebookBoundDrawingPrivate* priv = mdnotebook_bounddrawing_get_instance_private(self);

	gdouble ex0,ey0,ex1,ey1;
	gboolean rerender = FALSE;

	ex0 = cx-padding;
	ey0 = cy-padding;
	ex1 = cx+padding;
	ey1 = cy+padding;

	GSList* tmp = NULL;
	GSList** head = &priv->strokes;
	while (*head) {
		tmp = *head;

		gboolean inside_rect = mdnotebook_stroke_test_rectangle((MdNotebookStroke*)(*head)->data, ex0, ey0, ex1, ey1);
		rerender = rerender || inside_rect;

		if (inside_rect) {
			*head = tmp->next;
			mdnotebook_stroke_free(tmp->data);
			g_slist_free_1(tmp);
		} else {
			head = &tmp->next;
		}
	}
	if (!priv->strokes)
		priv->strokes = g_slist_alloc();

	if (rerender)
		gtk_widget_queue_draw(GTK_WIDGET(self));
}
