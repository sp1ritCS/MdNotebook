#ifndef __MDNOTEBOOKBUFITEMCHECKMARK_H__
#define __MDNOTEBOOKBUFITEMCHECKMARK_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookproxbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_CHECKMARK (mdnotebook_bufitem_checkmark_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemCheckmark, mdnotebook_bufitem_checkmark, MDNOTEBOOK, BUFITEM_CHECKMARK, MdNotebookProxBufItem)

struct _MdNotebookBufItemCheckmarkClass {
	MdNotebookProxBufItemClass parent_class;
};

MdNotebookBufItem* mdnotebook_bufitem_checkmark_new(MdNotebookView* textview);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMLATEX_H__
