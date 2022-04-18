#ifndef __GTKMDNOTEBOOKZOOMVIEW_H__
#define __GTKMDNOTEBOOKZOOMVIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include <mdnotebookbuffer.h>

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_ZOOMVIEW (mdnotebook_zoomview_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookZoomView, mdnotebook_zoomview, MDNOTEBOOK, ZOOMVIEW, GtkWidget)

struct _MdNotebookZoomViewClass {
	GtkWidgetClass parent_class;

	gpointer padding[12];
};

GtkWidget* mdnotebook_zoomview_new(void);

GtkTextView* mdnotebook_zoomview_get_textview(MdNotebookZoomView* self);
void mdnotebook_zoomview_set_textview(MdNotebookZoomView* self, GtkTextView* child);

gdouble mdnotebook_zoomview_get_zoom(MdNotebookZoomView* self);
void mdnotebook_zoomview_set_zoom(MdNotebookZoomView* self, gdouble zoom);

G_END_DECLS

#endif // __GTKMDNOTEBOOKZOOMVIEW_H__
