#ifndef __GTKMDNOTEBOOKTOOLBAR_H__
#define __GTKMDNOTEBOOKTOOLBAR_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include <mdnotebookview.h>
#include <mdnotebookviewextra.h>

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_TOOLBAR (mdnotebook_toolbar_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookToolbar, mdnotebook_toolbar, MDNOTEBOOK, TOOLBAR, GtkBox)

struct _MdNotebookToolbarClass {
	GtkBoxClass parent_class;
};

GtkWidget* mdnotebook_toolbar_new(GListModel* tools);
GtkWidget* mdnotebook_toolbar_new_from_view(MdNotebookView* view);

GListModel* mdnotebook_toolbar_get_tools(MdNotebookToolbar* self);
void mdnotebook_toolbar_set_tools(MdNotebookToolbar* self, GListModel* tools);

MdNotebookView* mdnotebook_toolbar_get_view(MdNotebookToolbar* self);
void mdnotebook_toolbar_set_view(MdNotebookToolbar* self, MdNotebookView* view);

G_END_DECLS

#endif // __GTKMDNOTEBOOKTOOLBAR_H__
