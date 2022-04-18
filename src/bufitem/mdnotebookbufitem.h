#ifndef __MDNOTEBOOKBUFITEM_H__
#define __MDNOTEBOOKBUFITEM_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "gtkmdnotebookbuffer.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM (mdnotebook_bufitem_get_type())
G_DECLARE_INTERFACE (MdNotebookBufItem, mdnotebook_bufitem, MDNOTEBOOK, BUFITEM, GObject)

struct _MdNotebookBufItemInterface {
	GTypeInterface parent_iface;

	void (*init) (MdNotebookBufItem* self, MdNotebookBuffer* buffer);
	void (*cursor_changed) (MdNotebookBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);
	void (*buffer_changed) (MdNotebookBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);

	gpointer padding[12];
};

void mdnotebook_bufitem_init(MdNotebookBufItem* self, MdNotebookBuffer* buffer);
void mdnotebook_bufitem_cursor_changed(MdNotebookBufItem *self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);
void mdnotebook_bufitem_buffer_changed(MdNotebookBufItem *self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEM_H__
