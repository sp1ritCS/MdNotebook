#ifndef __MDNOTEBOOKBUFITEM_H__
#define __MDNOTEBOOKBUFITEM_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM (mdnotebook_bufitem_get_type())
G_DECLARE_INTERFACE (MdNotebookBufItem, mdnotebook_bufitem, MDNOTEBOOK, BUFITEM, GObject)

struct _MdNotebookBufItemInterface {
	GTypeInterface parent_iface;

	void (*init) (MdNotebookBufItem* self, GtkTextBuffer* buffer);
	void (*changed) (MdNotebookBufItem* self, GtkTextBuffer* buffer, GtkTextIter start, GtkTextIter end);

	gpointer padding[12];
};

void mdnotebook_bufitem_init(MdNotebookBufItem* self, GtkTextBuffer* buffer);
void mdnotebook_bufitem_changed(MdNotebookBufItem *self, GtkTextBuffer* buffer, GtkTextIter start, GtkTextIter end);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEM_H__
