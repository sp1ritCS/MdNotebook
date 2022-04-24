#ifndef __MDNOTEBOOKBUFITEMLATEXTWO_H__
#define __MDNOTEBOOKBUFITEMLATEXTWO_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookproxbufitem.h"
#include "bufitem/latex/mdnotebooklatexequation.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_LATEX_TWO (mdnotebook_bufitem_latex_two_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemLatexTwo, mdnotebook_bufitem_latex_two, MDNOTEBOOK, BUFITEM_LATEX_TWO, MdNotebookProxBufItem)

struct _MdNotebookBufItemLatexTwoClass {
	MdNotebookProxBufItemClass parent_class;
};

MdNotebookBufItem* mdnotebook_bufitem_latex_two_new(MdNotebookView* textview);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMLATEX_H__
