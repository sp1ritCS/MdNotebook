#define MDNOTEBOOK_VIEW_EXPOSE_INTERNAS
#include "booktool/mdnotebookbooktoolpen.h"

#include "bufitem/mdnotebookproxbufitem.h"

#define _ __attribute__((unused))

G_DEFINE_TYPE(MdNotebookBookToolPen, mdnotebook_booktool_pen, MDNOTEBOOK_TYPE_BOOKTOOL)

static const gchar* mdnotebook_booktool_pen_booktool_icon_name(MdNotebookBookTool*) {
	return "document-edit";
}

static void mdnotebook_booktool_pen_commit_stroke(MdNotebookBookTool* tool) {
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	GtkTextView* gview = GTK_TEXT_VIEW(view);
	GtkTextBuffer* buf = gtk_text_view_get_buffer(gview);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);

	GtkTextIter i,k;
	GdkRectangle r;

	/* clip the stroke to the document area */
	gtk_text_view_get_iter_at_location(gview, &i, 0, 0);
	gtk_text_view_get_iter_location(gview, &i, &r);
	mdnotebook_stroke_force_min_xy(stroke_proxy->active, r.x, r.y);

	/* get current bounding box */
	gdouble x0,x1,y0,y1;
	if (!mdnotebook_stroke_get_bbox(stroke_proxy->active, &x0, &x1, &y0, &y1))
		return;

	MdNotebookBoundDrawing* d = NULL;

	/* search for preceding regions we can merge with */
	gtk_text_view_get_iter_at_location(gview, &i, x0, y0);
	k = i; // for later
	do {
		GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&i);
		if (anch) {
			guint len;
			GtkWidget** w = gtk_text_child_anchor_get_widgets(anch, &len);
			if (len) {
				//printf("match?\n");
				d = mdnotebook_bounddrawing_try_upcast(w[0]);
				break;
			}
		}
		GtkTextIter start;
		gtk_text_buffer_get_start_iter(buf, &start);
		if (gtk_text_iter_compare(&i, &start) == 0) break; // nothing more to the left
		GtkTextIter j = i;
		gtk_text_iter_backward_char(&j);
		gunichar prev_char = gtk_text_iter_get_char(&j);
		if (!g_unichar_isspace(prev_char) && prev_char != 0xFFFC) {
			break;
		}
		i = j;
	} while (true);

	if (d != NULL) {
		/* found a region to merge with above; we can adjust for its real location,
		 * but that's relative to the text body, so need to fixup */
		int bx,by;
		gtk_text_view_window_to_buffer_coords(gview, GTK_TEXT_WINDOW_TEXT, 0, 0, &bx, &by);
		GtkAllocation alloc;
		gtk_widget_get_allocation(GTK_WIDGET(d), &alloc);

		if (mdnotebook_bounddrawing_add_stroke(d, stroke_proxy->active, -alloc.x - bx, -alloc.y - by, FALSE))
			goto commit_stroke_done;
	}

	/* can't merge up, but maybe there's a region further down that can be expanded? */
	GtkTextIter end;
	gtk_text_buffer_get_start_iter(buf, &end);
	do {
		GtkTextChildAnchor* anch = gtk_text_iter_get_child_anchor(&i);
		if (anch) {
			guint len;
			GtkWidget** w = gtk_text_child_anchor_get_widgets(anch, &len);
			if (len) {
				//printf("match?\n");
				d = mdnotebook_bounddrawing_try_upcast(w[0]);
				break;
			}
		}
		gunichar next_char = gtk_text_iter_get_char(&k);
		if (!g_unichar_isspace(next_char) && next_char != 0xFFFC) {
			break;
		}
		gtk_text_iter_forward_char(&k);
	} while (gtk_text_iter_compare(&k, &end) != 0);

	if (d) {
		/* found a region to merge with below; we need to expand it before adding this stroke */

		/* find where the drawing should start */
		gtk_text_view_get_iter_at_location(gview, &i, x0, y0);
		gtk_text_view_get_iter_location(gview, &i, &r);

		//printf("get rect: %d %d %d %d\n",r.x,r.y,r.width,r.height);

		/* once again, need to correct allocation to be in buffer coords */
		int bx, by;
		gtk_text_view_window_to_buffer_coords(gview, GTK_TEXT_WINDOW_TEXT, 0, 0, &bx, &by);

		GtkAllocation alloc;
		gtk_widget_get_allocation(GTK_WIDGET(d), &alloc);
		int dx = -alloc.x-bx + r.x,
		    dy = -alloc.y-by + r.y;

		gint width,height;
		mdnotebook_bounddrawing_get_size(d, &width, &height);
		if (!mdnotebook_bounddrawing_update_size(d, width, height, dx, dy)) goto commit_stroke_new_drawing;
		if (!mdnotebook_bounddrawing_add_stroke(d, stroke_proxy->active, -r.x, -r.y, FALSE)) goto commit_stroke_new_drawing;

		/* eat intermittent spaces */
		gtk_text_buffer_delete(buf, &i, &k);
	} else goto commit_stroke_new_drawing;

