#include "mdnotebookzoomview.h"
#include "mdnotebookview.h"

typedef struct {
	GtkWidget* child;
	gdouble oldzoom;
	gdouble zoom;
} MdNotebookZoomViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookZoomView, mdnotebook_zoomview, GTK_TYPE_WIDGET)

enum {
	PROP_CHILD = 1,
	PROP_ZOOM,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_zoomview_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookZoomView* self = MDNOTEBOOK_ZOOMVIEW(object);

	switch (prop_id) {
		case PROP_CHILD:
			g_value_set_object(value, mdnotebook_zoomview_get_textview(self));
			break;
		case PROP_ZOOM:
			g_value_set_double(value, mdnotebook_zoomview_get_zoom(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_zoomview_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookZoomView* self = MDNOTEBOOK_ZOOMVIEW(object);

	switch (prop_id) {
		case PROP_CHILD:
			mdnotebook_zoomview_set_textview(self, g_value_get_object(value));
			break;
		case PROP_ZOOM:
			mdnotebook_zoomview_set_zoom(self, g_value_get_double(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_zoomview_dispose(GObject* object) {
	MdNotebookZoomView* self = MDNOTEBOOK_ZOOMVIEW(object);
	MdNotebookZoomViewPrivate* priv = mdnotebook_zoomview_get_instance_private(self);

	g_clear_pointer(&priv->child, gtk_widget_unparent);
	G_OBJECT_CLASS(mdnotebook_zoomview_parent_class)->dispose(object);
}

static void mdnotebook_zoomview_measure(GtkWidget* widget, GtkOrientation orientation, gint for_size, gint* min, gint* nat, gint* min_baseline, gint* nat_baseline) {
	MdNotebookZoomView* self = MDNOTEBOOK_ZOOMVIEW(widget);
	MdNotebookZoomViewPrivate* priv = mdnotebook_zoomview_get_instance_private(self);

	gtk_widget_measure(priv->child, orientation, for_size, min, nat, min_baseline, nat_baseline);

	*min *= priv->zoom;
	*nat *= priv->zoom;
}

static void mdnotebook_zoomview_size_allocate(GtkWidget* widget, gint width, gint height, gint baseline) {
	MdNotebookZoomView* self = MDNOTEBOOK_ZOOMVIEW(widget);
	MdNotebookZoomViewPrivate* priv = mdnotebook_zoomview_get_instance_private(self);

	GskTransform* trans = gsk_transform_new();
	trans = gsk_transform_scale(trans, priv->zoom, priv->zoom);

	gtk_widget_allocate(priv->child, width, height, baseline, trans);
}

static void mdnotebook_zoomview_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	MdNotebookZoomView* self = MDNOTEBOOK_ZOOMVIEW(widget);
	MdNotebookZoomViewPrivate* priv = mdnotebook_zoomview_get_instance_private(self);

	gtk_widget_snapshot_child(widget, priv->child, snapshot);
}

static void mdnotebook_zoomview_class_init(MdNotebookZoomViewClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);

	object_class->dispose = mdnotebook_zoomview_dispose;
	object_class->get_property = mdnotebook_zoomview_get_property;
	object_class->set_property = mdnotebook_zoomview_set_property;

	widget_class->measure = mdnotebook_zoomview_measure;
	widget_class->size_allocate = mdnotebook_zoomview_size_allocate;
	widget_class->snapshot = mdnotebook_zoomview_snapshot;

	obj_properties[PROP_CHILD] = g_param_spec_object("child", "Child", "The child widget", GTK_TYPE_TEXT_VIEW, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	obj_properties[PROP_ZOOM] = g_param_spec_double("zoom", "Zoom", "Zoomfactor of the Textview", 0., 25., 1., G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY);
	
	g_object_class_install_properties(object_class,  N_PROPERTIES, obj_properties);
}

static void mdnotebook_handle_pinchzoom(GtkGestureZoom* event, gdouble scale, MdNotebookZoomView* self) {
	MdNotebookZoomViewPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_ZOOMVIEW(self));

	priv = mdnotebook_zoomview_get_instance_private(self);

	priv->zoom = priv->oldzoom * scale;

	gtk_widget_queue_resize(GTK_WIDGET(self));

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_ZOOM]);
}

static void mdnotebook_handle_pinchzoom_end(GtkGestureZoom* event, GdkEventSequence* seq, MdNotebookZoomView* self) {
	MdNotebookZoomViewPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_ZOOMVIEW(self));

	priv = mdnotebook_zoomview_get_instance_private(self);

	mdnotebook_zoomview_set_zoom(self, priv->zoom);
}


static void mdnotebook_zoomview_init(MdNotebookZoomView* self) {
	MdNotebookZoomViewPrivate* priv;

	priv = mdnotebook_zoomview_get_instance_private(self);

	priv->child = NULL;
	mdnotebook_zoomview_set_textview(self, GTK_TEXT_VIEW(mdnotebook_view_new()));

	GtkGesture* gestzoom = gtk_gesture_zoom_new();
	g_signal_connect(gestzoom, "scale-changed", G_CALLBACK(mdnotebook_handle_pinchzoom), self);
	g_signal_connect(gestzoom, "end", G_CALLBACK(mdnotebook_handle_pinchzoom_end), self);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gestzoom));
}

GtkWidget* mdnotebook_zoomview_new(void) {
	return g_object_new(MDNOTEBOOK_TYPE_ZOOMVIEW, NULL);
}

GtkTextView* mdnotebook_zoomview_get_textview(MdNotebookZoomView* self) {
	MdNotebookZoomViewPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_ZOOMVIEW(self), NULL);

	priv = mdnotebook_zoomview_get_instance_private(self);

	return GTK_TEXT_VIEW(priv->child);
}

void mdnotebook_zoomview_set_textview(MdNotebookZoomView* self, GtkTextView* view) {
	MdNotebookZoomViewPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_ZOOMVIEW(self));
	g_return_if_fail(view == NULL || GTK_IS_TEXT_VIEW(view));

	priv = mdnotebook_zoomview_get_instance_private(self);

	if (priv->child == GTK_WIDGET(view))
		return;

	if (priv->child)
		gtk_widget_unparent(priv->child);

	priv->child = GTK_WIDGET(view);

	if (priv->child)
		gtk_widget_set_parent(priv->child, GTK_WIDGET(self));

	/*GListModel* child_controllers = gtk_widget_observe_controllers(priv->child);
	GtkGestureClick* click_gesture = NULL;
	for (guint i = 0; i < g_list_model_get_n_items(child_controllers); i++) {
		GtkEventController* controller = GTK_EVENT_CONTROLLER(g_list_model_get_object(child_controllers, i));
		if (GTK_IS_GESTURE_CLICK(controller)) {
			click_gesture = GTK_GESTURE_CLICK(g_object_ref(controller));
			printf("found click handler\n");
			break;
		}
	}
	g_object_unref(child_controllers);
	priv->child_click_gesture = click_gesture;*/

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_CHILD]);
}

gdouble mdnotebook_zoomview_get_zoom(MdNotebookZoomView* self) {
	MdNotebookZoomViewPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_ZOOMVIEW(self), 0.);

	priv = mdnotebook_zoomview_get_instance_private(self);

	return priv->zoom;
}

void mdnotebook_zoomview_set_zoom(MdNotebookZoomView* self, gdouble zoom) {
	MdNotebookZoomViewPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_ZOOMVIEW(self));

	priv = mdnotebook_zoomview_get_instance_private(self);

	priv->zoom = zoom;
	priv->oldzoom = zoom;

	gtk_widget_queue_resize(GTK_WIDGET(self));

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_ZOOM]);
}
