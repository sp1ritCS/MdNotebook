#ifndef __GTKMDNOTEBOOKDRAWING_H__
#define __GTKMDNOTEBOOKDRAWING_H__

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

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
} MdNotebookStroke;

MdNotebookStroke* mdnotebook_stroke_new(guint32 color);
void mdnotebook_stroke_append_node(MdNotebookStroke* stroke, gdouble x, gdouble y, gdouble pressure);
void mdnotebook_stroke_force_min_xy(MdNotebookStroke* stroke, gdouble x, gdouble y);
gboolean mdnotebook_stroke_get_bbox(MdNotebookStroke* stroke, gdouble* x0, gdouble* x1, gdouble* y0, gdouble* y1);
void mdnotebook_stroke_render(MdNotebookStroke* stroke, cairo_t* ctx);
void mdnotebook_stroke_set_color(MdNotebookStroke* stroke, guint32 color);


#define MDNOTEBOOK_TYPE_BOUNDDRAWING (mdnotebook_bounddrawing_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBoundDrawing, mdnotebook_bounddrawing, MDNOTEBOOK, BOUNDDRAWING, GtkWidget)

struct _MdNotebookBoundDrawingClass {
	GtkWidgetClass parent_class;

	gpointer padding[12];
};

GtkWidget* mdnotebook_bounddrawing_new(void);

MdNotebookBoundDrawing* mdnotebook_bounddrawing_try_upcast(GtkWidget* w);

gboolean mdnotebook_bounddrawing_get_size(MdNotebookBoundDrawing* self, gint* width, gint* height);

gboolean mdnotebook_bounddrawing_update_size(MdNotebookBoundDrawing* self, gint neww, int newh, int dx, int dy);
gboolean mdnotebook_bounddrawing_add_stroke(MdNotebookBoundDrawing* self, MdNotebookStroke* stroke /* takes ownership */, gdouble dx, gdouble dy, gboolean force);

G_END_DECLS

#endif // __GTKMDNOTEBOOKDRAWING_H__
