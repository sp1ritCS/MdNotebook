#ifndef __MDNOTEBOOKLATEXEQUATION_H__
#define __MDNOTEBOOKLATEXEQUATION_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "mdnotebookview.h"

G_BEGIN_DECLS

#define MDNOTEBOOK_TYPE_LATEX_EQUATION (mdnotebook_latex_equation_get_type())
G_DECLARE_DERIVABLE_TYPE (MdNotebookLatexEquation, mdnotebook_latex_equation, MDNOTEBOOK, LATEX_EQUATION, GtkWidget)

struct _MdNotebookLatexEquationClass {
	GtkWidgetClass parent_class;

	gpointer padding[12];
};

GtkWidget* mdnotebook_latex_equation_new(MdNotebookView* view);

MdNotebookView* mdnotebook_latex_equation_get_textview(MdNotebookLatexEquation* self);
void mdnotebook_latex_equation_set_textview(MdNotebookLatexEquation* self, MdNotebookView* view);

gchar* mdnotebook_latex_equation_get_equation(MdNotebookLatexEquation* self);
void mdnotebook_latex_equation_set_equation(MdNotebookLatexEquation* self, const gchar* equation);

gfloat mdnotebook_latex_equation_get_width(MdNotebookLatexEquation* self);
void mdnotebook_latex_equation_set_width(MdNotebookLatexEquation* self, gfloat width);

gfloat mdnotebook_latex_equation_get_text_size(MdNotebookLatexEquation* self);
void mdnotebook_latex_equation_set_text_size(MdNotebookLatexEquation* self, gfloat text_size);

guint32 mdnotebook_latex_equation_get_color(MdNotebookLatexEquation* self);
void mdnotebook_latex_equation_set_color(MdNotebookLatexEquation* self, guint32 color);

gint64 mdnotebook_latex_equation_get_baseline(MdNotebookLatexEquation* self);

void mdnotebook_latex_equation_init_microtex();

G_END_DECLS

#endif // __MDNOTEBOOKLATEXEQUATION_H__
