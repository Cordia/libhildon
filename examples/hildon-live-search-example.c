/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2009 Nokia Corporation, all rights reserved.
 *
 * Author: Alberto Garcia <agarcia@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include                                        <hildon/hildon.h>

static const char *countries[] = {
    "Austria", "Belgium", "Bulgaria", "Cyprus", "Czech Republic",
    "Denmark", "Estonia", "Finland", "France", "Germany", "Greece",
    "Hungary", "Ireland", "Italy", "Latvia", "Lithuania", "Luxembourg",
    "Malta", "Netherlands", "Poland", "Portugal", "Romania", "Slovakia",
    "Slovenia", "Spain", "Sweden", "United Kingdom"
};

static gboolean
filter_func                                     (GtkTreeModel *model,
                                                 GtkTreeIter  *iter,
                                                 gchar        *text,
                                                 gpointer      data)
{
    gboolean retvalue;
    gchar *rowtext = NULL;

    gtk_tree_model_get (model, iter, 0, &rowtext, -1);
    retvalue = g_str_has_prefix (rowtext, text);
    g_free (rowtext);

    return retvalue;
}

static HildonLiveSearch *
create_live_search                              (GtkTreeModelFilter *filter)
{
    HildonLiveSearch *live;

    live = HILDON_LIVE_SEARCH (hildon_live_search_new ());
    hildon_live_search_set_filter (live, filter);
    hildon_live_search_set_visible_func (live, filter_func, NULL, NULL);

    /* Instead of hildon_live_search_set_filter_func(), we could have used
     * hildon_live_search_set_text_column (live, 0); */

    return live;
}

static GtkTreeModel *
create_model                                    (void)
{
    int i;
    GtkListStore *store;
    GtkTreeModel *filter;

    store = gtk_list_store_new (1, G_TYPE_STRING);

    for (i = 0; i < G_N_ELEMENTS (countries); i++) {
        gtk_list_store_insert_with_values (store, NULL, i, 0, countries[i], -1);
    }

    filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (store), NULL);
    g_object_unref (store);

    return filter;
}

static GtkWidget *
create_tree_view                                (void)
{
    GtkWidget *tree_view;
    GtkCellRenderer *renderer;
    GtkTreeModel *model;

    tree_view = hildon_gtk_tree_view_new (HILDON_UI_MODE_NORMAL);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);

    model = create_model ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), model);
    g_object_unref (model);

    renderer = gtk_cell_renderer_text_new ();
    g_object_set (renderer, "xalign", 0.5, "weight", PANGO_WEIGHT_BOLD, NULL);

    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view), 0,
                                                 "Column 0", renderer, "text", 0, NULL);

    return tree_view;
}


int
main                                            (int    argc,
                                                 char **argv)
{
    GtkWidget *window;
    GtkWidget *panarea;
    GtkWidget *vbox;
    GtkWidget *treeview;
    HildonLiveSearch *live;
    GtkTreeModelFilter *filter;

    hildon_gtk_init (&argc, &argv);

    window = hildon_window_new ();
    vbox = gtk_vbox_new (FALSE, 0);
    panarea = hildon_pannable_area_new ();

    treeview = create_tree_view ();
    filter = GTK_TREE_MODEL_FILTER (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

    live = create_live_search (filter);
    hildon_live_search_widget_hook (live, window, GTK_TREE_VIEW (treeview));

    gtk_container_add (GTK_CONTAINER (panarea), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), panarea, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (live), FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    gtk_widget_show_all (window);

    gtk_main ();

    return 0;
}
