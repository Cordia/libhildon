/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2008, 2009 Nokia Corporation, all rights reserved.
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

GtkTextView *textview;
GtkTextBuffer *buffer;

static void
set_text_button_clicked                         (GtkButton *button,
                                                 GtkEntry  *entry)
{
    gtk_text_buffer_set_text (buffer, gtk_entry_get_text (entry), -1);
}

static void
set_placeholder_button_clicked                  (GtkButton *button,
                                                 GtkEntry  *entry)
{
    //hildon_gtk_text_view_set_placeholder_text (textview, gtk_entry_get_text (entry));
}

static void
text_changed                                    (GtkTextBuffer *buffer,
                                                 GtkLabel      *label)
{
    const gchar *text;
    GtkTextIter start, end;

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);

    text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    if (text != NULL && *text != '\0') {
        gtk_label_set_text (label, text);
    } else {
        gtk_label_set_text (label, "(empty)");
    }
}

int
main                                            (int    argc,
                                                 char **argv)
{
    GtkWidget *win;
    GtkWidget *label;
    GtkWidget *textentry, *textbutton, *texthbox;
    GtkWidget *placeholderentry, *placeholderbutton, *placeholderhbox;
    GtkWidget *textviewframe;
    GtkBox *vbox;

    hildon_gtk_init (&argc, &argv);

    /* Window and vbox to pack everything */
    win = hildon_stackable_window_new ();
    vbox = GTK_BOX (gtk_vbox_new (FALSE, 10));

    /* Entry to modify the text of the main HildonEntry */
    textentry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    textbutton = hildon_gtk_button_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    gtk_button_set_label (GTK_BUTTON (textbutton), "Set text view text");
    texthbox = gtk_hbox_new (FALSE, 10);

    /* Entry to modify the placeholder of the main HildonEntry */
    placeholderentry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    placeholderbutton = hildon_gtk_button_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
    gtk_button_set_label (GTK_BUTTON (placeholderbutton), "Set text view placeholder");
    placeholderhbox = gtk_hbox_new (FALSE, 10);

    /* The text view */
    textview = GTK_TEXT_VIEW (hildon_text_view_new ());
    buffer = gtk_text_view_get_buffer (textview);
    //hildon_gtk_text_view_set_placeholder_text (textview, "This is a placeholder - change using the buttons above");
    textviewframe = gtk_frame_new (NULL);

    /* This label is used to show the contents -not the placeholder- of the HildonTextView */
    label = gtk_label_new (NULL);

    /* Pack all widgets */
    gtk_box_pack_start (GTK_BOX (texthbox), textentry, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (texthbox), textbutton, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (placeholderhbox), placeholderentry, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (placeholderhbox), placeholderbutton, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (textviewframe), GTK_WIDGET (textview));

    gtk_box_pack_start (vbox, texthbox, FALSE, FALSE, 0);
    gtk_box_pack_start (vbox, placeholderhbox, FALSE, FALSE, 0);
    gtk_box_pack_start (vbox, textviewframe, TRUE, TRUE, 0);
    gtk_box_pack_start (vbox, gtk_label_new ("Contents of the text view:"), TRUE, TRUE, 0);
    gtk_box_pack_start (vbox, label, TRUE, TRUE, 0);

    gtk_container_set_border_width (GTK_CONTAINER (win), 20);
    gtk_container_add (GTK_CONTAINER (win), GTK_WIDGET (vbox));

    /* Connect signals */
    g_signal_connect (win, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (buffer, "changed", G_CALLBACK (text_changed), label);
    g_signal_connect (textbutton, "clicked",
                      G_CALLBACK (set_text_button_clicked), textentry);
    g_signal_connect (placeholderbutton, "clicked",
                      G_CALLBACK (set_placeholder_button_clicked), placeholderentry);

    /* Set the initial state of the label */
    text_changed (buffer, GTK_LABEL (label));

    /* Run example */
    gtk_widget_show_all (win);
    gtk_main ();

    return 0;
}
