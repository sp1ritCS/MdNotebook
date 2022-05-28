#ifndef __MDNOTEBOOKBOOKTOOLERASER_H__
#define __MDNOTEBOOKBOOKTOOLERASER_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "booktool/mdnotebookbooktool.h"

G_BEGIN_DECLS

#define ERASER_OUTLINE 2.0

#define MDNOTEBOOK_TYPE_BOOKTOOL_ERASER (mdnotebook_booktool_eraser_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBookToolEraser, mdnotebook_booktool_eraser, MDNOTEBOOK, BOOKTOOL_ERASER, MdNotebookBookTool)

struct _MdNotebookBookToolEraserClass {
	MdNotebookBookToolClass parent_class;
};

MdNotebookBookTool* mdnotebook_booktool_eraser_new(MdNotebookView* view);

gdouble mdnotebook_booktool_eraser_get_size(MdNotebookBookToolEraser* self, gboolean include_outline);
void mdnotebook_booktool_eraser_get_area(MdNotebookBookToolEraser* self, gdouble cx, gdouble cy, gboolean include_outline, gdouble* x0, gdouble* y0, gdouble* size);

G_END_DECLS

#endif // __MDNOTEBOOKBOOKTOOLERASER_H__
