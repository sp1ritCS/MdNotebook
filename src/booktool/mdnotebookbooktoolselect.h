#ifndef __MDNOTEBOOKBOOKTOOLSELECT_H__
#define __MDNOTEBOOKBOOKTOOLSELECT_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "booktool/mdnotebookbooktool.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BOOKTOOL_SELECT (mdnotebook_booktool_select_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBookToolSelect, mdnotebook_booktool_select, MDNOTEBOOK, BOOKTOOL_SELECT, MdNotebookBookTool)

struct _MdNotebookBookToolSelectClass {
	MdNotebookBookToolClass parent_class;
};

MdNotebookBookTool* mdnotebook_booktool_select_new(MdNotebookView* view);

G_END_DECLS

#endif // __MDNOTEBOOKBOOKTOOLSELECT_H__
