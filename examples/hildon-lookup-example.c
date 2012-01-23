/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Author: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
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

#include                                        <stdio.h>
#include                                        <stdlib.h>
#include                                        <glib.h>
#include                                        <gtk/gtk.h>
#include                                        <hildon/hildon.h>

static GtkWidget*
create_button_with_icon                         (const gchar *icon);

static GtkWidget*
create_button_with_icon                         (const gchar *icon)
{
    GtkImage *image = GTK_IMAGE (gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_BUTTON));
    if (image == NULL) {
        g_warning ("Failed to create a GtkImage from icon name: %s", icon);
        return NULL;
    }

    gtk_misc_set_padding (GTK_MISC (image), 12, 12);

    GtkButton *button = GTK_BUTTON (gtk_button_new ());

    gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (image));

    return (GtkWidget *) button;
}

int
main                                            (int argc,
                                                 char **argv)
{
    hildon_gtk_init (&argc, &argv);

    GtkDialog *dialog = GTK_DIALOG (gtk_dialog_new ());
    gtk_window_set_title (GTK_WINDOW (dialog), "icons");

    GtkTable *table = GTK_TABLE (gtk_table_new (3, 3, TRUE));
    gtk_table_attach (table, create_button_with_icon (GTK_STOCK_OK), 0, 1, 0, 1, 0, 0, 0, 0);
    gtk_table_attach (table, create_button_with_icon (GTK_STOCK_CANCEL), 1, 2, 0, 1, 0, 0, 0, 0);
    gtk_table_attach (table, create_button_with_icon (GTK_STOCK_QUIT), 2, 3, 0, 1, 0, 0, 0, 0);
    
    gtk_table_attach (table, create_button_with_icon (GTK_STOCK_SAVE), 0, 1, 1, 2, 0, 0, 0, 0);
    gtk_table_attach (table, create_button_with_icon (GTK_STOCK_MEDIA_PAUSE), 1, 2, 1, 2, 0, 0, 0, 0);
    gtk_table_attach (table, create_button_with_icon (GTK_STOCK_FILE), 2, 3, 1, 2, 0, 0, 0, 0);
    
    gtk_table_set_col_spacings (table, 6);
    gtk_table_set_row_spacings (table, 6);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (dialog)), GTK_WIDGET (table), FALSE, FALSE, 0);

    gtk_dialog_add_button (dialog, "Close", GTK_RESPONSE_CLOSE);

    gtk_widget_show_all (GTK_WIDGET (dialog));
    gtk_dialog_run (dialog);
    
    return 0;
}


