#ifndef __MDNOTEBOOKBOOKTOOL_H__
#define __MDNOTEBOOKBOOKTOOL_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "mdnotebookbuffer.h"
#include "mdnotebookview.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BOOKTOOL (mdnotebook_booktool_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBookTool, mdnotebook_booktool, MDNOTEBOOK, BOOKTOOL, GObject)

struct _MdNotebookBookToolClass {
	GObjectClass parent_class;

	const gchar* (*icon_name) (MdNotebookBookTool* self);

	void (*registered) (MdNotebookBookTool* self, MdNotebookView* view);
	void (*activated) (MdNotebookBookTool* self, MdNotebookView* view);
	void (*deactivated) (MdNotebookBookTool* self, MdNotebookView* view);
	gboolean (*gesture_start) (MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure);
	gboolean (*gesture_end) (MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure);
	gboolean (*gesture_move) (MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure);
	void (*render_pointer_texture) (MdNotebookBookTool* self, cairo_t* ctx, gdouble x, gdouble y);

	gpointer padding[12];
};

const gchar* mdnotebook_booktool_icon_name(MdNotebookBookTool* self);

void mdnotebook_booktool_registered(MdNotebookBookTool* self, MdNotebookView* view);
void mdnotebook_booktool_activated(MdNotebookBookTool* self, MdNotebookView* view);
void mdnotebook_booktool_deactivated(MdNotebookBookTool* self, MdNotebookView* view);
gboolean mdnotebook_booktool_gesture_start(MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure);
gboolean mdnotebook_booktool_gesture_end(MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure);
gboolean mdnotebook_booktool_gesture_move(MdNotebookBookTool* self, gdouble x, gdouble y, gdouble pressure);
void mdnotebook_booktool_render_pointer_texture(MdNotebookBookTool* self, cairo_t* ctx, gdouble x, gdouble y);

MdNotebookView* mdnotebook_booktool_get_textview(MdNotebookBookTool* self);
void mdnotebook_booktool_set_textview(MdNotebookBookTool* self, MdNotebookView* view);

void mdnotebook_booktool_activate(MdNotebookBookTool* self);

G_END_DECLS

#endif // __MDNOTEBOOKBOOKTOOL_H__
