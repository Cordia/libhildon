/*
 * This file is a part of hildon examples
 *
 * Copyright (C) 2008 Nokia Corporation, all rights reserved.
 *
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

#include                                        <glib.h>
#include                                        <gtk/gtk.h>
#include                                        <hildon/hildon.h>

static GtkWidget *create_selector ();
static GtkWidget *get_visible_content (GtkWidget * window);

static GtkWindow *parent_window = NULL;

static GtkWidget *label = NULL;

static void
value_changed (HildonPickerButton * button,
               gpointer user_data)
{
  HildonTouchSelector *selector;
  gchar *aux_string = NULL;

  selector = hildon_picker_button_get_selector (button);
  aux_string = hildon_touch_selector_get_current_text (selector);
  gtk_label_set_text (GTK_LABEL (label), aux_string);
  g_free (aux_string);
}

static GtkWidget *
create_selector ()
{
  GtkWidget *selector = NULL;
  GSList *icon_list = NULL;
  GtkListStore *store_icons = NULL;
  GSList *item = NULL;
  GtkCellRenderer *renderer = NULL;
  HildonTouchSelectorColumn *column = NULL;

  selector = hildon_touch_selector_new ();

  icon_list = gtk_stock_list_ids ();

  store_icons = gtk_list_store_new (1, G_TYPE_STRING);
  for (item = icon_list; item; item = g_slist_next (item)) {
    GtkTreeIter iter;
    gchar *label = item->data;

    gtk_list_store_append (store_icons, &iter);
    gtk_list_store_set (store_icons, &iter, 0, label, -1);
    g_free (label);
  }
  g_slist_free (icon_list);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_renderer_set_fixed_size (renderer, -1, 100);

  column = hildon_touch_selector_append_column (HILDON_TOUCH_SELECTOR (selector),
                                                GTK_TREE_MODEL (store_icons),
                                                renderer, "stock-id", 0, NULL);
  hildon_touch_selector_column_set_text_column (column, 0);

  g_object_unref (store_icons);

  hildon_touch_selector_set_column_selection_mode (HILDON_TOUCH_SELECTOR (selector),
                                                   HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE);

  return selector;
}

static GtkWidget *
get_visible_content (GtkWidget * window)
{
  GtkWidget *result = NULL;
  GtkWidget *button = NULL;
  GtkWidget *selector;

  label = gtk_label_new ("Here we are going to put the selection");

  button = hildon_picker_button_new (HILDON_SIZE_AUTO, HILDON_BUTTON_ARRANGEMENT_VERTICAL);
  hildon_button_set_title (HILDON_BUTTON (button), "Click me..");
  selector = create_selector ();
  hildon_picker_button_set_selector (HILDON_PICKER_BUTTON (button),
                                     HILDON_TOUCH_SELECTOR (selector));

  g_signal_connect (G_OBJECT (button), "value-changed",
                    G_CALLBACK (value_changed), window);

  result = gtk_vbox_new (FALSE, 6);

  gtk_container_add (GTK_CONTAINER (result), button);
  gtk_container_add (GTK_CONTAINER (result), label);

  return result;
}

int
main (int argc, char **argv)
{
  HildonProgram *program = NULL;
  GtkWidget *window = NULL;

  hildon_gtk_init (&argc, &argv);

  program = hildon_program_get_instance ();
  g_set_application_name
    ("hildon-touch-selector cell renderer example program");

  window = hildon_stackable_window_new ();
  parent_window = GTK_WINDOW (window);
  hildon_program_add_window (program, HILDON_WINDOW (window));

  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  GtkWidget *vbox = get_visible_content (window);

  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_widget_show_all (GTK_WIDGET (window));

  gtk_main ();

  return 0;
}
