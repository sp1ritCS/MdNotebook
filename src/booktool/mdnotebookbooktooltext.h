#ifndef __MDNOTEBOOKBOOKTOOLTEXT_H__
#define __MDNOTEBOOKBOOKTOOLTEXT_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "booktool/mdnotebookbooktool.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BOOKTOOL_TEXT (mdnotebook_booktool_text_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBookToolText, mdnotebook_booktool_text, MDNOTEBOOK, BOOKTOOL_TEXT, MdNotebookBookTool)

struct _MdNotebookBookToolTextClass {
	MdNotebookBookToolClass parent_class;
};

MdNotebookBookTool* mdnotebook_booktool_text_new(MdNotebookView* view);

G_END_DECLS

#endif // __MDNOTEBOOKBOOKTOOLTEXT_H__
