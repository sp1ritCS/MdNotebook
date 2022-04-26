#include "bufitem/mdnotebookbufitemcheckmark.h"

#define _ __attribute__((unused))

typedef struct {
	GtkTextTag* checkmark;
	gint counter;
} MdNotebookBufItemCheckmarkPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MdNotebookBufItemCheckmark, mdnotebook_bufitem_checkmark, MDNOTEBOOK_TYPE_PROXBUFITEM)

static void mdnotebook_bufitem_checkmark_object_dispose(GObject* object) {
	MdNotebookBufItemCheckmarkPrivate* priv = mdnotebook_bufitem_checkmark_get_instance_private(MDNOTEBOOK_BUFITEM_CHECKMARK(object));

	if (priv->checkmark)
		g_object_unref(priv->checkmark);
}

static GtkTextTag* mdnotebook_bufitem_checkmark_proxbufitem_tag(MdNotebookProxBufItem* self) {
	MdNotebookBufItemCheckmarkPrivate* priv = mdnotebook_bufitem_checkmark_get_instance_private(MDNOTEBOOK_BUFITEM_CHECKMARK(self));
	if (priv->checkmark)
		return priv->checkmark;

	GtkTextBuffer* buf = GTK_TEXT_BUFFER(mdnotebook_bufitem_get_buffer(MDNOTEBOOK_BUFITEM(self)));
	priv->checkmark = gtk_text_buffer_create_tag(buf, "mdnb:checkmark",
		"foreground", "#A0A8C0",
		"weight", PANGO_WEIGHT_BOLD,
	NULL);

	return priv->checkmark;
}

static gboolean mdnotebook_bufitem_checkmark_test_nonwhitespace(gunichar ch, gpointer) {
	return ch != ' ' && ch != '\t';
}

static gboolean mdnotebook_bufitem_checkmark_test_nonnumber(gunichar ch, gpointer) {
	return ch <= 0x30 || ch >= 0x39;
}

static gboolean mdnotebook_bufitem_checkmark_is_valid_enumeration(GtkTextIter* i, const GtkTextIter* end) {
	gunichar n = gtk_text_iter_get_char(i);
	if (n == '*' || n == '-') {
		gtk_text_iter_forward_char(i);
		if (gtk_text_iter_get_char(i) == ' ') {
			gtk_text_iter_forward_char(i);
			return TRUE;
		} else {
			return FALSE;
		}
	} else if (n >= 0x30 && n <= 0x39) {
		if (!gtk_text_iter_forward_find_char(i, mdnotebook_bufitem_checkmark_test_nonnumber, NULL, end))
			return FALSE;

		if (gtk_text_iter_get_char(i) != '.')
			return FALSE;

		gtk_text_iter_forward_char(i);
		if (gtk_text_iter_get_char(i) != ' ')
			return FALSE;

		gtk_text_iter_forward_char(i);
		return TRUE;
	} else {
		return FALSE;
	}
}

static void mdnotebook_bufitem_checkmark_apply_checkmarked_list_items(MdNotebookBufItemCheckmark* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	MdNotebookBufItemCheckmarkPrivate* priv = mdnotebook_bufitem_checkmark_get_instance_private(self);
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(buffer);
	GtkTextTag* checkmarktag = mdnotebook_bufitem_checkmark_proxbufitem_tag((MdNotebookProxBufItem*)self);
	GtkTextIter active = *start;

	gint counter = 0;

	while (true) {
		if (mdnotebook_bufitem_is_iter_in_private(buffer, &active))
			goto skip_check;
		gunichar c = gtk_text_iter_get_char(&active);
		if (c == ' ' || c == '\t') {
			if (!gtk_text_iter_forward_find_char(&active, mdnotebook_bufitem_checkmark_test_nonwhitespace, NULL, end))
				break;
		}

		if (mdnotebook_bufitem_checkmark_is_valid_enumeration(&active, end)) {
			GtkTextIter begin = active;
			if (gtk_text_iter_get_char(&active) != '[')
				goto skip_check;
			gtk_text_iter_forward_char(&active);

			gunichar t = gtk_text_iter_get_char(&active);
			if (t != 'X' && t != 'x'  && t != ' ')
				goto skip_check;
			gtk_text_iter_forward_char(&active);

			if (gtk_text_iter_get_char(&active) != ']')
				goto skip_check;
			gtk_text_iter_forward_char(&active);

			if (gtk_text_iter_get_char(&active) != ' ')
				goto skip_check;

			gtk_text_buffer_apply_tag(buf, checkmarktag, &begin, &active);
			counter++;
		}

skip_check:
		if (!gtk_text_iter_forward_line(&active))
			break;
	}

	if (priv->counter != counter) {
		priv->counter = counter;
		mdnotebook_proxbufitem_bufitem_items_changed(MDNOTEBOOK_PROXBUFITEM(self), buffer, start, end);
	}
}

