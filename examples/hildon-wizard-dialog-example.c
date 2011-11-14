/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2005, 2006, 2007, 2009 Nokia Corporation, all rights reserved.
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
#include                                        <string.h>
#include                                        <glib.h>
#include                                        <gtk/gtk.h>
#include                                        <hildon/hildon.h>

gboolean
on_page_switch (GtkNotebook *notebook, 
                GtkNotebookPage *page, 
                guint num,
                HildonWizardDialog *dialog);

gboolean
on_page_switch (GtkNotebook *notebook, 
                GtkNotebookPage *page, 
                guint num,
                HildonWizardDialog *dialog)
{
    g_debug ("Page %d", num);

    if (num == 1) {
    /*     g_debug ("Making next insensitive! %d", num); */
    /*     gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), HILDON_WIZARD_DIALOG_NEXT, FALSE); */
    }

    return TRUE;
}

static gboolean
some_page_func (GtkNotebook *nb,
                gint current,
                gpointer userdata)
{
  GtkEntry *entry;

  /* Validate data only for the third page. */
  switch (current) {
  case 2:
    entry = GTK_ENTRY (gtk_notebook_get_nth_page (nb, current));
    return (strlen (gtk_entry_get_text (entry)) != 0);
  default:
    return TRUE;
  }
}

int
main (int argc, char **argv)
{
    hildon_gtk_init (&argc, &argv);

    GtkWidget *notebook = gtk_notebook_new ();
    GtkWidget *label_1 = gtk_label_new ("Page 1");
    GtkWidget *label_2 = gtk_label_new ("Page 2");
    GtkWidget *entry_3 = hildon_entry_new (HILDON_SIZE_AUTO);
#ifdef MAEMO_GTK
    hildon_gtk_entry_set_placeholder_text (GTK_ENTRY (entry_3), " Write something to continue");
#endif
    GtkWidget *label_4 = gtk_label_new ("Page 4");

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), label_1, NULL);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), label_2, NULL);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), entry_3, NULL);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), label_4, NULL);

    GtkWidget *dialog = hildon_wizard_dialog_new (NULL, "Wizard", GTK_NOTEBOOK (notebook));
    g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (on_page_switch), dialog);
    hildon_wizard_dialog_set_forward_page_func (HILDON_WIZARD_DIALOG (dialog),
                                                some_page_func, NULL, NULL);

    gtk_widget_show_all (dialog);
    gtk_dialog_run (GTK_DIALOG (dialog));
    
    return 0;
}


