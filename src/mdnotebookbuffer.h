#ifndef __GTKMDNOTEBOOKBUFFER_H__
#define __GTKMDNOTEBOOKBUFFER_H__

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFFER (mdnotebook_buffer_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBuffer, mdnotebook_buffer, MDNOTEBOOK, BUFFER, GtkTextBuffer)

struct _MdNotebookBufferClass {
	GtkTextBufferClass parent_class;

	gpointer padding[12];
};

GtkTextBuffer* mdnotebook_buffer_new(GtkTextTagTable* table);

void mdnotebook_buffer_lock_bufchange(MdNotebookBuffer* self);
void mdnotebook_buffer_unlock_bufchange(MdNotebookBuffer* self);
#ifdef MDNOTEBOOK_BUFFER_EXPOSE_INTERNAS
gboolean* mdnotebook_buffer_get_bufchange_ptr(MdNotebookBuffer* self);
#endif

G_END_DECLS

#endif // __GTKMDNOTEBOOKBUFFER_H__