commit_stroke_new_drawing:
	{
		d = MDNOTEBOOK_BOUNDDRAWING(mdnotebook_bounddrawing_new());
		gtk_text_view_get_iter_at_location(gview, &i, x0, y0);

		GtkTextTag* tag_hidden = mdnotebook_proxbufitem_get_invisible_tag(MDNOTEBOOK_BUFFER(buf)); // TODO: maybe save to a avoid strcmp?
		if (gtk_text_iter_has_tag(&i, tag_hidden)) {
			gtk_text_iter_forward_to_tag_toggle(&i, tag_hidden);
			gtk_text_iter_forward_char(&i); // TODO: Is this safe?
		}

		/* figure where the bottom right goes, so we can chomp spaces that we painted over */
		gtk_text_view_get_iter_at_location(gview, &k, x1, y1);
		/* TODO */

		/* figure out where it got anchored in the text, so we can translate the stroke correctly */
		gtk_text_view_get_iter_location(gview, &i, &r);
		if (r.x > x0) {
			/* end of document, line too far to the right; add a new line */
			/* TODO: check this logic */
			gtk_text_buffer_insert(buf, &i, "\n", -1);
			gtk_text_view_get_iter_at_location(gview, &i, x0, y0);
			gtk_text_view_get_iter_location(gview, &i, &r);
		}

		GtkTextChildAnchor* anch = gtk_text_buffer_create_child_anchor(buf, &i);
		gtk_text_view_add_child_at_anchor(gview, GTK_WIDGET(d), anch);

		//printf("anchoring: %f %f -> %d %d\n",x0,y0,r.x,r.y);

		mdnotebook_bounddrawing_add_stroke(d, stroke_proxy->active, -r.x, -r.y, TRUE);
	}

commit_stroke_done:
	gtk_widget_show(GTK_WIDGET(d));
	// floats.insert
	stroke_proxy->active = mdnotebook_stroke_new(stroke_proxy->active->color);
	// on_changed();
}

static gboolean mdnotebook_booktool_pen_booktool_gesture_start(MdNotebookBookTool* tool, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);
	stroke_proxy->active->num_nodes = 0;
	// potentional reset the allocated stroke area back to 0x80?
	mdnotebook_stroke_append_node(stroke_proxy->active, x, y, pressure);
	gtk_widget_queue_draw(stroke_proxy->overlay);

	return TRUE;
}
static gboolean mdnotebook_booktool_pen_booktool_gesture_move(MdNotebookBookTool* tool, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);
	mdnotebook_stroke_append_node(stroke_proxy->active, x, y, pressure);
	gtk_widget_queue_draw(stroke_proxy->overlay);

	return TRUE;
}
static gboolean mdnotebook_booktool_pen_booktool_gesture_end(MdNotebookBookTool* tool, gdouble x, gdouble y, gdouble pressure) {
	MdNotebookView* view = mdnotebook_booktool_get_textview(tool);
	MdNotebookViewStrokeProxy* stroke_proxy = mdnotebook_view_get_stroke_proxy(view);
	mdnotebook_stroke_append_node(stroke_proxy->active, x, y, pressure);
	gtk_widget_queue_draw(stroke_proxy->overlay);

	mdnotebook_booktool_pen_commit_stroke(tool);

	return TRUE;
}

static void mdnotebook_booktool_pen_class_init(MdNotebookBookToolPenClass* class) {
	MdNotebookBookToolClass* booktool_class = MDNOTEBOOK_BOOKTOOL_CLASS(class);
	booktool_class->icon_name = mdnotebook_booktool_pen_booktool_icon_name;
	booktool_class->gesture_start = mdnotebook_booktool_pen_booktool_gesture_start;
	booktool_class->gesture_end = mdnotebook_booktool_pen_booktool_gesture_end;
	booktool_class->gesture_move = mdnotebook_booktool_pen_booktool_gesture_move;
}

static void mdnotebook_booktool_pen_init(_ MdNotebookBookToolPen* self) {}

MdNotebookBookTool* mdnotebook_booktool_pen_new(MdNotebookView* view) {
	return g_object_new(MDNOTEBOOK_TYPE_BOOKTOOL_PEN, "textview", view, NULL);
}
