#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "booktool/mdnotebookbooktooleraser.h"

#include "bufitem/mdnotebookproxbufitem.h"

#define _ __attribute__((unused))

typedef struct {
	gdouble eraser_padding;

	gboolean in_gesture;
} MdNotebookBookToolEraserPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MdNotebookBookToolEraser, mdnotebook_booktool_eraser, MDNOTEBOOK_TYPE_BOOKTOOL)

static const gchar* mdnotebook_booktool_eraser_booktool_icon_name(MdNotebookBookTool*) {
	return "tool-eraser-symbolic";
}

static void mdnotebook_booktool_eraser_booktool_activated(MdNotebookBookTool*, MdNotebookView* view) {
	mdnotebook_view_set_cursor_from_name(view, "cell");
}
static void mdnotebook_booktool_eraser_booktool_deactivated(MdNotebookBookTool*, MdNotebookView* view) {
	mdnotebook_view_set_cursor_from_name(view, NULL);
}

static void mdnotebook_booktool_eraser_erase_at_position(MdNotebookBookToolEraser* self, gdouble x, gdouble y, gdouble padding) {
	MdNotebookView* view = mdnotebook_booktool_get_textview(MDNOTEBOOK_BOOKTOOL(self));
	GtkTextView* gview = (GtkTextView*)view;
	GtkTextBuffer* buf = gtk_text_view_get_buffer(gview);
	GtkTextIter i,j;
	gtk_text_view_get_iter_at_position(gview, &i, NULL, x - padding, y - padding);
	gtk_text_view_get_iter_at_position(gview, &j, NULL, x + padding, y + padding);

	GtkTextIter end;
	gtk_text_buffer_get_end_iter(buf, &end);
	do {
		GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&i);
		if (anch) {
			guint len;
			GtkWidget** w = gtk_text_child_anchor_get_widgets(anch, &len);
			if (len) {
				if (MDNOTEBOOK_IS_BOUNDDRAWING(w[0])) {
					GtkAllocation alloc;
					gtk_widget_get_allocation(w[0], &alloc);
					mdnotebook_bounddrawing_erase_sqare_area(MDNOTEBOOK_BOUNDDRAWING(w[0]), x-alloc.x, y-alloc.y, padding);
				}
			}
		}

		if (gtk_text_iter_compare(&i, &j) == 0) break;
	} while (gtk_text_iter_forward_char(&i));
}

static gboolean mdnotebook_booktool_eraser_booktool_gesture_start(MdNotebookBookTool* tool, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookBookToolEraser* self = MDNOTEBOOK_BOOKTOOL_ERASER(tool);
	MdNotebookBookToolEraserPrivate* priv = mdnotebook_booktool_eraser_get_instance_private(self);
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);

	priv->in_gesture = TRUE;
	mdnotebook_view_set_cursor_from_name(view, "none");

	mdnotebook_booktool_eraser_erase_at_position(self, x, y, mdnotebook_booktool_eraser_get_size(self, true) * pressure);

	mdnotebook_view_redraw_overlay(view);
	return TRUE;
}
static gboolean mdnotebook_booktool_eraser_booktool_gesture_move(MdNotebookBookTool* tool, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookBookToolEraser* self = MDNOTEBOOK_BOOKTOOL_ERASER(tool);
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);

	mdnotebook_booktool_eraser_erase_at_position(self, x, y, mdnotebook_booktool_eraser_get_size(self, true) * pressure);

	mdnotebook_view_redraw_overlay(view);
	return TRUE;
}
static gboolean mdnotebook_booktool_eraser_booktool_gesture_end(MdNotebookBookTool* tool, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookBookToolEraser* self = MDNOTEBOOK_BOOKTOOL_ERASER(tool);
	MdNotebookBookToolEraserPrivate* priv = mdnotebook_booktool_eraser_get_instance_private(self);
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);

	priv->in_gesture = FALSE;
	mdnotebook_view_set_cursor_from_name(view, "cell");

	mdnotebook_booktool_eraser_erase_at_position(self, x, y, mdnotebook_booktool_eraser_get_size(self, true) * pressure);

	mdnotebook_view_redraw_overlay(view);
	return TRUE;
}

static void mdnotebook_booktool_eraser_booktool_render_pointer_texture(MdNotebookBookTool* tool, cairo_t* ctx, gdouble x, gdouble y) {
	MdNotebookBookToolEraserPrivate* priv = mdnotebook_booktool_eraser_get_instance_private(MDNOTEBOOK_BOOKTOOL_ERASER(tool));

	if (!priv->in_gesture)
		return;

	cairo_set_line_width(ctx, 1.0);
	cairo_set_source_rgba(ctx,0.8, 0.1, 0.0, 0.5);

	gdouble x0,y0,size;
	mdnotebook_booktool_eraser_get_area(MDNOTEBOOK_BOOKTOOL_ERASER(tool), x, y, false, &x0, &y0, &size);
	cairo_rectangle(ctx, x0, y0, size, size);
	cairo_stroke_preserve(ctx);
	cairo_set_source_rgba(ctx, 0.7, 0.2, 0.1, 0.5);
	cairo_fill(ctx);
}

static void mdnotebook_booktool_eraser_class_init(MdNotebookBookToolEraserClass* class) {
	MdNotebookBookToolClass* booktool_class = MDNOTEBOOK_BOOKTOOL_CLASS(class);
	booktool_class->icon_name = mdnotebook_booktool_eraser_booktool_icon_name;
	booktool_class->activated= mdnotebook_booktool_eraser_booktool_activated;
	booktool_class->deactivated= mdnotebook_booktool_eraser_booktool_deactivated;
	booktool_class->gesture_start = mdnotebook_booktool_eraser_booktool_gesture_start;
	booktool_class->gesture_end = mdnotebook_booktool_eraser_booktool_gesture_end;
	booktool_class->gesture_move = mdnotebook_booktool_eraser_booktool_gesture_move;
	booktool_class->render_pointer_texture = mdnotebook_booktool_eraser_booktool_render_pointer_texture;
}

static void mdnotebook_booktool_eraser_init(MdNotebookBookToolEraser* self) {
	MdNotebookBookToolEraserPrivate* priv = mdnotebook_booktool_eraser_get_instance_private(self);

	priv->eraser_padding = 7.5;
	priv->in_gesture = FALSE;
}

MdNotebookBookTool* mdnotebook_booktool_eraser_new(MdNotebookView* view) {
	return g_object_new(MDNOTEBOOK_TYPE_BOOKTOOL_ERASER, "textview", view, NULL);
}

gdouble mdnotebook_booktool_eraser_get_size(MdNotebookBookToolEraser* self, gboolean incl_outline) {
	MdNotebookBookToolEraserPrivate* priv;
	g_return_val_if_fail(MDNOTEBOOK_IS_BOOKTOOL_ERASER(self), 0.0);
	priv = mdnotebook_booktool_eraser_get_instance_private(self);

	return 1.0/*stroke_width*/*priv->eraser_padding+((gdouble)(gint)incl_outline*ERASER_OUTLINE);
}
void mdnotebook_booktool_eraser_get_area(MdNotebookBookToolEraser* self, gdouble cx, gdouble cy, gboolean incl_outline, gdouble* x0, gdouble* y0, gdouble* size) {
	gdouble final_padding = mdnotebook_booktool_eraser_get_size(self, incl_outline);
	*x0 = cx - final_padding;
	*y0 = cy - final_padding;
	// final_padding*2+(incl_outline ? 0.5 : 0)
	*size = final_padding*2+((gdouble)(gint)incl_outline)*0.5;
}
