#ifndef __GTKMDNOTEBOOKDRAWING_H__
#define __GTKMDNOTEBOOKDRAWING_H__

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define MDNOTEBOOK_STROKE_SIZE_MULTIPLIER 5.0

typedef struct {
	gdouble x;
	gdouble y;
	gdouble p;
} MdNotebookStrokeNode;
typedef struct {
	guint32 color;
	gsize num_nodes;
	gsize alloc_nodes;
	MdNotebookStrokeNode* nodes;
	gboolean selected;
} MdNotebookStroke;

typedef struct {
	gdouble x;
	gdouble y;
} MdNotebookBoundDrawingSelectionNode;

MdNotebookStroke* mdnotebook_stroke_new(guint32 color);
void mdnotebook_stroke_append_node(MdNotebookStroke* stroke, gdouble x, gdouble y, gdouble pressure);
void mdnotebook_stroke_force_min_xy(MdNotebookStroke* stroke, gdouble x, gdouble y);
gboolean mdnotebook_stroke_get_bbox(MdNotebookStroke* stroke, gdouble* x0, gdouble* x1, gdouble* y0, gdouble* y1);
gboolean mdnotebook_stroke_test_rectangle(MdNotebookStroke* stroke, gdouble x0, gdouble y0, gdouble x1, gdouble y1);
gboolean mdnotebook_stroke_select_area(MdNotebookStroke* stroke, const MdNotebookBoundDrawingSelectionNode* nodes, gsize num_nodes, gdouble ax, gdouble ay);
void mdnotebook_stroke_render(MdNotebookStroke* stroke, cairo_t* ctx, gboolean debug_mode);
void mdnotebook_stroke_render_selection_glow(MdNotebookStroke* stroke, cairo_t* ctx);
void mdnotebook_stroke_set_color(MdNotebookStroke* stroke, guint32 color);


#define MDNOTEBOOK_TYPE_BOUNDDRAWING (mdnotebook_bounddrawing_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBoundDrawing, mdnotebook_bounddrawing, MDNOTEBOOK, BOUNDDRAWING, GtkWidget)

struct _MdNotebookBoundDrawingClass {
	GtkWidgetClass parent_class;

	gpointer padding[12];
};

GtkWidget* mdnotebook_bounddrawing_new(void);

MdNotebookBoundDrawing* mdnotebook_bounddrawing_try_upcast(GtkWidget* w);

gboolean mdnotebook_bounddrawing_get_debug(MdNotebookBoundDrawing* self);
void mdnotebook_bounddrawing_set_debug(MdNotebookBoundDrawing* self, gboolean debug);

gboolean mdnotebook_bounddrawing_get_size(MdNotebookBoundDrawing* self, gint* width, gint* height);

gboolean mdnotebook_bounddrawing_update_size(MdNotebookBoundDrawing* self, gint neww, int newh, int dx, int dy);
gboolean mdnotebook_bounddrawing_add_stroke(MdNotebookBoundDrawing* self, MdNotebookStroke* stroke /* takes ownership */, gdouble dx, gdouble dy, gboolean force);

void mdnotebook_bounddrawing_erase_sqare_area(MdNotebookBoundDrawing* self, gdouble cx, gdouble cy, gdouble padding);

void mdnotebook_bounddrawing_select_area(MdNotebookBoundDrawing* self, MdNotebookBoundDrawingSelectionNode* nodes, gsize num_nodes, gdouble ax, gdouble ay);
void mdnotebook_bounddrawing_unselect(MdNotebookBoundDrawing* self);
void mdnotebook_bounddrawing_delete_selected(MdNotebookBoundDrawing* self);

G_END_DECLS

#endif // __GTKMDNOTEBOOKDRAWING_H__