static void mdnotebook_bufitem_checkmark_bufitem_buffer_changed(MdNotebookBufItem* self, MdNotebookBuffer* buffer, const GtkTextIter* start, const GtkTextIter* end) {
	MdNotebookBufItemCheckmarkPrivate* priv = mdnotebook_bufitem_checkmark_get_instance_private(MDNOTEBOOK_BUFITEM_CHECKMARK(self));
	if (priv->checkmark)
		gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(buffer), priv->checkmark, start, end);

	mdnotebook_bufitem_checkmark_apply_checkmarked_list_items(MDNOTEBOOK_BUFITEM_CHECKMARK(self), buffer, start, end);
}

static void mdnotebook_bufitem_checkmark_toggled(GtkCheckButton* self, MdNotebookView* view) {
	printf("toggled\n");
}
static GtkWidget* mdnotebook_bufitem_checkmark_proxbufitem_render(MdNotebookProxBufItem* self, const GtkTextIter* begin, const GtkTextIter* end) {
	MdNotebookView* view = mdnotebook_bufitem_get_textview(MDNOTEBOOK_BUFITEM(self));
	gchar* str = gtk_text_iter_get_text(begin, end);
	GtkWidget* eq = gtk_check_button_new();
	if (g_utf8_strlen(str, -1) >= 2) {
		gtk_check_button_set_active(GTK_CHECK_BUTTON(eq), str[1] == 'X' || str[1] == 'x');
	}
	g_free(str);
	g_signal_connect(eq, "toggled", G_CALLBACK(mdnotebook_bufitem_checkmark_toggled), view);

	return eq;
}
static void mdnotebook_bufitem_checkmark_proxbufitem_update(_ MdNotebookProxBufItem* self, GtkWidget* render, const GtkTextIter* begin, const GtkTextIter* end) {
	g_return_if_fail(GTK_IS_CHECK_BUTTON(render));
	gchar* str = gtk_text_iter_get_text(begin, end);
	if (g_utf8_strlen(str, -1) >= 2)
		gtk_check_button_set_active(GTK_CHECK_BUTTON(render), str[1] == 'X' || str[1] == 'x');
	g_free(str);
}

static gboolean mdnotebook_bufitem_two_poxbufitem_test_widget(_ MdNotebookProxBufItem* self, GtkWidget* to_test) {
	return GTK_IS_CHECK_BUTTON(to_test);
}

static gint64 mdnotebook_bufitem_two_poxbufitem_get_baseline(_ MdNotebookProxBufItem* self, GtkWidget* widget) {
	g_return_val_if_fail(GTK_IS_CHECK_BUTTON(widget), -1);

	return 2;
}

static void mdnotebook_bufitem_checkmark_class_init(MdNotebookBufItemCheckmarkClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	MdNotebookBufItemClass* bufitem_class = MDNOTEBOOK_BUFITEM_CLASS(class);
	MdNotebookProxBufItemClass* proxbufitem_class = MDNOTEBOOK_PROXBUFITEM_CLASS(class);

	object_class->dispose = mdnotebook_bufitem_checkmark_object_dispose;

	bufitem_class->buffer_changed = mdnotebook_bufitem_checkmark_bufitem_buffer_changed;

	proxbufitem_class->tag = mdnotebook_bufitem_checkmark_proxbufitem_tag;
	proxbufitem_class->render = mdnotebook_bufitem_checkmark_proxbufitem_render;
	proxbufitem_class->update = mdnotebook_bufitem_checkmark_proxbufitem_update;
	proxbufitem_class->test_widget = mdnotebook_bufitem_two_poxbufitem_test_widget;
	proxbufitem_class->get_baseline = mdnotebook_bufitem_two_poxbufitem_get_baseline;
}

static void mdnotebook_bufitem_checkmark_init(MdNotebookBufItemCheckmark* self) {
	MdNotebookBufItemCheckmarkPrivate* priv = mdnotebook_bufitem_checkmark_get_instance_private(self);
	priv->checkmark = NULL;
	priv->counter = 0;
}

MdNotebookBufItem* mdnotebook_bufitem_checkmark_new(MdNotebookView* textview) {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_CHECKMARK, "textview", textview, NULL);
}
