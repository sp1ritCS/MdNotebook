#include "bufitem/latex/mdnotebooklatexequation.h"

#include <microtex.h>
#include <graphic_cairo.h>
#include <utils/exceptions.h>
namespace tex = microtex;

class MicroTeXErrorState {
public:
	tex::ex_tex exception;

	void (*ui_handler)(cairo_t*);

	gint width;
	gint height;
	gint baseline;

	MicroTeXErrorState(tex::ex_tex& exception, void (*fun)(cairo_t*), gint width = 128, gint height = 16, gint baseline = 0) :
		exception(std::move(exception)),
		ui_handler(fun),
		width(width),
		height(height),
		baseline(baseline) {}
};

#define _ __attribute__((unused))

typedef struct {
	gboolean shown;

	MdNotebookView* view;

	tex::Render* microtex;
	MicroTeXErrorState* error;

	gchar* equation;
	gfloat width;

	gfloat text_size;
	guint32 color;

} MdNotebookLatexEquationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookLatexEquation, mdnotebook_latex_equation, GTK_TYPE_WIDGET)

#define MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(obj) \
	static_cast<MdNotebookLatexEquationPrivate *>(mdnotebook_latex_equation_get_instance_private(MDNOTEBOOK_LATEX_EQUATION(obj)))

extern "C" void mdnotebook_latex_equation_view_resized(_ MdNotebookView* view, gint width, MdNotebookLatexEquation* self) {
	mdnotebook_latex_equation_set_width(self, width-64);
}

enum {
	PROP_TEXTVIEW = 1,
	PROP_EQUATION,
	PROP_WIDTH,
	PROP_TEXT_SIZE,
	PROP_COLOR,
	N_PROPERTIES
};

