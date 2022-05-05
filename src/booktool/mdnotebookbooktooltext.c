#include "booktool/mdnotebookbooktooltext.h"

#define _ __attribute__((unused))

G_DEFINE_TYPE(MdNotebookBookToolText, mdnotebook_booktool_text, MDNOTEBOOK_TYPE_BOOKTOOL)

static const gchar* mdnotebook_booktool_text_booktool_icon_name(MdNotebookBookTool*) {
	return "format-justify-left";
}

static void mdnotebook_booktool_text_class_init(MdNotebookBookToolTextClass* class) {
	MdNotebookBookToolClass* booktool_class = MDNOTEBOOK_BOOKTOOL_CLASS(class);

	booktool_class->icon_name = mdnotebook_booktool_text_booktool_icon_name;
}

static void mdnotebook_booktool_text_init(_ MdNotebookBookToolText* self) {}

MdNotebookBookTool* mdnotebook_booktool_text_new(MdNotebookView* view) {
	return g_object_new(MDNOTEBOOK_TYPE_BOOKTOOL_TEXT, "textview", view, NULL);
}
