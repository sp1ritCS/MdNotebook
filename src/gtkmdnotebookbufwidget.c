#include "gtkmdnotebookbufwidget.h"
#include "mdnotebook_enums.h"

typedef enum {
	MDNOTEBOOK_BUFWIDGET_CORNER_NONE = 0,
	MDNOTEBOOK_BUFWIDGET_CORNER_NORTHWEST,
	MDNOTEBOOK_BUFWIDGET_CORNER_NORTHEAST,
	MDNOTEBOOK_BUFWIDGET_CORNER_SOUTHWEST,
	MDNOTEBOOK_BUFWIDGET_CORNER_SOUTHEAST
} MdNotebookBufWidgetCorner;

typedef struct {
	GtkWidget* child;
	gdouble xzoom;
	gdouble yzoom;
	gdouble xzoomoff;
	gdouble yzoomoff;
	MdNotebookBufWidgetCorner drag;
} MdNotebookBufWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBufWidget, mdnotebook_bufwidget, GTK_TYPE_WIDGET)

enum {
	PROP_CHILD = 1,
	PROP_XZOOM,
	PROP_YZOOM,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_bufwidget_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookBufWidget* self = MDNOTEBOOK_BUFWIDGET(object);

	switch (prop_id) {
		case PROP_CHILD:
			g_value_set_object(value, mdnotebook_bufwidget_get_child(self));
			break;
		case PROP_XZOOM:
			g_value_set_double(value, mdnotebook_bufwidget_get_zoom(self, MDNOTEBOOK_AXIS_X));
			break;
		case PROP_YZOOM:
			g_value_set_double(value, mdnotebook_bufwidget_get_zoom(self, MDNOTEBOOK_AXIS_Y));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

}

static void mdnotebook_bufwidget_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookBufWidget* self = MDNOTEBOOK_BUFWIDGET(object);

