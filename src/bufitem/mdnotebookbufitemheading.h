#ifndef __MDNOTEBOOKBUFITEMHEADING_H__
#define __MDNOTEBOOKBUFITEMHEADING_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_HEADING (mdnotebook_bufitem_heading_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemHeading, mdnotebook_bufitem_heading, MDNOTEBOOK, BUFITEM_HEADING, MdNotebookBufItem)

struct _MdNotebookBufItemHeadingClass {
	MdNotebookBufItemClass parent_class;
};

MdNotebookBufItem* mdnotebook_bufitem_heading_new();

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMHEADING_H__
