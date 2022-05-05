#ifndef __MDNOTEBOOKBOOKTOOLPEN_H__
#define __MDNOTEBOOKBOOKTOOLPEN_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "booktool/mdnotebookbooktool.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BOOKTOOL_PEN (mdnotebook_booktool_pen_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBookToolPen, mdnotebook_booktool_pen, MDNOTEBOOK, BOOKTOOL_PEN, MdNotebookBookTool)

struct _MdNotebookBookToolPenClass {
	MdNotebookBookToolClass parent_class;
};

MdNotebookBookTool* mdnotebook_booktool_pen_new(MdNotebookView* view);

G_END_DECLS

#endif // __MDNOTEBOOKBOOKTOOLPEN_H__
