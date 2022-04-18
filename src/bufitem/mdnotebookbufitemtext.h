#ifndef __MDNOTEBOOKBUFITEMTEXT_H__
#define __MDNOTEBOOKBUFITEMTEXT_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_TEXT (mdnotebook_bufitem_text_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemText, mdnotebook_bufitem_text, MDNOTEBOOK, BUFITEM_TEXT, GObject)

struct _MdNotebookBufItemTextClass {
	GObjectClass parent_class;

	gpointer padding[12];
};

MdNotebookBufItem* mdnotebook_bufitem_text_new();

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMTEXT_H__