static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void mdnotebook_latex_equation_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
	MdNotebookLatexEquation* self = MDNOTEBOOK_LATEX_EQUATION(object);

	switch (prop_id) {
		case PROP_TEXTVIEW:
			g_value_set_object(value, mdnotebook_latex_equation_get_textview(self));
			break;
		case PROP_EQUATION:
			g_value_set_string(value, mdnotebook_latex_equation_get_equation(self));
			break;
		case PROP_WIDTH:
			g_value_set_float(value, mdnotebook_latex_equation_get_width(self));
			break;
		case PROP_TEXT_SIZE:
			g_value_set_float(value, mdnotebook_latex_equation_get_text_size(self));
			break;
		case PROP_COLOR:
			g_value_set_uint(value, mdnotebook_latex_equation_get_color(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

}

static void mdnotebook_latex_equation_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
	MdNotebookLatexEquation* self = MDNOTEBOOK_LATEX_EQUATION(object);

	switch (prop_id) {
		case PROP_TEXTVIEW:
			mdnotebook_latex_equation_set_textview(self, MDNOTEBOOK_VIEW(g_value_get_object(value)));
			break;
		case PROP_EQUATION:
			mdnotebook_latex_equation_set_equation(self, g_value_get_string(value));
			break;
		case PROP_WIDTH:
			mdnotebook_latex_equation_set_width(self, g_value_get_float(value));
			break;
		case PROP_TEXT_SIZE:
			mdnotebook_latex_equation_set_text_size(self, g_value_get_float(value));
			break;
		case PROP_COLOR:
			mdnotebook_latex_equation_set_color(self, g_value_get_uint(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void mdnotebook_latex_equation_dispose(GObject* object) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(object);

	if (priv->microtex)
		delete priv->microtex;
	if (priv->error)
		delete priv->error;
	if (priv->equation)
		g_free(priv->equation);

	g_signal_handlers_disconnect_by_func(priv->view, (gpointer)mdnotebook_latex_equation_view_resized, object);
	if (priv->view)
		g_object_unref(priv->view);

	G_OBJECT_CLASS(mdnotebook_latex_equation_parent_class)->dispose(object);
}

static GtkSizeRequestMode mdnotebook_latex_equation_get_request_mode(_ GtkWidget* widget) {
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}
static void mdnotebook_latex_equation_measure(GtkWidget* widget, GtkOrientation orientation, _ int for_size, int* min, int* nat, int* min_baseline, int* nat_baseline) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(widget);
	if (!priv->microtex) {
		g_info("uninitialized microtex\n");
		if (!priv->error) {
			g_critical("uninitialized microtex and error state\n");
			return;
		}

		if (orientation == GTK_ORIENTATION_HORIZONTAL) {
			*min = priv->error->width;
			*nat = priv->error->width;
		}
		if (orientation == GTK_ORIENTATION_VERTICAL) {
			*min = priv->error->height;
			*nat = priv->error->height;
			*min_baseline = priv->error->baseline;
			*nat_baseline = priv->error->baseline;
		}
		return;
	}

	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		*min = priv->microtex->getWidth() + 1;
		*nat = priv->microtex->getWidth() + 1;
	}
	if (orientation == GTK_ORIENTATION_VERTICAL) {
		*min = priv->microtex->getHeight() + 1;
		*nat = priv->microtex->getHeight() + 1;
		*min_baseline = priv->microtex->getBaseline();
		*nat_baseline = priv->microtex->getBaseline();
		// when there is no rendered LaTeX, µTeX sets baseline to -F_MAX
		if (*min_baseline == G_MININT32)
			*min_baseline = 0;
		if (*nat_baseline == G_MININT32)
			*nat_baseline = 0;
	}
}
static void mdnotebook_latex_equation_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(widget);

	gdouble
		width = gtk_widget_get_allocated_width(widget),
		height = gtk_widget_get_allocated_height(widget);

	graphene_rect_t rectangle;
	graphene_rect_t* rect = graphene_rect_init(&rectangle, 0, 0, width, height);
	// do I have to free this?

	cairo_t* ctx = gtk_snapshot_append_cairo(snapshot, rect);

	if (!priv->microtex) {
		g_info("uninitialized microtex\n");
		if (!priv->error) {
			g_critical("uninitialized microtex and error state\n");
			goto cairo_ctx_free;
		}
		priv->error->ui_handler(ctx);
	} else {
		microtex::Graphics2D_cairo g2(ctx);
		priv->microtex->draw(g2, 0, 0);
	}
cairo_ctx_free:
	cairo_destroy(ctx);
}

static void mdnotebook_latex_equation_create_render(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);
	GtkWidget* widget = GTK_WIDGET(self);

	if (!priv->shown)
		return;

	if (priv->microtex)
		delete priv->microtex;

	if (priv->error) {
		delete priv->error;
		priv->error = NULL;
	}

	if (priv->equation)
		try {
			priv->microtex = tex::MicroTeX::parse(std::string(priv->equation),
				mdnotebook_latex_equation_get_width(self),
				mdnotebook_latex_equation_get_text_size(self),
				mdnotebook_latex_equation_get_text_size(self) / 3.f,
				mdnotebook_latex_equation_get_color(self)
			);

			if (gtk_widget_get_has_tooltip(widget))
				gtk_widget_set_has_tooltip(widget, FALSE);

		} catch(tex::ex_parse &e) {
			g_warning("MicroTeX parsing error: %s\n", e.what());

			gtk_widget_set_tooltip_text(widget, e.what());

			priv->error = new MicroTeXErrorState(e, +[](cairo_t* ctx) {
				cairo_set_source_rgb(ctx, 1, 0, 0);
				cairo_set_font_size(ctx, 10);
				cairo_move_to(ctx, 0, 14);
				cairo_show_text(ctx, "LaTeX parse failed");

				cairo_fill(ctx);
			});

			priv->microtex = NULL;
		} catch(tex::ex_invalid_state &e) {
			g_warning("MicroTeX rendering error: %s\n", e.what());

			gtk_widget_set_tooltip_text(widget, e.what());

			priv->error = new MicroTeXErrorState(e, +[](cairo_t* ctx) {
				cairo_set_source_rgb(ctx, 1, 0, 0);
				cairo_set_font_size(ctx, 10);
				cairo_move_to(ctx, 0, 14);
				cairo_show_text(ctx, "LaTeX render failed");

				cairo_fill(ctx);
			});

			priv->microtex = NULL;
		} catch(tex::ex_tex &e) {
			g_warning("Unknown MicroTeX error: %s\n", e.what());

			gtk_widget_set_tooltip_text(widget, e.what());

			priv->error = new MicroTeXErrorState(e, +[](cairo_t* ctx) {
				cairo_set_source_rgb(ctx, 1, 0, 0);
				cairo_set_font_size(ctx, 10);
				cairo_move_to(ctx, 0, 14);
				cairo_show_text(ctx, "Unknown MicroTeX error");

				cairo_fill(ctx);
			});

			priv->microtex = NULL;
		}
	else
		priv->microtex = NULL;

	gtk_widget_queue_resize(GTK_WIDGET(self));
}

void mdnotebook_latex_equation_show(GtkWidget* widget) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(widget);
	priv->shown = TRUE;
	mdnotebook_latex_equation_create_render(MDNOTEBOOK_LATEX_EQUATION(widget));
	GTK_WIDGET_CLASS(mdnotebook_latex_equation_parent_class)->show(widget);
}
void mdnotebook_latex_equation_hide(GtkWidget* widget) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(widget);
	priv->shown = FALSE;
	GTK_WIDGET_CLASS(mdnotebook_latex_equation_parent_class)->hide(widget);
}

