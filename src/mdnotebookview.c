#include "mdnotebookconfig.h"

#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "mdnotebookview.h"
#include "mdnotebookbuffer.h"
#include "mdnotebookbufferextra.h"
#include "bufitem/mdnotebookbufitem.h"
#include "bufitem/mdnotebookbufitemcodeblock.h"
#include "bufitem/mdnotebookbufitemdynblock.h"
#include "bufitem/mdnotebookbufitemheading.h"
#include "bufitem/mdnotebookbufitemtext.h"

#include "booktool/mdnotebookbooktool.h"
#include "booktool/mdnotebookbooktoolpen.h"
#include "booktool/mdnotebookbooktooltext.h"

#include "bufitem/mdnotebookbufitemcheckmark.h"

#ifdef MDNOTEBOOK_HAVE_LATEX
#include "bufitem/latex/mdnotebookbufitemlatex.h"
#include "bufitem/latex2/mdnotebookbufitemlatextwo.h"
#endif

#define _ __attribute__((unused))

static GtkTextBuffer* mdnotebook_view_create_buffer(GtkTextView*) {
	return mdnotebook_buffer_new(NULL);
}

typedef struct {
	gdouble x;
	gdouble y;
} MdNotebookViewPointerPosition;
typedef struct {
	GdkModifierType modifier_keys;
	guint latest_keyval;
	GListStore* booktools;
	MdNotebookBookTool* active_tool;
	GtkGesture* stylus_gesture;
	MdNotebookViewPointerPosition pointer_pos;
	MdNotebookViewStrokeProxy stroke_proxy;
} MdNotebookViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookView, mdnotebook_view, GTK_TYPE_TEXT_VIEW)

enum {
	HORIZONTAL_RESIZE,
	N_SIGNALS
};

static guint mdnotebook_view_signals[N_SIGNALS] = { 0, };

static void mdnotebook_view_stroke_proxy_draw_fun(_ GtkDrawingArea* area, cairo_t* ctx, _ int width, _ int height, MdNotebookView* view) {
	g_return_if_fail(MDNOTEBOOK_IS_VIEW(view));
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(view);

	mdnotebook_stroke_render(priv->stroke_proxy.active, ctx);

	mdnotebook_booktool_render_pointer_texture(priv->active_tool, ctx, priv->pointer_pos.x, priv->pointer_pos.y);
}

static void mdnotebook_view_dispose(GObject* object) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(MDNOTEBOOK_VIEW(object));

	g_clear_object(&priv->booktools);

	if (priv->stroke_proxy.active)
		g_free(g_steal_pointer(&priv->stroke_proxy.active));
	if (priv->stroke_proxy.overlay)
		g_object_unref(g_steal_pointer(&priv->stroke_proxy.overlay));

	G_OBJECT_CLASS(mdnotebook_view_parent_class)->dispose(object);
}

static void mdnotebook_view_size_allocate(GtkWidget* widget, int width, int height, int baseline) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(MDNOTEBOOK_VIEW(widget));
	GtkAllocation rect = { .x = 0, .y = 0, .width = width, .height = height };

	GTK_WIDGET_CLASS(mdnotebook_view_parent_class)->size_allocate(widget, width, height, baseline);
	g_signal_emit(widget, mdnotebook_view_signals[HORIZONTAL_RESIZE], 0, width);

	gtk_widget_size_allocate(priv->stroke_proxy.overlay, &rect, baseline);
}

static void mdnotebook_view_class_init(MdNotebookViewClass* class) {
	G_OBJECT_CLASS(class)->dispose = mdnotebook_view_dispose;
	GTK_TEXT_VIEW_CLASS(class)->create_buffer = mdnotebook_view_create_buffer;
	GTK_WIDGET_CLASS(class)->size_allocate = mdnotebook_view_size_allocate;

	mdnotebook_view_signals[HORIZONTAL_RESIZE] = g_signal_new("horizontal-resize",
		G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_LAST, 0,
		NULL, NULL, NULL,
		G_TYPE_NONE, 1, G_TYPE_INT);
}