	switch (prop_id) {
		case PROP_CHILD:
			mdnotebook_bufwidget_set_child(self, g_value_get_object(value));
			break;
		case PROP_XZOOM:
			mdnotebook_bufwidget_set_zoom(self, MDNOTEBOOK_AXIS_X, g_value_get_double(value));
			break;
		case PROP_YZOOM:
			mdnotebook_bufwidget_set_zoom(self, MDNOTEBOOK_AXIS_Y, g_value_get_double(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_bufwidget_dispose(GObject* object) {
	MdNotebookBufWidget* self = MDNOTEBOOK_BUFWIDGET(object);
	MdNotebookBufWidgetPrivate* priv = mdnotebook_bufwidget_get_instance_private(self);

	g_clear_pointer(&priv->child, gtk_widget_unparent);
	G_OBJECT_CLASS(mdnotebook_bufwidget_parent_class)->dispose(object);
}

static void mdnotebook_bufwidget_measure(GtkWidget* widget, GtkOrientation orientation, gint for_size, gint* minimum, gint* natural, gint* minimum_baseline, gint* natural_baseline) {
	MdNotebookBufWidget* self = MDNOTEBOOK_BUFWIDGET(widget);
	MdNotebookBufWidgetPrivate* priv = mdnotebook_bufwidget_get_instance_private(self);

	gtk_widget_measure(priv->child, orientation, for_size, minimum, natural, minimum_baseline, natural_baseline);

	if (orientation == GTK_ORIENTATION_VERTICAL) {
		*minimum *= priv->yzoom;
		*natural *= priv->yzoom;
		*minimum_baseline *= priv->yzoom;
		*natural_baseline *= priv->yzoom;
	}
	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		*minimum *= priv->xzoom;
		*natural *= priv->xzoom;
	}
}

const gdouble SELECTION_HANDLE_LEN = 6.;

static void mdnotebook_bufwidget_size_allocate(GtkWidget* widget, gint width, gint height, gint baseline) {
	MdNotebookBufWidget* self = MDNOTEBOOK_BUFWIDGET(widget);
	MdNotebookBufWidgetPrivate* priv = mdnotebook_bufwidget_get_instance_private(self);
	
	gdouble sfx = 1 - (2*SELECTION_HANDLE_LEN)/width;
	gdouble sfy = 1 - (2*SELECTION_HANDLE_LEN)/height;
	
	GskTransform* trans = gsk_transform_new();
	trans = gsk_transform_translate(trans, &GRAPHENE_POINT_INIT(SELECTION_HANDLE_LEN,SELECTION_HANDLE_LEN));
	trans = gsk_transform_scale(trans, priv->xzoom * sfx, priv->yzoom * sfy);
	

	gtk_widget_allocate(priv->child, (gdouble)width / priv->xzoom, (gdouble)height / priv->yzoom, baseline, trans);
}

static MdNotebookBufWidgetCorner mdnotebook_bufwidget_corner_test(MdNotebookBufWidget* self, gdouble x, gdouble y) {
	gdouble
		width = gtk_widget_get_allocated_width(GTK_WIDGET(self)),
		height = gtk_widget_get_allocated_height(GTK_WIDGET(self));
	
	if (graphene_rect_contains_point(&GRAPHENE_RECT_INIT(0, 0, SELECTION_HANDLE_LEN, SELECTION_HANDLE_LEN), &GRAPHENE_POINT_INIT(x, y)))
		return MDNOTEBOOK_BUFWIDGET_CORNER_NORTHWEST;
	if (graphene_rect_contains_point(&GRAPHENE_RECT_INIT(width - SELECTION_HANDLE_LEN, 0, SELECTION_HANDLE_LEN, SELECTION_HANDLE_LEN), &GRAPHENE_POINT_INIT(x, y)))
		return MDNOTEBOOK_BUFWIDGET_CORNER_NORTHEAST;
	if (graphene_rect_contains_point(&GRAPHENE_RECT_INIT(0, height - SELECTION_HANDLE_LEN, SELECTION_HANDLE_LEN, SELECTION_HANDLE_LEN), &GRAPHENE_POINT_INIT(x, y)))
		return MDNOTEBOOK_BUFWIDGET_CORNER_SOUTHWEST;
	if (graphene_rect_contains_point(&GRAPHENE_RECT_INIT(width - SELECTION_HANDLE_LEN, height - SELECTION_HANDLE_LEN, SELECTION_HANDLE_LEN, SELECTION_HANDLE_LEN), &GRAPHENE_POINT_INIT(x, y)))
		return MDNOTEBOOK_BUFWIDGET_CORNER_SOUTHEAST;
	
	return MDNOTEBOOK_BUFWIDGET_CORNER_NONE;
} 

static void mdnotebook_bufwidget_handle_motion(GtkEventControllerMotion *event,
	gdouble x,
	gdouble y,
	MdNotebookBufWidget* self
) {
	g_return_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self));
	
	if (!gtk_event_controller_motion_contains_pointer(event))
		return;
	
	MdNotebookBufWidgetCorner corner = mdnotebook_bufwidget_corner_test(self, x, y);
	switch (corner) {
	case MDNOTEBOOK_BUFWIDGET_CORNER_NORTHWEST:
		gtk_widget_set_cursor_from_name(GTK_WIDGET(self), "nw-resize");
		break;
	case MDNOTEBOOK_BUFWIDGET_CORNER_NORTHEAST:
		gtk_widget_set_cursor_from_name(GTK_WIDGET(self), "ne-resize");
		break;
	case MDNOTEBOOK_BUFWIDGET_CORNER_SOUTHWEST:
		gtk_widget_set_cursor_from_name(GTK_WIDGET(self), "sw-resize");
		break;
	case MDNOTEBOOK_BUFWIDGET_CORNER_SOUTHEAST:
		gtk_widget_set_cursor_from_name(GTK_WIDGET(self), "se-resize");
		break;
	default:
		gtk_widget_set_cursor_from_name(GTK_WIDGET(self), "default");
	}
}

static void mdnotebook_bufwidget_handle_drag_start(GtkGestureDrag* event, gdouble x, gdouble y, MdNotebookBufWidget* self) {
	MdNotebookBufWidgetPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self));
	priv = mdnotebook_bufwidget_get_instance_private(self);
	
	priv->drag = mdnotebook_bufwidget_corner_test(self, x, y);
}
static void mdnotebook_bufwidget_handle_drag_mov(GtkGestureDrag* event, gdouble xoff, gdouble yoff, MdNotebookBufWidget* self) {
	MdNotebookBufWidgetPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self));
	priv = mdnotebook_bufwidget_get_instance_private(self);
	
	if (priv->drag) {
		priv->xzoomoff = xoff;
		priv->yzoomoff = yoff;
		gtk_widget_queue_draw(GTK_WIDGET(self));
		gtk_gesture_set_state(GTK_GESTURE(event), GTK_EVENT_SEQUENCE_CLAIMED);
	}
}
static void mdnotebook_bufwidget_handle_drag_end(GtkGestureDrag* event, gdouble xoff, gdouble yoff, MdNotebookBufWidget* self) {
	MdNotebookBufWidgetPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self));
	priv = mdnotebook_bufwidget_get_instance_private(self);
	
	if (priv->drag) {
		gdouble
			width = gtk_widget_get_allocated_width(GTK_WIDGET(self)),
			height = gtk_widget_get_allocated_height(GTK_WIDGET(self));

		gdouble
			nwidth = width + xoff,
			nheight = height + yoff;

		mdnotebook_bufwidget_set_zoom(self, MDNOTEBOOK_AXIS_X, fabs(nwidth / (width / priv->xzoom)));
		mdnotebook_bufwidget_set_zoom(self, MDNOTEBOOK_AXIS_Y, fabs(nheight / (height / priv->yzoom)));

		priv->drag = MDNOTEBOOK_BUFWIDGET_CORNER_NONE;
	}
}

