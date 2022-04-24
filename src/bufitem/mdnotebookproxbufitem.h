#ifndef __MDNOTEBOOKPROXBUFITEM_H__
#define __MDNOTEBOOKPROXBUFITEM_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_PROXBUFITEM (mdnotebook_proxbufitem_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookProxBufItem, mdnotebook_proxbufitem, MDNOTEBOOK, PROXBUFITEM, MdNotebookBufItem)

struct _MdNotebookProxBufItemClass {
	MdNotebookBufItemClass parent_class;

	GtkTextTag* (*tag) (MdNotebookProxBufItem* self);
	GtkWidget* (*render) (MdNotebookProxBufItem* self, const GtkTextIter* begin, const GtkTextIter* end);
	void (*update) (MdNotebookProxBufItem* self, GtkWidget* render, const GtkTextIter* begin, const GtkTextIter* end);

	gpointer padding[12];
};

G_END_DECLS

#endif // __MDNOTEBOOKPROXBUFITEM_H__