static gboolean mdnotebook_view_key_pressed(_ GtkEventController* ctl, guint keyval, _ guint keycode, GdkModifierType state, MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), FALSE);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	priv->modifier_keys = state & (GDK_MODIFIER_MASK);
	priv->latest_keyval = keyval;

	return FALSE;
}

static void mdnotebook_view_pointer_motion(_ GtkEventControllerMotion* ctl, gdouble x, gdouble y, MdNotebookView* self) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	priv->pointer_pos.x = x;
	priv->pointer_pos.y = y;
}

static void mdnotebook_view_booktool_gesture_start(MdNotebookView* self, GtkGesture* gest, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);
	if (mdnotebook_booktool_gesture_start(priv->active_tool, x, y, pressure)) {
		gtk_gesture_set_state(gest, GTK_EVENT_SEQUENCE_CLAIMED);
	}
}
static void mdnotebook_view_booktool_gesture_end(MdNotebookView* self, GtkGesture* gest, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);
	if (mdnotebook_booktool_gesture_end(priv->active_tool, x, y, pressure)) {
		gtk_gesture_set_state(gest, GTK_EVENT_SEQUENCE_CLAIMED);
	}
}
static void mdnotebook_view_booktool_gesture_move(MdNotebookView* self, GtkGesture* gest, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);
	if (mdnotebook_booktool_gesture_move(priv->active_tool, x, y, pressure)) {
		gtk_gesture_set_state(gest, GTK_EVENT_SEQUENCE_CLAIMED);
	}
}


static void mdnotebook_view_drag_begin(GtkGestureDrag* gest, gdouble x, gdouble y, MdNotebookView* self) {
	mdnotebook_view_booktool_gesture_start(self, GTK_GESTURE(gest), x, y, 1.0);
}
static void mdnotebook_view_drag_end(GtkGestureDrag* gest, gdouble x_off, gdouble y_off, MdNotebookView* self) {
	gdouble x,y;
	gtk_gesture_drag_get_start_point(gest, &x, &y);
	mdnotebook_view_booktool_gesture_end(self, GTK_GESTURE(gest), x + x_off, y + y_off, 1.0);
}
static void mdnotebook_view_drag_update(GtkGestureDrag* gest, gdouble x_off, gdouble y_off, MdNotebookView* self) {
	gdouble x,y;
	gtk_gesture_drag_get_start_point(gest, &x, &y);
	mdnotebook_view_booktool_gesture_move(self, GTK_GESTURE(gest), x + x_off, y + y_off, 1.0);
}
static void mdnotebook_view_stylus_down(GtkGestureStylus* gest, gdouble x, gdouble y, MdNotebookView* self) {
	gdouble pressure;
		if (!gtk_gesture_stylus_get_axis(gest, GDK_AXIS_PRESSURE, &pressure))
			pressure = 1.0;
		mdnotebook_view_booktool_gesture_start(self, GTK_GESTURE(gest), x, y, pressure);
}
static void mdnotebook_view_stylus_up(GtkGestureStylus* gest, gdouble x, gdouble y, MdNotebookView* self) {
	gdouble pressure;
		if (!gtk_gesture_stylus_get_axis(gest, GDK_AXIS_PRESSURE, &pressure))
			pressure = 1.0;
		mdnotebook_view_booktool_gesture_end(self, GTK_GESTURE(gest), x, y, pressure);
}
static void mdnotebook_view_stylus_move(GtkGestureStylus* gest, gdouble x, gdouble y, MdNotebookView* self) {
	gdouble pressure;
		if (!gtk_gesture_stylus_get_axis(gest, GDK_AXIS_PRESSURE, &pressure))
			pressure = 1.0;
		mdnotebook_view_booktool_gesture_move(self, GTK_GESTURE(gest), x, y, pressure);
}

void mdnotebook_view_add_booktool(MdNotebookView* self, MdNotebookBookTool* tool);
static void mdnotebook_view_init(MdNotebookView* self) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);
	MdNotebookBuffer* buffer = MDNOTEBOOK_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self)));

	priv->modifier_keys = 0;
	priv->latest_keyval = 0;

	GtkEventController* keyctl = gtk_event_controller_key_new();
	g_signal_connect(keyctl, "key-pressed", G_CALLBACK(mdnotebook_view_key_pressed), self);
	gtk_widget_add_controller(GTK_WIDGET(self), keyctl);

	MdNotebookBufItem* codeblock = mdnotebook_bufitem_codeblock_new(self);
	MdNotebookBufItem* title = mdnotebook_bufitem_heading_new();

