#ifndef __GTKMDNOTEBOOKBUFWIDGET_H__
#define __GTKMDNOTEBOOKBUFWIDGET_H__

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFWIDGET (mdnotebook_bufwidget_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufWidget, mdnotebook_bufwidget, MDNOTEBOOK, BUFWIDGET, GtkWidget)

struct _MdNotebookBufWidgetClass {
	GtkWidgetClass parent_class;

	gpointer padding[12];
};

GtkWidget* mdnotebook_bufwidget_new(void);

GtkWidget* mdnotebook_bufwidget_get_child(MdNotebookBufWidget* self);
void mdnotebook_bufwidget_set_child(MdNotebookBufWidget* self, GtkWidget* child);

G_END_DECLS

#endif // __GTKMDNOTEBOOKBUFWIDGET_H__
