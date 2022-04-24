#ifndef __MDNOTEBOOKBUFITEM_H__
#define __MDNOTEBOOKBUFITEM_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "mdnotebookbuffer.h"
#include "mdnotebookview.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM (mdnotebook_bufitem_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItem, mdnotebook_bufitem, MDNOTEBOOK, BUFITEM, GObject)

struct _MdNotebookBufItemClass {
	GObjectClass parent_class;

	void (*registered) (MdNotebookBufItem* self, MdNotebookBuffer* buffer);
	void (*cursor_changed) (MdNotebookBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);
	void (*buffer_changed) (MdNotebookBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);
	void (*on_insert) (MdNotebookBufItem* self, MdNotebookBuffer* buffer, GtkTextMark* location, gchar* text, gint len);

	gpointer padding[12];
};

void mdnotebook_bufitem_push_iter(MdNotebookBufItem* self, const GtkTextIter* i);
void mdnotebook_bufitem_pop_iter(MdNotebookBufItem* self, GtkTextIter* i);

MdNotebookView* mdnotebook_bufitem_get_textview(MdNotebookBufItem* self);
void mdnotebook_bufitem_set_textview(MdNotebookBufItem* self, MdNotebookView* view);

MdNotebookBuffer* mdnotebook_bufitem_get_buffer(MdNotebookBufItem* self);

void mdnotebook_bufitem_registered(MdNotebookBufItem* self, MdNotebookBuffer* buffer);
void mdnotebook_bufitem_cursor_changed(MdNotebookBufItem *self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);
void mdnotebook_bufitem_buffer_changed(MdNotebookBufItem *self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end);
void mdnotebook_bufitem_on_insert(MdNotebookBufItem* self, MdNotebookBuffer* buffer, GtkTextMark* location, gchar* text, gint len);

GtkTextTag* mdnotebook_bufitem_get_private_tag(MdNotebookBuffer* self);
gboolean mdnotebook_bufitem_is_iter_in_private(MdNotebookBuffer* self, const GtkTextIter* it);
#ifdef MDNOTEBOOK_EXPOSE_INTERNAS
void mdnotebook_butitem_strip_private(MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end);
#endif
gboolean mdnotebook_bufitem_check_char(gunichar ch, gpointer user_data);
gboolean mdnotebook_bufitem_check_backward_whitespace(const GtkTextIter* ch);
gboolean mdnotebook_bufitem_get_tag_extends(const GtkTextIter* t, GtkTextTag* tag, GtkTextIter* left, GtkTextIter* right);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEM_H__
