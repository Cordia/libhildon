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

int
main                                            (int argc,
                                                 char **argv)
{
    hildon_gtk_init (&argc, &argv);

    GtkDialog *dialog = GTK_DIALOG (gtk_dialog_new ());

    GtkVBox *vbox = GTK_VBOX (gtk_dialog_get_content_area (dialog));

    GtkSizeGroup *size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    GtkWidget *caption1 = hildon_caption_new (size_group, "Caption std", gtk_entry_new (), 
                                              NULL, 0);
    GtkWidget *caption2 = hildon_caption_new (size_group, "Caption left", gtk_entry_new (), 
                                              gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON), 0);
    GtkWidget *caption3 = hildon_caption_new (size_group, "Caption right", gtk_entry_new (), 
                                              gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON), 0);
    GtkWidget *caption4 = hildon_caption_new (size_group, "Check caption", gtk_check_button_new (), 
                                              gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON), 0);
    GtkWidget *caption5 = hildon_caption_new (size_group, "", gtk_entry_new (), 
                                              NULL, 0);

    hildon_caption_set_icon_position (HILDON_CAPTION (caption2), HILDON_CAPTION_POSITION_LEFT);
    hildon_caption_set_icon_position (HILDON_CAPTION (caption3), HILDON_CAPTION_POSITION_RIGHT);
    hildon_caption_set_label_markup (HILDON_CAPTION (caption5), "<b>Bold caption</b>");
    
    gtk_box_pack_start (GTK_BOX (vbox), caption1, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), caption2, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), caption3, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), caption4, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), caption5, FALSE, FALSE, 0);

    gtk_dialog_add_button (dialog, "Close", GTK_RESPONSE_CLOSE);

    gtk_widget_show_all (GTK_WIDGET (dialog));
    gtk_dialog_run (dialog);
    
    return 0;
}


