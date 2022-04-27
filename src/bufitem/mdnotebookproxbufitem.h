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
	gboolean (*test_widget) (MdNotebookProxBufItem* self, GtkWidget* widget);
	gint64 (*get_baseline) (MdNotebookProxBufItem* self, GtkWidget* widget);

	gpointer padding[12];
};

void mdnotebook_proxbufitem_bufitem_items_changed(MdNotebookProxBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);

gboolean mdnotebook_proxbufitem_test_iter_has_widget(const GtkTextIter* i);

GtkTextTag* mdnotebook_proxbufitem_get_invisible_tag(MdNotebookBuffer* self);

G_END_DECLS

#endif // __MDNOTEBOOKPROXBUFITEM_H__
