/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * This file is part of libosso-abook
 *
 * Copyright (C) 2008-2009 Nokia Corporation. All rights reserved.
 *
 * Contact: Joergen Scheibengruber <jorgen.scheibengruber@nokia.com>
 */

#ifndef _OSSO_ABOOK_LIVE_SEARCH
#define _OSSO_ABOOK_LIVE_SEARCH

#include "osso-abook-filter-model.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define OSSO_ABOOK_TYPE_LIVE_SEARCH osso_abook_live_search_get_type()

#define OSSO_ABOOK_LIVE_SEARCH(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 OSSO_ABOOK_TYPE_LIVE_SEARCH, \
                 OssoABookLiveSearch))
#define OSSO_ABOOK_LIVE_SEARCH_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 OSSO_ABOOK_TYPE_LIVE_SEARCH, \
                 OssoABookLiveSearchClass))
#define OSSO_ABOOK_IS_LIVE_SEARCH(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 OSSO_ABOOK_TYPE_LIVE_SEARCH))
#define OSSO_ABOOK_IS_LIVE_SEARCH_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 OSSO_ABOOK_TYPE_LIVE_SEARCH))
#define OSSO_ABOOK_LIVE_SEARCH_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 OSSO_ABOOK_TYPE_LIVE_SEARCH, \
                 OssoABookLiveSearchClass))

/**
 * OssoABookLiveSearch:
 *
 * All the fields of this structure are private to the object's
 * implementation and should never be accessed directly.
 */
struct _OssoABookLiveSearch {
        /*< private >*/
        GtkToolbar parent;
};

struct _OssoABookLiveSearchClass {
        GtkToolbarClass parent_class;
};

GType
osso_abook_live_search_get_type      (void);

GtkWidget *
osso_abook_live_search_new           (void);

void
osso_abook_live_search_append_text   (OssoABookLiveSearch *livesearch,
                                      const char           *utf8);

const char *
osso_abook_live_search_get_text      (OssoABookLiveSearch *livesearch);

void
osso_abook_live_search_set_filter    (OssoABookLiveSearch *livesearch,
                                      OssoABookFilterModel *filter);

void
osso_abook_live_search_widget_hook   (OssoABookLiveSearch *livesearch,
                                      GtkWidget            *widget,
                                      GtkTreeView          *kb_focus);

void
osso_abook_live_search_widget_unhook (OssoABookLiveSearch *livesearch);

void
osso_abook_live_search_save_state    (OssoABookLiveSearch *livesearch,
                                      GKeyFile             *key_file);

void
osso_abook_live_search_restore_state (OssoABookLiveSearch *livesearch,
                                      GKeyFile             *key_file);

G_END_DECLS

#endif /* _OSSO_ABOOK_LIVE_SEARCH */
