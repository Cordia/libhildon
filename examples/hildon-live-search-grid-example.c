/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2010 Nokia Corporation, all rights reserved.
 *
 * Author: Claudio Saavedra <csaavedra@igalia.com>
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

static GtkTreeModel *
create_model (void)
{
    GtkListStore *model;
    GSList *icon_list = NULL;
    GSList *item = NULL;

    model = gtk_list_store_new (2, G_TYPE_STRING, GDK_TYPE_PIXBUF);
    icon_list = gtk_stock_list_ids ();

    for (item = icon_list; item; item = g_slist_next (item)) {
        GtkTreeIter iter;
        GtkStockItem stock_item;
        gchar *stock_id;
        GdkPixbuf *pixbuf;

        stock_id = (gchar *)item->data;
        gtk_stock_lookup (stock_id, &stock_item);
        pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                           stock_id, hildon_get_icon_pixel_size (GTK_ICON_SIZE_MENU),
                                           0, NULL);
        if (pixbuf) {
            gtk_list_store_append (model, &iter);
            gtk_list_store_set (model, &iter, 0,
                                stock_item.label, 1, pixbuf, -1);

            g_object_unref (pixbuf);
        }
        g_free (stock_id);
    }

    return GTK_TREE_MODEL (model);
}

int
main                                            (int    argc,
                                                 char **argv)
{
    GtkWidget *window;
    GtkWidget *panarea, *iconview, *vbox;
    GtkWidget *livesearch;
    GtkTreeModel *filter;

    hildon_gtk_init (&argc, &argv);

    window = hildon_window_new ();
    vbox = gtk_vbox_new (FALSE, 0);
    panarea = hildon_pannable_area_new ();

    filter = gtk_tree_model_filter_new (create_model (), NULL);

    iconview = hildon_gtk_icon_view_new_with_model (HILDON_UI_MODE_NORMAL, filter);
    gtk_icon_view_set_text_column (GTK_ICON_VIEW (iconview), 0);
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (iconview), 1);
    livesearch = hildon_live_search_new ();
    hildon_live_search_set_filter (HILDON_LIVE_SEARCH (livesearch),
                                   GTK_TREE_MODEL_FILTER (filter));
    hildon_live_search_widget_hook (HILDON_LIVE_SEARCH (livesearch),
                                    iconview, iconview);
    hildon_live_search_set_text_column (HILDON_LIVE_SEARCH (livesearch), 0);
    g_object_unref (filter);

    gtk_container_add (GTK_CONTAINER (panarea), iconview);
    gtk_box_pack_start (GTK_BOX (vbox), panarea, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), livesearch, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    gtk_widget_show_all (window);

    gtk_main ();

    return 0;
}
