#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "mdnotebooktoolbar.h"

#define _ __attribute__((unused))

typedef struct {
	GtkWidget* toolbox;
	GtkWidget* colorbox;
	GListModel* tools;
	MdNotebookView* view;
} MdNotebookToolbarPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookToolbar, mdnotebook_toolbar, GTK_TYPE_BOX)

enum {
	PROP_TOOLS = 1,
	PROP_VIEW,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_toolbar_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookToolbar* self = MDNOTEBOOK_TOOLBAR(object);

	switch (prop_id) {
		case PROP_TOOLS:
			g_value_set_object(value, mdnotebook_toolbar_get_tools(self));
			break;
		case PROP_VIEW:
			g_value_set_object(value, mdnotebook_toolbar_get_view(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_toolbar_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookToolbar* self = MDNOTEBOOK_TOOLBAR(object);

	switch (prop_id) {
		case PROP_TOOLS:
			mdnotebook_toolbar_set_tools(self, g_value_get_object(value));
			break;
		case PROP_VIEW:
			mdnotebook_toolbar_set_view(self, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_toolbar_dispose(GObject* object) {
	MdNotebookToolbar* self = MDNOTEBOOK_TOOLBAR(object);
	MdNotebookToolbarPrivate* priv = mdnotebook_toolbar_get_instance_private(self);

	if (priv->tools)
		g_clear_object(&priv->tools);
	if (priv->view)
		g_clear_object(&priv->view);

	G_OBJECT_CLASS(mdnotebook_toolbar_parent_class)->dispose(object);
}

static void mdnotebook_toolbar_class_init(MdNotebookToolbarClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);

	object_class->get_property = mdnotebook_toolbar_get_property;
	object_class->set_property = mdnotebook_toolbar_set_property;
	object_class->dispose = mdnotebook_toolbar_dispose;

	obj_properties[PROP_TOOLS] = g_param_spec_object("tools", "Tools", "The MdNotebook.BookTool list model from MdNotebook.View", G_TYPE_LIST_MODEL, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
	obj_properties[PROP_VIEW] = g_param_spec_object("view", "View", "The MdNotebook.View the toolbar is attached to", MDNOTEBOOK_TYPE_VIEW, G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_properties(object_class,  N_PROPERTIES, obj_properties);
}

static void mdnotebook_colorbtn_changed(GtkColorButton* btn, MdNotebookToolbar* self) {
	g_return_if_fail(MDNOTEBOOK_IS_TOOLBAR(self));
	MdNotebookView* view = mdnotebook_toolbar_get_view(self);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);

	GdkRGBA gcolor;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(btn), &gcolor);
	guint32 color = (guint8)(gcolor.alpha*255)<<24|(guint8)(gcolor.red*255)<<16|(guint8)(gcolor.green*255)<<8|(guint8)(gcolor.blue*255);
	mdnotebook_stroke_set_color(stroke_proxy->active, color);
}

static void mdnotebook_toolbar_init(MdNotebookToolbar* self) {
	MdNotebookToolbarPrivate* priv = mdnotebook_toolbar_get_instance_private(self);
	priv->tools = NULL;

	priv->toolbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	priv->colorbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_widget_set_margin_start(priv->toolbox, 6);
	gtk_widget_set_margin_end(priv->toolbox, 6);
	gtk_widget_set_margin_start(priv->colorbox, 6);
	gtk_widget_set_margin_end(priv->colorbox, 6);

	gtk_box_append(GTK_BOX(self), priv->toolbox);
	gtk_box_append(GTK_BOX(self), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	gtk_box_append(GTK_BOX(self), priv->colorbox);

	GdkRGBA black = {.red = 0, .green = 0, .blue = 0, .alpha = 1};
	GtkWidget* color_picker = gtk_color_button_new_with_rgba(&black);
	g_signal_connect(color_picker, "color-set", G_CALLBACK(mdnotebook_colorbtn_changed), self);
	gtk_box_append(GTK_BOX(self), color_picker);
}

static void mdnotebook_toolbar_toolbtn_pressed(GtkButton*, MdNotebookBookTool* tool) {
	mdnotebook_booktool_activate(tool);
}
static void mdnotebook_toolbar_resetup_tools(MdNotebookToolbar* self) {
	MdNotebookToolbarPrivate* priv = mdnotebook_toolbar_get_instance_private(self);

	GListModel* children = gtk_widget_observe_children(priv->toolbox);
	guint num_children = g_list_model_get_n_items(children);
	for (guint i = 0; i < num_children; i++)
		gtk_box_remove(GTK_BOX(priv->toolbox), GTK_WIDGET(g_list_model_get_object(children, i)));
	g_object_unref(children);

	guint num_tools = g_list_model_get_n_items(priv->tools);
	for (guint i = 0; i < num_tools; i++) {
		MdNotebookBookTool* tool = MDNOTEBOOK_BOOKTOOL(g_list_model_get_object(priv->tools, i));
		GtkWidget* btn = gtk_button_new();
		gtk_button_set_icon_name(GTK_BUTTON(btn), mdnotebook_booktool_icon_name(tool));
		g_signal_connect(btn, "clicked", G_CALLBACK(mdnotebook_toolbar_toolbtn_pressed), tool);
		gtk_box_append(GTK_BOX(priv->toolbox), btn);
	}
}
static void mdnotebook_toolbar_tools_changes(_ GListModel* tools, _ guint pos, _ guint removed, _ guint added, MdNotebookToolbar* self) {
	g_return_if_fail(MDNOTEBOOK_IS_TOOLBAR(self));
	mdnotebook_toolbar_resetup_tools(self);
}

GtkWidget* mdnotebook_toolbar_new(GListModel* tools) {
	return g_object_new(MDNOTEBOOK_TYPE_TOOLBAR, "orientation", GTK_ORIENTATION_HORIZONTAL, "tools", tools, NULL);
}
GtkWidget* mdnotebook_toolbar_new_from_view(MdNotebookView* view) {
	GListModel* tools = mdnotebook_view_get_tools(view);
	return g_object_new(MDNOTEBOOK_TYPE_TOOLBAR, "orientation", GTK_ORIENTATION_HORIZONTAL, "tools", tools, "view", view, NULL);
}

GListModel* mdnotebook_toolbar_get_tools(MdNotebookToolbar* self) {
	MdNotebookToolbarPrivate* priv;
	g_return_val_if_fail(MDNOTEBOOK_IS_TOOLBAR(self), NULL);
	priv = mdnotebook_toolbar_get_instance_private(self);

	return priv->tools;
}
void mdnotebook_toolbar_set_tools(MdNotebookToolbar* self, GListModel* tools) {
	MdNotebookToolbarPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_TOOLBAR(self));
	g_return_if_fail(G_IS_LIST_MODEL(tools));
	priv = mdnotebook_toolbar_get_instance_private(self);

	if (priv->tools)
		g_object_unref(priv->tools);

	priv->tools = g_object_ref(tools);
	mdnotebook_toolbar_resetup_tools(self);
	if (priv->tools)
		g_signal_connect(priv->tools, "items-changed", G_CALLBACK(mdnotebook_toolbar_tools_changes), self);
}

MdNotebookView* mdnotebook_toolbar_get_view(MdNotebookToolbar* self) {
	MdNotebookToolbarPrivate* priv;
	g_return_val_if_fail(MDNOTEBOOK_IS_TOOLBAR(self), NULL);
	priv = mdnotebook_toolbar_get_instance_private(self);

	return priv->view;
}
void mdnotebook_toolbar_set_view(MdNotebookToolbar* self, MdNotebookView* view) {
	MdNotebookToolbarPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_TOOLBAR(self));
	g_return_if_fail(MDNOTEBOOK_IS_VIEW(view));
	priv = mdnotebook_toolbar_get_instance_private(self);

	if (priv->view)
		g_object_unref(priv->view);

	priv->view = g_object_ref(view);
}