static void mdnotebook_latex_equation_class_init(MdNotebookLatexEquationClass* klass) {
	GObjectClass* object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

	object_class->get_property = mdnotebook_latex_equation_get_property;
	object_class->set_property = mdnotebook_latex_equation_set_property;
	object_class->dispose = mdnotebook_latex_equation_dispose;

	widget_class->get_request_mode = mdnotebook_latex_equation_get_request_mode;
	widget_class->measure = mdnotebook_latex_equation_measure;
	widget_class->snapshot = mdnotebook_latex_equation_snapshot;

	widget_class->show = mdnotebook_latex_equation_show;
	widget_class->hide = mdnotebook_latex_equation_hide;

	obj_properties[PROP_TEXTVIEW] = g_param_spec_object("textview", "TextView", "The Gtk.TextView this LatexEquation will be rendering in", MDNOTEBOOK_TYPE_VIEW, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_EXPLICIT_NOTIFY));
	obj_properties[PROP_EQUATION] = g_param_spec_string("latex", "LaTeX", "LaTeX equation", "", (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	obj_properties[PROP_WIDTH] = g_param_spec_float("width", "Width", "The maximal width the LaTeX equation may use horizontally", -1, G_MAXFLOAT, -1, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	obj_properties[PROP_TEXT_SIZE] = g_param_spec_float("text_size", "Textsize", "The text size to draw", 6, G_MAXFLOAT, 18, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	obj_properties[PROP_COLOR] = g_param_spec_uint("color", "Color", "The foreground color to draw", 0, G_MAXUINT32, 0xff000000, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void mdnotebook_latex_equation_init(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);
	priv->shown = TRUE;
	priv->view = NULL;
	priv->equation = NULL;
	priv->microtex = NULL;
	priv->width = -1;
	priv->text_size = 18;
	priv->color = 0xff000000;
}

extern "C" {
GtkWidget* mdnotebook_latex_equation_new(MdNotebookView* view) {
	return static_cast<GtkWidget *>(g_object_new(MDNOTEBOOK_TYPE_LATEX_EQUATION, "textview", view, NULL));
}

MdNotebookView* mdnotebook_latex_equation_get_textview(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self), NULL);

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	return priv->view;
}

void mdnotebook_latex_equation_set_textview(MdNotebookLatexEquation* self, MdNotebookView* view) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self));
	g_return_if_fail(view == NULL || MDNOTEBOOK_IS_VIEW(view));

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	if (priv->view == view)
		return;

	priv->view = g_object_ref(view);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXTVIEW]);
}

gchar* mdnotebook_latex_equation_get_equation(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self), NULL);

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	return priv->equation;
}

void mdnotebook_latex_equation_set_equation(MdNotebookLatexEquation* self, const gchar* equation) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self));

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	if (priv->equation == equation)
		return;

	if (g_strcmp0(priv->equation, equation) == 0)
		return;

	if (priv->equation)
		g_free(priv->equation);

	priv->equation = g_strdup(equation);

	mdnotebook_latex_equation_create_render(self);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_EQUATION]);
}

gfloat mdnotebook_latex_equation_get_width(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self), NAN);

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	return priv->width;
}

void mdnotebook_latex_equation_set_width(MdNotebookLatexEquation* self, gfloat width) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self));

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	if (priv->width == width)
		return;

	priv->width = width;

	mdnotebook_latex_equation_create_render(self);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_WIDTH]);
}

gfloat mdnotebook_latex_equation_get_text_size(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self), NAN);

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	return priv->text_size;
}

void mdnotebook_latex_equation_set_text_size(MdNotebookLatexEquation* self, gfloat text_size) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self));

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	if (priv->text_size == text_size)
		return;

	priv->text_size = text_size;

	if (priv->microtex) {
		priv->microtex->setTextSize(priv->text_size);
		gtk_widget_queue_resize(GTK_WIDGET(self));
	}

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_TEXT_SIZE]);
}

guint32 mdnotebook_latex_equation_get_color(MdNotebookLatexEquation* self) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_val_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self), 0xff000000);

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	return priv->color;
}

void mdnotebook_latex_equation_set_color(MdNotebookLatexEquation* self, guint32 color) {
	MdNotebookLatexEquationPrivate* priv;

	g_return_if_fail(MDNOTEBOOK_IS_LATEX_EQUATION(self));

	priv = MDNOTEBOOK_LATEX_EQUATION_GET_PRIVATE(self);

	if (priv->color == color)
		return;

	priv->color = color;

	if (priv->microtex) {
		priv->microtex->setForeground(priv->color);
		gtk_widget_queue_draw(GTK_WIDGET(self));
	}

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_COLOR]);
}

void mdnotebook_latex_equation_init_microtex() {
	if (tex::MicroTeX::isInited()) {
		g_warning("MicroTeX already initialized\n");
		return;
	}

	tex::InitFontSenseAuto autoinit;
	tex::MicroTeX::init(autoinit);
	tex::PlatformFactory::registerFactory("gtk", std::make_unique<tex::PlatformFactory_cairo>());
	tex::PlatformFactory::activate("gtk");
	tex::MicroTeX::overrideTexStyle(true, tex::TexStyle::display);
}
}
