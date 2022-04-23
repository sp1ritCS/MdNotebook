#ifndef __MDNOTEBOOKBUFITEMLATEX_H__
#define __MDNOTEBOOKBUFITEMLATEX_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bufitem/mdnotebookbufitem.h"
#include "mdnotebookview.h"
#include "bufitem/latex/mdnotebooklatexequation.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_BUFITEM_LATEX (mdnotebook_bufitem_latex_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookBufItemLatex, mdnotebook_bufitem_latex, MDNOTEBOOK, BUFITEM_LATEX, MdNotebookBufItem)

struct _MdNotebookBufItemLatexClass {
	MdNotebookBufItemClass parent_class;
};

MdNotebookBufItem* mdnotebook_bufitem_latex_new(MdNotebookView* textview);

MdNotebookView* mdnotebook_bufitem_latex_get_textview(MdNotebookBufItemLatex* self);
void mdnotebook_bufitem_latex_set_textview(MdNotebookBufItemLatex* self, MdNotebookView* view);

G_END_DECLS

#endif // __MDNOTEBOOKBUFITEMLATEX_H__
