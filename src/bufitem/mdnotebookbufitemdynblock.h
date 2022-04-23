#ifndef __MDNOTEBOOKBUFITEMDYNBLOCK_H__
#define __MDNOTEBOOKBUFITEMDYNBLOCK_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_DYNBLOCK (mdnotebook_bufitem_dynblock_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemDynBlock, mdnotebook_bufitem_dynblock, MDNOTEBOOK, BUFITEM_DYNBLOCK, MdNotebookBufItem)

struct _MdNotebookBufItemDynBlockClass {
	MdNotebookBufItemClass parent_class;
};

MdNotebookBufItem* mdnotebook_bufitem_dynblock_new();

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMDYNBLOCK_H__
