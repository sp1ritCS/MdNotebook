#ifndef __GTKMDNOTEBOOKVIEW_H__
#define __GTKMDNOTEBOOKVIEW_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include <mdnotebookbuffer.h>
#include <mdnotebookdrawing.h>

G_BEGIN_DECLS

#ifdef MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
typedef struct {
	GtkWidget* overlay;
	MdNotebookStroke* active;
} MdNotebookViewStrokeProxy;
#endif // MDNOTEBOOK_VIEW_EXPOSE_INTERNAS

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

gboolean mdnotebook_view_select_tool_by_type(MdNotebookView* self, GType* tool);
GListModel* mdnotebook_view_get_tools(MdNotebookView* self);

void mdnotebook_view_set_cursor(MdNotebookView* self, GdkCursor* cursor);
void mdnotebook_view_set_cursor_from_name(MdNotebookView* self, const gchar* cursor);

void mdnotebook_view_redraw_overlay(MdNotebookView* self);

#ifdef MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
MdNotebookViewStrokeProxy* mdnotebook_view_get_stroke_proxy(MdNotebookView* self);
void mdnotebook_view_set_stylus_gesture_state(MdNotebookView* self, gboolean state);
#endif // MDNOTEBOOK_VIEW_EXPOSE_INTERNAS

G_END_DECLS

#endif // __GTKMDNOTEBOOKVIEW_H__
