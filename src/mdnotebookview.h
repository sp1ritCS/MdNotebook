#ifndef __GTKMDNOTEBOOKVIEW_H__
#define __GTKMDNOTEBOOKVIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include <mdnotebookbuffer.h>

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_VIEW (mdnotebook_view_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookView, mdnotebook_view, MDNOTEBOOK, VIEW, GtkTextView)

struct _MdNotebookViewClass {
	GtkTextViewClass parent_class;

	gpointer padding[12];
};

GtkWidget* mdnotebook_view_new(void);
GtkWidget* mdnotebook_view_new_with_buffer(MdNotebookBuffer* buffer);

GdkModifierType mdnotebook_view_get_modifier_keys(MdNotebookView* self);
guint mdnotebook_view_get_latest_keyval(MdNotebookView* self);

G_END_DECLS

#endif // __GTKMDNOTEBOOKVIEW_H__
