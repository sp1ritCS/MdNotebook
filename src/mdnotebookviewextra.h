#ifndef __GTKMDNOTEBOOKVIEWEXTRA_H__
#define __GTKMDNOTEBOOKVIEWEXTRA_H__

#include "mdnotebookview.h"
#include "booktool/mdnotebookbooktool.h"

G_BEGIN_DECLS

gboolean mdnotebook_view_select_tool(MdNotebookView* self, MdNotebookBookTool* tool);
void mdnotebook_view_add_booktool(MdNotebookView* self, MdNotebookBookTool* tool);

G_END_DECLS

#endif // __GTKMDNOTEBOOKTOOLEXTRA_H__