#ifdef MDNOTEBOOK_HAVE_LATEX
	//MdNotebookBufItem* latex = mdnotebook_bufitem_latex_new(self);
	MdNotebookBufItem* latex = mdnotebook_bufitem_latex_two_new(self);
	mdnotebook_buffer_add_bufitem(buffer, latex);
#endif

	MdNotebookBufItem* checkmark = mdnotebook_bufitem_checkmark_new(self);
	MdNotebookBufItem* dynblock = mdnotebook_bufitem_dynblock_new(self);
	MdNotebookBufItem* text = mdnotebook_bufitem_text_new();
	mdnotebook_buffer_add_bufitem(buffer, codeblock);
	mdnotebook_buffer_add_bufitem(buffer, title);
	mdnotebook_buffer_add_bufitem(buffer, checkmark);
	mdnotebook_buffer_add_bufitem(buffer, dynblock);
	mdnotebook_buffer_add_bufitem(buffer, text);

	priv->booktools = g_list_store_new(MDNOTEBOOK_TYPE_BOOKTOOL);
	MdNotebookBookTool* texttool = mdnotebook_booktool_text_new(self);
	MdNotebookBookTool* pentool = mdnotebook_booktool_pen_new(self);
	priv->active_tool = pentool;
	mdnotebook_view_add_booktool(self, texttool);
	mdnotebook_view_add_booktool(self, pentool);

	GtkEventController* motionctl = gtk_event_controller_motion_new();
	g_signal_connect(motionctl, "motion", G_CALLBACK(mdnotebook_view_pointer_motion), self);
	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(motionctl), GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(motionctl));

	GtkGesture* stylusctl = gtk_gesture_stylus_new();
	g_signal_connect(stylusctl, "down", G_CALLBACK(mdnotebook_view_stylus_down), self);
	g_signal_connect(stylusctl, "up", G_CALLBACK(mdnotebook_view_stylus_up), self);
	g_signal_connect(stylusctl, "motion", G_CALLBACK(mdnotebook_view_stylus_move), self);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(stylusctl));

	priv->stylus_gesture = stylusctl;

	GtkGesture* dragctl = gtk_gesture_drag_new();
	g_signal_connect(dragctl, "drag-begin", G_CALLBACK(mdnotebook_view_drag_begin), self);
	g_signal_connect(dragctl, "drag-end", G_CALLBACK(mdnotebook_view_drag_end), self);
	g_signal_connect(dragctl, "drag-update", G_CALLBACK(mdnotebook_view_drag_update), self);
	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(dragctl), GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(dragctl));

	priv->pointer_pos.x = -1;
	priv->pointer_pos.y = -1;

	// BEGIN Stroke Proxy
	priv->stroke_proxy.overlay = gtk_drawing_area_new();
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(priv->stroke_proxy.overlay), (GtkDrawingAreaDrawFunc)mdnotebook_view_stroke_proxy_draw_fun, self, NULL);
	gtk_text_view_add_overlay(GTK_TEXT_VIEW(self), priv->stroke_proxy.overlay, 0, 0);

	mdnotebook_booktool_activated(priv->active_tool, self);

	priv->stroke_proxy.active = mdnotebook_stroke_new(0xff000000);
}

GtkWidget* mdnotebook_view_new(void) {
	return g_object_new(MDNOTEBOOK_TYPE_VIEW, NULL);
}

GtkWidget* mdnotebook_view_new_with_buffer(MdNotebookBuffer* buffer) {
	GtkWidget* view = mdnotebook_view_new();
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), GTK_TEXT_BUFFER(buffer));

	return view;
}

GdkModifierType mdnotebook_view_get_modifier_keys(MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), 0);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	return priv->modifier_keys;
}

guint mdnotebook_view_get_latest_keyval(MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), 0);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	return priv->latest_keyval;
}