void mdnotebook_draw_selection_handle(cairo_t* ctx, double x, double y) {
	double
		len = SELECTION_HANDLE_LEN,
		radius = 1.;
	
	double degrees = M_PI / 180.0; 
	
	cairo_new_sub_path(ctx);
	cairo_arc(ctx, x + len - radius, y + radius, radius, -90 * degrees, 0 * degrees);
	cairo_arc(ctx, x + len - radius, y + len - radius, radius, 0 * degrees, 90 * degrees);
	cairo_arc(ctx, x + radius, y + len - radius, radius, 90 * degrees, 180 * degrees);
	cairo_arc(ctx, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
	cairo_close_path(ctx);
	
	cairo_set_source_rgba(ctx, 1., 1., 1., 1.);
	cairo_fill_preserve(ctx);
	cairo_set_source_rgba(ctx, .627,.659,.75,1);
	cairo_set_line_width(ctx, 1.);
	cairo_stroke(ctx);
}

static void mdnotebook_bufwidget_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	MdNotebookBufWidget* self = MDNOTEBOOK_BUFWIDGET(widget);
	MdNotebookBufWidgetPrivate* priv = mdnotebook_bufwidget_get_instance_private(self);
	
	gdouble
		width = gtk_widget_get_allocated_width(widget),
		height = gtk_widget_get_allocated_height(widget);

	if (priv->drag) {
		gdouble
			sfx = 1 + priv->xzoomoff / width,
			sfy = 1 + priv->yzoomoff / height;

		gtk_snapshot_scale(snapshot, sfx, sfy);
	}

	cairo_t* ctx = gtk_snapshot_append_cairo(snapshot,
		&GRAPHENE_RECT_INIT(0, 0, width, height)
	);
	
	// Begin CAIRO selection border
	cairo_set_line_width(ctx, 2.0);
	cairo_set_source_rgba(ctx, .627,.659,.75,1);
	const double dashes[1] = {2.};
	cairo_set_dash(ctx, dashes, 1, 0.);
	
	cairo_rectangle(ctx, SELECTION_HANDLE_LEN / 2, SELECTION_HANDLE_LEN / 2, width - SELECTION_HANDLE_LEN, height - SELECTION_HANDLE_LEN);
	cairo_stroke(ctx);
	
	// Begin CAIRO selection handle
	cairo_set_dash(ctx, NULL, 0, 0.);
	mdnotebook_draw_selection_handle(ctx, 0, 0);
	mdnotebook_draw_selection_handle(ctx, width - SELECTION_HANDLE_LEN, 0);
	mdnotebook_draw_selection_handle(ctx, 0, height - SELECTION_HANDLE_LEN);
	mdnotebook_draw_selection_handle(ctx, width - SELECTION_HANDLE_LEN, height - SELECTION_HANDLE_LEN);
	
	
	gtk_widget_snapshot_child(widget, priv->child, snapshot);
}

