#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "booktool/mdnotebookbooktooltext.h"

#define _ __attribute__((unused))

G_DEFINE_TYPE(MdNotebookBookToolText, mdnotebook_booktool_text, MDNOTEBOOK_TYPE_BOOKTOOL)

static const gchar* mdnotebook_booktool_text_booktool_icon_name(MdNotebookBookTool*) {
	return "format-justify-left-symbolic";
}

static void mdnotebook_booktool_text_booktool_activated(MdNotebookBookTool*, MdNotebookView* view) {
	mdnotebook_view_set_stylus_gesture_state(view, FALSE);
	mdnotebook_view_set_cursor_from_name(view, "text");
}
static void mdnotebook_booktool_text_booktool_deactivated(MdNotebookBookTool*, MdNotebookView* view) {
	mdnotebook_view_set_stylus_gesture_state(view, TRUE);
	mdnotebook_view_set_cursor(view, NULL);
}

static void mdnotebook_booktool_text_class_init(MdNotebookBookToolTextClass* class) {
	MdNotebookBookToolClass* booktool_class = MDNOTEBOOK_BOOKTOOL_CLASS(class);

	booktool_class->icon_name = mdnotebook_booktool_text_booktool_icon_name;
	booktool_class->activated = mdnotebook_booktool_text_booktool_activated;
	booktool_class->deactivated = mdnotebook_booktool_text_booktool_deactivated;
}

static void mdnotebook_booktool_text_init(_ MdNotebookBookToolText* self) {}

MdNotebookBookTool* mdnotebook_booktool_text_new(MdNotebookView* view) {
	return g_object_new(MDNOTEBOOK_TYPE_BOOKTOOL_TEXT, "textview", view, NULL);
}
