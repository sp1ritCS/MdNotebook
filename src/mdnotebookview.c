#include "mdnotebookconfig.h"

#include "mdnotebookview.h"
#include "mdnotebookbuffer.h"
#include "mdnotebookbufferextra.h"
#include "bufitem/mdnotebookbufitem.h"
#include "bufitem/mdnotebookbufitemcodeblock.h"
#include "bufitem/mdnotebookbufitemdynblock.h"
#include "bufitem/mdnotebookbufitemheading.h"
#include "bufitem/mdnotebookbufitemtext.h"

#include "bufitem/mdnotebookbufitemcheckmark.h"

#ifdef MDNOTEBOOK_HAVE_LATEX
#include "bufitem/latex/mdnotebookbufitemlatex.h"
#include "bufitem/latex2/mdnotebookbufitemlatextwo.h"
#endif

#define _ __attribute__((unused))

static GtkTextBuffer* mdnotebook_view_create_buffer(GtkTextView*) {
	return mdnotebook_buffer_new(NULL);
}


typedef struct {
	GdkModifierType modifier_keys;
	guint latest_keyval;
} MdNotebookViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookView, mdnotebook_view, GTK_TYPE_TEXT_VIEW)

enum {
	HORIZONTAL_RESIZE,
	N_SIGNALS
};

static guint mdnotebook_view_signals[N_SIGNALS] = { 0, };

static void mdnotebook_view_size_allocate(GtkWidget* widget, int width, int height, int baseline) {
	GTK_WIDGET_CLASS(mdnotebook_view_parent_class)->size_allocate(widget, width, height, baseline);
	g_signal_emit(widget, mdnotebook_view_signals[HORIZONTAL_RESIZE], 0, width);
}

static void mdnotebook_view_class_init(MdNotebookViewClass* class) {
	GTK_TEXT_VIEW_CLASS(class)->create_buffer = mdnotebook_view_create_buffer;
	GTK_WIDGET_CLASS(class)->size_allocate = mdnotebook_view_size_allocate;

	mdnotebook_view_signals[HORIZONTAL_RESIZE] = g_signal_new("horizontal-resize",
		G_TYPE_FROM_CLASS(class), G_SIGNAL_RUN_LAST, 0,
		NULL, NULL, NULL,
		G_TYPE_NONE, 1, G_TYPE_INT);
}

static gboolean mdnotebook_view_key_pressed(_ GtkEventController* ctl, guint keyval, _ guint keycode, GdkModifierType state, MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), FALSE);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	priv->modifier_keys = state & (GDK_MODIFIER_MASK);
	priv->latest_keyval = keyval;

	return FALSE;
}

static void mdnotebook_view_init(MdNotebookView* self) {
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);
	MdNotebookBuffer* buffer = MDNOTEBOOK_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self)));

	priv->modifier_keys = 0;
	priv->latest_keyval = 0;

	GtkEventController* keyctl = gtk_event_controller_key_new();
	g_signal_connect(keyctl, "key-pressed", G_CALLBACK(mdnotebook_view_key_pressed), self);
	gtk_widget_add_controller(GTK_WIDGET(self), keyctl);

	MdNotebookBufItem* codeblock = mdnotebook_bufitem_codeblock_new();
	MdNotebookBufItem* title = mdnotebook_bufitem_heading_new();

#ifdef MDNOTEBOOK_HAVE_LATEX
	//MdNotebookBufItem* latex = mdnotebook_bufitem_latex_new(self);
	MdNotebookBufItem* latex = mdnotebook_bufitem_latex_two_new(self);
	mdnotebook_buffer_add_bufitem(buffer, latex);
#endif

	MdNotebookBufItem* checkmark = mdnotebook_bufitem_checkmark_new(self);
	MdNotebookBufItem* dynblock = mdnotebook_bufitem_dynblock_new();
	MdNotebookBufItem* text = mdnotebook_bufitem_text_new();
	mdnotebook_buffer_add_bufitem(buffer, codeblock);
	mdnotebook_buffer_add_bufitem(buffer, title);
	mdnotebook_buffer_add_bufitem(buffer, checkmark);
	mdnotebook_buffer_add_bufitem(buffer, dynblock);
	mdnotebook_buffer_add_bufitem(buffer, text);
}

GtkWidget* mdnotebook_view_new(void) {
	return g_object_new(MDNOTEBOOK_TYPE_VIEW, NULL);
}

GtkWidget* mdnotebook_view_new_with_buffer(MdNotebookBuffer* buffer) {
	GtkWidget* view = mdnotebook_view_new();
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), GTK_TEXT_BUFFER(buffer));

	return view;
}

GdkModifierType mdnotebook_view_get_modifier_keys(MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), 0);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	return priv->modifier_keys;
}

guint mdnotebook_view_get_latest_keyval(MdNotebookView* self) {
	g_return_val_if_fail(MDNOTEBOOK_IS_VIEW(self), 0);
	MdNotebookViewPrivate* priv = mdnotebook_view_get_instance_private(self);

	return priv->latest_keyval;
}
