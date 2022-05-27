#ifndef __MDNOTEBOOKBUFITEMCODEBLOCK_H__
#define __MDNOTEBOOKBUFITEMCODEBLOCK_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_CODEBLOCK (mdnotebook_bufitem_codeblock_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemCodeblock, mdnotebook_bufitem_codeblock, MDNOTEBOOK, BUFITEM_CODEBLOCK, MdNotebookBufItem)

struct _MdNotebookBufItemCodeblockClass {
	MdNotebookBufItemClass parent_class;
};

MdNotebookBufItem* mdnotebook_bufitem_codeblock_new(MdNotebookView* view);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMCODEBLOCK_H__
