#ifndef __GTKMDNOTEBOOK_H__
#define __GTKMDNOTEBOOK_H__

#include <mdnotebookconfig.h>

#ifdef MDNOTEBOOK_HAVE_LATEX
#include <bufitem/latex/mdnotebookbufitemlatex.h>
#include <bufitem/latex/mdnotebooklatexequation.h>
#endif

#include <bufitem/mdnotebookbufitem.h>
#include <bufitem/mdnotebookbufitemdynblock.h>
#include <bufitem/mdnotebookbufitemheading.h>
#include <bufitem/mdnotebookbufitemtext.h>

#include <mdnotebookbuffer.h>
#include <mdnotebookbufferextra.h>
#include <mdnotebookbufwidget.h>
#include <mdnotebookview.h>
#include <mdnotebookzoomview.h>

#endif // __GTKMDNOTEBOOK_H__
