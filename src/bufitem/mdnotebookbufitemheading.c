#include "bufitem/mdnotebookbufitemheading.h"

#define _ __attribute__((unused))

static void mdnotebook_bufitem_heading_bufitem_iface_init(MdNotebookBufItemInterface* iface);

G_DEFINE_TYPE_WITH_CODE(MdNotebookBufItemHeading, mdnotebook_bufitem_heading, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(MDNOTEBOOK_TYPE_BUFITEM, mdnotebook_bufitem_heading_bufitem_iface_init))

gchar valid_titletags[6][9] = {"mdtitle1", "mdtitle2", "mdtitle3", "mdtitle4", "mdtitle5", "mdtitle6"};
static void strip_titletags(GtkTextBuffer* buf, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);

	for (size_t i = 0; i<6; i++) {
		GtkTextTag* titletag = gtk_text_tag_table_lookup(tagtable, valid_titletags[i]);
		if (titletag)
			gtk_text_buffer_remove_tag(buf, titletag, start, end);
	}

	GtkTextTag* titlecolortag = gtk_text_tag_table_lookup(tagtable, "mdtitlecolor");
	if (titlecolortag)
		gtk_text_buffer_remove_tag(buf, titlecolortag, start, end);
}

static void mdnotebook_bufitem_heading_bufitem_buffer_changed(_ MdNotebookBufItem* iface, MdNotebookBuffer* self, const GtkTextIter* start, const GtkTextIter* end) {
	GtkTextBuffer* buf = GTK_TEXT_BUFFER(self);
	GtkTextTagTable* tagtable = gtk_text_buffer_get_tag_table(buf);
	GtkTextIter active = *start;

	GtkTextTag* privatetag = mdnotebook_bufitem_get_private_tag(self);

	strip_titletags(buf, start, end);

	while (true) {
		if (mdnotebook_bufitem_is_iter_in_private(self, &active))
			goto skip_check;
		gunichar c = gtk_text_iter_get_char(&active);
		if (c == '#') {
			GtkTextIter checker = active;
			guint8 level = 1;
			while (gtk_text_iter_forward_char(&checker)) {
				if (gtk_text_iter_get_char(&checker) == '#') {
					level++;
				} else {
					if (gtk_text_iter_get_char(&checker) == ' ')
						break;
					else
						level = 0;
				}
			}

			if (level) {
				gchar label[9] = "mdtitleX";
				g_ascii_dtostr(&label[7], 2, level);
				GtkTextTag* leveltag = gtk_text_tag_table_lookup(tagtable, label);
				if (!leveltag) {
					if (level <= 3)
						leveltag = gtk_text_buffer_create_tag(buf, label,
							"weight", PANGO_WEIGHT_BOLD,
							"size", (36 * PANGO_SCALE) / level,
						NULL);
					else
						leveltag = gtk_text_buffer_create_tag(buf, label,
							"weight", PANGO_WEIGHT_BOLD,
						NULL);
				}

				GtkTextTag* titlecolortag = gtk_text_tag_table_lookup(tagtable, "mdtitlecolor");
				if (!titlecolortag)
					titlecolortag = gtk_text_buffer_create_tag(buf, "mdtitlecolor",
							"foreground", "#A0A8C0",
						NULL);
				if (gtk_text_iter_starts_tag(&active, titlecolortag)) {
					if (gtk_text_iter_ends_tag(&checker, titlecolortag)) {
						goto skip_titlecolortag;
					} else {
						GtkTextIter endl = checker;
						gtk_text_iter_forward_to_line_end(&endl);
						gtk_text_buffer_remove_tag(buf, titlecolortag, &active, &checker);
					}
				}
				gtk_text_buffer_apply_tag(buf, titlecolortag, &active, &checker);
skip_titlecolortag:

				gtk_text_iter_forward_to_line_end(&checker);
				if (gtk_text_iter_starts_tag(&active, leveltag)) {
					if (gtk_text_iter_ends_tag(&checker, leveltag)) {
						goto skip_leveltag;
					} else {
						gtk_text_buffer_remove_tag(buf, leveltag, &active, &checker);
					}
				}
				gtk_text_buffer_apply_tag(buf, leveltag, &active, &checker);
skip_leveltag:
				// TODO: allow this to be toggled with a property, if tiles should
				// be allowed to contain other widgets
				gtk_text_buffer_apply_tag(buf, privatetag, &active, &checker);
			}
		}

skip_check:
		if (!gtk_text_iter_forward_line(&active))
			break;
	}
}

static void mdnotebook_bufitem_heading_class_init(_ MdNotebookBufItemHeadingClass* class) {}
static void mdnotebook_bufitem_heading_bufitem_iface_init(MdNotebookBufItemInterface* iface) {
	iface->buffer_changed = mdnotebook_bufitem_heading_bufitem_buffer_changed;
}

static void mdnotebook_bufitem_heading_init(_ MdNotebookBufItemHeading* self) {}

MdNotebookBufItem* mdnotebook_bufitem_heading_new() {
	return g_object_new(MDNOTEBOOK_TYPE_BUFITEM_HEADING, NULL);
}