static gboolean mdnotebook_view_cmp_booktool_type(gconstpointer lhs, gconstpointer rhs) {
	return G_OBJECT_TYPE(lhs) == G_OBJECT_TYPE(rhs);
}
void mdnotebook_view_add_booktool(MdNotebookView* self, MdNotebookBookTool* tool) {
	MdNotebookViewPrivate* priv;
	g_return_if_fail(MDNOTEBOOK_IS_VIEW(self));
	priv = mdnotebook_view_get_instance_private(self);

	if (g_list_store_find_with_equal_func(priv->booktools, tool, mdnotebook_view_cmp_booktool_type, NULL)) {
		g_warning("%s is already registered in the MdNotebook.View\n", g_type_name(G_OBJECT_TYPE(tool)));
	} else {
		g_list_store_append(priv->booktools, tool);
		mdnotebook_booktool_registered(tool, self);
	}

	g_object_unref(tool);
}

gboolean mdnotebook_view_select_tool(MdNotebookView* self, MdNotebookBookTool* tool) {
	MdNotebookViewPrivate* priv;
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), FALSE);
	priv = mdnotebook_view_get_instance_private(self);

	if (g_list_store_find(priv->booktools, tool, NULL)) {
		mdnotebook_booktool_deactivated(priv->active_tool, self);
		priv->active_tool = tool;
		mdnotebook_booktool_activated(priv->active_tool, self);
		return TRUE;
	} else {
		g_warning("Tried to select unregistered tool %s\n", g_type_name(G_OBJECT_TYPE(tool)));
		return FALSE;
	}
}
static gboolean mdnotebook_view_cmp_booktool_type_t(gconstpointer lhs, gconstpointer rhs) {
	return G_OBJECT_TYPE(lhs) == *(GType*)rhs;
}
gboolean mdnotebook_view_select_tool_by_type(MdNotebookView* self, GType* tool) {
	MdNotebookViewPrivate* priv;
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), FALSE);
	priv = mdnotebook_view_get_instance_private(self);

	guint position;
	if (g_list_store_find_with_equal_func(priv->booktools, tool, mdnotebook_view_cmp_booktool_type_t, &position)) {
		mdnotebook_booktool_deactivated(priv->active_tool, self);
		priv->active_tool = MDNOTEBOOK_BOOKTOOL(g_list_model_get_object(G_LIST_MODEL(priv->booktools), position));
		mdnotebook_booktool_activated(priv->active_tool, self);
		return TRUE;
	} else {
		g_warning("Tried to select unregistered tool %s\n", g_type_name(*tool));
		return FALSE;
	}
}

GListModel* mdnotebook_view_get_tools(MdNotebookView* self) {
	MdNotebookViewPrivate* priv;
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), NULL);
	priv = mdnotebook_view_get_instance_private(self);

	return G_LIST_MODEL(priv->booktools);
}

void mdnotebook_view_set_cursor(MdNotebookView* self, GdkCursor* cursor) {
	MdNotebookViewStrokeProxy* prox = mdnotebook_view_get_stroke_proxy(self);
	if (!prox)
		return;

	gtk_widget_set_cursor(prox->overlay, cursor);
	gtk_widget_set_cursor(GTK_WIDGET(self), cursor);
}
void mdnotebook_view_set_cursor_from_name(MdNotebookView* self, const gchar* cursor) {
	MdNotebookViewStrokeProxy* prox = mdnotebook_view_get_stroke_proxy(self);
	if (!prox)
		return;

	gtk_widget_set_cursor_from_name(prox->overlay, cursor);
	gtk_widget_set_cursor_from_name(GTK_WIDGET(self), cursor);
}

MdNotebookViewStrokeProxy* mdnotebook_view_get_stroke_proxy(MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), NULL);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	return &priv->stroke_proxy;
}

void mdnotebook_view_set_stylus_gesture_state(MdNotebookView* self, gboolean state) {
	g_return_if_fail(MDNOTEBOOK_IS_VIEW(self));
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	if (state)
		gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(priv->stylus_gesture), GTK_PHASE_CAPTURE);
	else
		gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(priv->stylus_gesture), GTK_PHASE_NONE);
}