static void mdnotebook_bufwidget_class_init(MdNotebookBufWidgetClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);

	object_class->dispose = mdnotebook_bufwidget_dispose;
	object_class->get_property = mdnotebook_bufwidget_get_property;
	object_class->set_property = mdnotebook_bufwidget_set_property;

	widget_class->measure = mdnotebook_bufwidget_measure;
	widget_class->size_allocate = mdnotebook_bufwidget_size_allocate;
	widget_class->snapshot = mdnotebook_bufwidget_snapshot;

	obj_properties[PROP_CHILD] = g_param_spec_object("child", "Child", "The child widget", GTK_TYPE_WIDGET, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	obj_properties[PROP_XZOOM] = g_param_spec_double("xzoom", "X Zoom", "Zoomfactor in X-axis Direction (Width)", 0., G_MAXFLOAT, 1., G_PARAM_READWRITE);
	obj_properties[PROP_YZOOM] = g_param_spec_double("yzoom", "Y Zoom", "Zoomfactor in Y-axis Direction (Height)", 0., G_MAXFLOAT, 1., G_PARAM_READWRITE);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void mdnotebook_bufwidget_init(MdNotebookBufWidget* self) {
	MdNotebookBufWidgetPrivate* priv = mdnotebook_bufwidget_get_instance_private(self);

	priv->child = NULL;
	priv->xzoom = 2.;
	priv->yzoom = 1.5;
	priv->xzoomoff = 0.;
	priv->yzoomoff = 0.;
	priv->drag = MDNOTEBOOK_BUFWIDGET_CORNER_NONE;
	
	GtkEventController* pointermov = gtk_event_controller_motion_new();
	g_signal_connect(pointermov, "motion", G_CALLBACK(mdnotebook_bufwidget_handle_motion), self);
	gtk_widget_add_controller(GTK_WIDGET(self), pointermov);
	
	GtkGesture* pointergest = gtk_gesture_drag_new();
	g_signal_connect(pointergest, "drag-begin", G_CALLBACK(mdnotebook_bufwidget_handle_drag_start), self);
	g_signal_connect(pointergest, "drag-update", G_CALLBACK(mdnotebook_bufwidget_handle_drag_mov), self);
	g_signal_connect(pointergest, "drag-end", G_CALLBACK(mdnotebook_bufwidget_handle_drag_end), self);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(pointergest));
}

GtkWidget* mdnotebook_bufwidget_new(void) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFWIDGET, NULL);
}

GtkWidget* mdnotebook_bufwidget_get_child(MdNotebookBufWidget* self) {
	MdNotebookBufWidgetPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self), NULL);

	priv = mdnotebook_bufwidget_get_instance_private(self);

	return priv->child;
}

void mdnotebook_bufwidget_set_child(MdNotebookBufWidget* self, GtkWidget* child) {
	MdNotebookBufWidgetPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self));
	g_return_if_fail(child == NULL || GTK_IS_WIDGET(child));

	priv = mdnotebook_bufwidget_get_instance_private(self);

	if (priv->child == child)
		return;

	if (priv->child)
		gtk_widget_unparent(priv->child);

	priv->child = child;

	if (priv->child)
		gtk_widget_set_parent(priv->child, GTK_WIDGET(self));

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CHILD]);
}

gdouble mdnotebook_bufwidget_get_zoom(MdNotebookBufWidget* self, MdNotebookAxis axis) {
	MdNotebookBufWidgetPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self), 1.);

	priv = mdnotebook_bufwidget_get_instance_private(self);

	switch (axis) {
	case MDNOTEBOOK_AXIS_X:
		return priv->xzoom;
	case MDNOTEBOOK_AXIS_Y:
		return priv->yzoom;
	default:
		g_critical("mdnotebook_bufwidget_get_zoom received axis out of bounds");
		return 1.;
	}
}

void mdnotebook_bufwidget_set_zoom(MdNotebookBufWidget* self, MdNotebookAxis axis, gdouble zoom) {
	MdNotebookBufWidgetPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_BUFWIDGET(self));

	priv = mdnotebook_bufwidget_get_instance_private(self);

	switch (axis) {
	case MDNOTEBOOK_AXIS_X:
		priv->xzoom = zoom;
		break;
	case MDNOTEBOOK_AXIS_Y:
		priv->yzoom = zoom;
		break;
	default:
		g_critical("mdnotebook_bufwidget_set_zoom received axis out of bounds");
	}

	gtk_widget_queue_resize(GTK_WIDGET(self));
}
