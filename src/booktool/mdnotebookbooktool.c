#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "booktool/mdnotebookbooktool.h"
#include "mdnotebookviewextra.h"

#define _ __attribute__((unused))

typedef struct {
	MdNotebookView* view;
} MdNotebookBookToolPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (MdNotebookBookTool, mdnotebook_booktool, G_TYPE_OBJECT)

enum {
	PROP_TEXTVIEW = 1,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_booktool_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookBookTool* self = MDNOTEBOOK_BOOKTOOL(object);

	switch (prop_id) {
		case PROP_TEXTVIEW:
			g_value_set_object(value, mdnotebook_booktool_get_textview(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

}

static void mdnotebook_booktool_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookBookTool* self = MDNOTEBOOK_BOOKTOOL(object);

	switch (prop_id) {
		case PROP_TEXTVIEW:
			mdnotebook_booktool_set_textview(self, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_booktool_dispose(GObject* object) {
	MdNotebookBookToolPrivate* priv = mdnotebook_booktool_get_instance_private(MDNOTEBOOK_BOOKTOOL(object));

	if (priv->view)
		g_clear_object(&priv->view);

	G_OBJECT_CLASS(mdnotebook_booktool_parent_class)->dispose(object);
}

static void mdnotebook_booktool_class_init(MdNotebookBookToolClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);

	object_class->get_property = mdnotebook_booktool_get_property;
	object_class->set_property = mdnotebook_booktool_set_property;
	object_class->dispose = mdnotebook_booktool_dispose;

	class->icon_name = NULL;
	class->registered = NULL;
	class->activated = NULL;
	class->deactivated = NULL;
	class->gesture_start = NULL;
	class->gesture_end = NULL;
	class->gesture_move = NULL;

	obj_properties[PROP_TEXTVIEW] = g_param_spec_object("textview", "TextView", "The Gtk.TextView this BufItem will be rendering in", MDNOTEBOOK_TYPE_VIEW, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void mdnotebook_booktool_init(_ MdNotebookBookTool* self) {
	/*if (!mdnotebook_booktool_get_textview(self))
		g_critical("%s has no MdNotebook.View attached. This will cause issues.\n", g_type_name(G_OBJECT_TYPE(self)));*/
}

const gchar* mdnotebook_booktool_icon_name(MdNotebookBookTool* self) {
	MdNotebookBookToolClass* class;
	g_return_val_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self), NULL);

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	g_return_val_if_fail(class->icon_name != NULL, NULL);
	return class->icon_name(self);
}

void mdnotebook_booktool_registered(MdNotebookBookTool* self, MdNotebookView* view) {
	MdNotebookBookToolClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self));

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->registered != NULL)
		class->registered(self, view);
}
void mdnotebook_booktool_activated(MdNotebookBookTool* self, MdNotebookView* view) {
	MdNotebookBookToolClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self));

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->activated != NULL)
		class->activated(self, view);
}
void mdnotebook_booktool_deactivated(MdNotebookBookTool* self, MdNotebookView* view) {
	MdNotebookBookToolClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self));

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->deactivated != NULL)
		class->deactivated(self, view);
}
gboolean mdnotebook_booktool_gesture_start(MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookBookToolClass* class;
	g_return_val_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self), FALSE);

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->gesture_start != NULL)
		return class->gesture_start(self, x, y, pressure);
	else
		return FALSE;
}
gboolean mdnotebook_booktool_gesture_end(MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookBookToolClass* class;
	g_return_val_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self), FALSE);

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->gesture_end != NULL)
		return class->gesture_end(self, x, y, pressure);
	else
		return FALSE;
}
gboolean mdnotebook_booktool_gesture_move(MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookBookToolClass* class;
	g_return_val_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self), FALSE);

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->gesture_move != NULL)
		return class->gesture_move(self, x, y, pressure);
	else
		return FALSE;
}
void mdnotebook_booktool_render_surface(MdNotebookBookTool* self, cairo_t* ctx, gdouble x, gdouble y) {
	MdNotebookBookToolClass* class;
	g_return_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self));

	class = MDNOTEBOOK_BOOKTOOL_GET_CLASS(self);
	if (class->render_surface != NULL)
		class->render_surface(self, ctx, x, y);
}

MdNotebookView* mdnotebook_booktool_get_textview(MdNotebookBookTool* self) {
	MdNotebookBookToolPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self), NULL);

	priv = mdnotebook_booktool_get_instance_private(self);

	return priv->view;
}

void mdnotebook_booktool_set_textview(MdNotebookBookTool* self, MdNotebookView* view) {
	MdNotebookBookToolPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self));
	g_return_if_fail(view == NULL || MDNOTEBOOK_IS_VIEW(view));

	priv = mdnotebook_booktool_get_instance_private(self);

	if (priv->view == view)
		return;

	priv->view = g_object_ref(view);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXTVIEW]);
}

void mdnotebook_booktool_activate(MdNotebookBookTool* self) {
	g_return_if_fail(MDNOTEBOOK_IS_BOOKTOOL(self));

	MdNotebookView* view = mdnotebook_booktool_get_textview(self);
	mdnotebook_view_select_tool(view, self);
}
