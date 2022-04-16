#include "bufitem/mdnotebookbufitem.h"

#define _ __attribute__((unused))

G_DEFINE_INTERFACE (MdNotebookBufItem, mdnotebook_bufitem, G_TYPE_OBJECT)

static void mdnotebook_bufitem_default_init(_ MdNotebookBufItemInterface* iface) {}

void mdnotebook_bufitem_init(MdNotebookBufItem* self, GtkTextBuffer* buffer) {
	MdNotebookBufItemInterface *iface;

	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM(self));

	iface = MDNOTEBOOK_BUFITEM_GET_IFACE(self);
 	g_return_if_fail (iface->init != NULL);
	iface->init(self, buffer);
}

void mdnotebook_bufitem_changed(MdNotebookBufItem *self, GtkTextBuffer* buffer, GtkTextIter start, GtkTextIter end) {
	MdNotebookBufItemInterface *iface;

	g_return_if_fail(MDNOTEBOOK_IS_BUFITEM(self));

	iface = MDNOTEBOOK_BUFITEM_GET_IFACE(self);
 	g_return_if_fail (iface->changed != NULL);
	iface->changed(self, buffer, start, end);
}
