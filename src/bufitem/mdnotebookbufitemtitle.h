#ifndef __MDNOTEBOOKBUFITEMTITLE_H__
#define __MDNOTEBOOKBUFITEMTITLE_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_TITLE (mdnotebook_bufitem_title_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemTitle, mdnotebook_bufitem_title, MDNOTEBOOK, BUFITEM_TITLE, GObject)

struct _MdNotebookBufItemTitleClass {
	GObjectClass parent_class;

	gpointer padding[12];
};

MdNotebookBufItem* mdnotebook_bufitem_title_new();

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMTITLE_H__
