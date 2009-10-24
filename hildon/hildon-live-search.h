/*
 * This file is a part of hildon
 *
 * Copyright (C) 2007-2009 Nokia Corporation. All rights reserved.
 *
 * Based in OssoABookLiveSearch, OSSO Address Book.
 * Author: Joergen Scheibengruber <jorgen.scheibengruber@nokia.com>
 * Hildon version: Claudio Saavedra <csaavedra@igalia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 */

#ifndef _HILDON_LIVE_SEARCH
#define _HILDON_LIVE_SEARCH

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HILDON_TYPE_LIVE_SEARCH hildon_live_search_get_type()

#define HILDON_LIVE_SEARCH(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 HILDON_TYPE_LIVE_SEARCH, \
                 HildonLiveSearch))
#define HILDON_LIVE_SEARCH_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 HILDON_TYPE_LIVE_SEARCH, \
                 HildonLiveSearchClass))
#define HILDON_IS_LIVE_SEARCH(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 HILDON_TYPE_LIVE_SEARCH))
#define HILDON_IS_LIVE_SEARCH_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 HILDON_TYPE_LIVE_SEARCH))
#define HILDON_LIVE_SEARCH_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 HILDON_TYPE_LIVE_SEARCH, \
                 HildonLiveSearchClass))


typedef struct _HildonLiveSearch HildonLiveSearch;
typedef struct _HildonLiveSearchClass HildonLiveSearchClass;

/**
 * HildonLiveSearch:
 *
 * All the fields of this structure are private to the object's
 * implementation and should never be accessed directly.
 */
struct _HildonLiveSearch {
        /*< private >*/
        GtkToolbar parent;
};

struct _HildonLiveSearchClass {
        GtkToolbarClass parent_class;
};

GType
hildon_live_search_get_type      (void);

GtkWidget *
hildon_live_search_new           (void);

void
hildon_live_search_append_text   (HildonLiveSearch *livesearch,
                                  const char           *utf8);

const char *
hildon_live_search_get_text      (HildonLiveSearch *livesearch);

void
hildon_live_search_set_filter    (HildonLiveSearch   *livesearch,
                                  GtkTreeModelFilter *filter);

void
hildon_live_search_widget_hook   (HildonLiveSearch *livesearch,
                                  GtkWidget        *widget,
                                  GtkTreeView      *kb_focus);

void
hildon_live_search_widget_unhook (HildonLiveSearch *livesearch);

void
hildon_live_search_save_state    (HildonLiveSearch *livesearch,
                                  GKeyFile         *key_file);

void
hildon_live_search_restore_state (HildonLiveSearch *livesearch,
                                  GKeyFile         *key_file);

G_END_DECLS

#endif /* _HILDON_LIVE_SEARCH */
