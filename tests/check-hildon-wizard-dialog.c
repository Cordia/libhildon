/*
 * This file is a part of hildon tests
 *
 * Copyright (C) 2006, 2007 Nokia Corporation, all rights reserved.
 *
 * Contact: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
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

#include <stdlib.h>
#include <check.h>
#include <glib/gprintf.h>
#include "test_suites.h"
#include "check_utils.h"

#include <hildon/hildon-wizard-dialog.h>

#define DEFAULT_NOTEBOOK_PAGES 3

/* -------------------- Fixtures -------------------- */

static HildonWizardDialog *wizard_dialog = NULL;
static GtkWindow * showed_window = NULL;

static void 
fx_setup_default_wizard_dialog ()
{
  int argc = 0;
  gint i;
  GtkNotebook *book;
  GtkBox *box[DEFAULT_NOTEBOOK_PAGES];
  GtkLabel *label;

  gtk_init(&argc, NULL);

  showed_window = GTK_WINDOW(create_test_window());
    
  /* Create a default notebook for the wizard */
  book = GTK_NOTEBOOK(gtk_notebook_new());

  /* Check notebook object has been created properly */
  fail_if(!GTK_IS_NOTEBOOK(book),
          "hildon-wizard-dialog: Auxiliar notebook creation failed.");  

  for(i=0; i<DEFAULT_NOTEBOOK_PAGES; i++)
    {
      box[i] = GTK_BOX(gtk_vbox_new(FALSE, 0));
      label = GTK_LABEL(gtk_label_new(g_strdup_printf("Label for page %d", i)));
      gtk_box_pack_start (box[i], GTK_WIDGET(label),
                          TRUE, TRUE, 0);
      gtk_notebook_append_page(book, GTK_WIDGET(box[i]), NULL); 
    } 

  /* Create the wizard dialog */
  wizard_dialog = HILDON_WIZARD_DIALOG(hildon_wizard_dialog_new(showed_window, "Wizard test", book));

  show_test_window(GTK_WIDGET(showed_window));
  
  show_test_window(GTK_WIDGET(wizard_dialog));
  
  /* Check wizard dialog object has been created properly */
  fail_if(!HILDON_IS_WIZARD_DIALOG(wizard_dialog),
          "hildon-wizard-dialog: Creation failed.");  

}

static void 
fx_teardown_default_wizard_dialog ()
{
  gtk_widget_destroy (GTK_WIDGET(wizard_dialog));
}

/* -------------------- Test cases -------------------- */

/* ----- Test case for set/get property notebook -----*/

/**
 * Purpose: Check set and get of property "wizard-notebook"
 * Cases considered:
 *    - Set and get of a regular notebook.
 */
START_TEST (test_set_get_property_wizard_notebook_regular)
{
  GtkNotebook *book, *old_book;
  GValue book_value = {0,};
  GValue ret_book_value = {0,};
  gint i;
  GtkBox *box[2];
  GtkLabel *label;

  book = GTK_NOTEBOOK(gtk_notebook_new());
  for(i=0; i<2; i++)
    {
      box[i] = GTK_BOX(gtk_vbox_new(FALSE, 0));
      label = GTK_LABEL(gtk_label_new(g_strdup_printf("Label for page %d", i)));
      gtk_box_pack_start (box[i], GTK_WIDGET(label),
                          TRUE, TRUE, 0);
      gtk_notebook_append_page(book, GTK_WIDGET(box[i]), NULL); 
    }

  /* Free the old notebook */
  g_value_init(&ret_book_value, GTK_TYPE_NOTEBOOK);
  g_object_get_property(G_OBJECT(wizard_dialog), "wizard-notebook", &ret_book_value);
  old_book = g_value_get_object (&ret_book_value);
  gtk_widget_destroy (GTK_WIDGET(old_book));

  g_value_init(&book_value, GTK_TYPE_NOTEBOOK);
  g_value_set_object(&book_value, book);
  g_value_reset (&ret_book_value);

  /* Test1: Set and get a regular notebook */
  /* We don't test dialog title, set during setting of property "wizard-notebook"
     because it is a visual issue and not a functional one, so it can be changed 
     at any time without breaking any test */
  g_object_set_property(G_OBJECT(wizard_dialog), "wizard-notebook", &book_value);
  g_object_get_property(G_OBJECT(wizard_dialog), "wizard-notebook", &ret_book_value);
  fail_if(g_value_get_object(&ret_book_value) != g_value_get_object(&book_value),
          "hildon-wizard-dialog: set property \"wizard-notebook\" but get_property returned a different notebook object");
}
END_TEST

/**
 * Purpose: Check set and get of property "wizard-notebook" with
 *          invalid values.
 * Cases considered:
 *    - Set property to NULL
 */
START_TEST (test_set_get_property_wizard_notebook_invalid)
{
  GValue book_value = {0,};
  GValue ret_book_value = {0,};

  /* Test1: set property to NULL */
  g_value_init(&book_value, GTK_TYPE_NOTEBOOK);
  g_value_set_object(&book_value, NULL);
  g_object_set_property(G_OBJECT(wizard_dialog), "wizard-notebook", &book_value);
  g_object_get_property(G_OBJECT(wizard_dialog), "wizard-notebook", &ret_book_value);
  fail_if((g_value_get_object(&ret_book_value) != NULL),
          "hildon-wizard-dialog: Set \"wizard-notebook\" property to NULL is not forbidden");
}
END_TEST


/* ---------- Suite creation ---------- */

Suite *create_hildon_wizard_dialog_suite()
{
  /* Create the suite */
  Suite *s = suite_create("HildonWizardDialog");

  /* Create test cases */
  TCase *tc1 = tcase_create("set_get_property_wizard_notebook");

  /* Create test case for property "wizard-notebook" and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_wizard_dialog, fx_teardown_default_wizard_dialog);
  tcase_add_test(tc1, test_set_get_property_wizard_notebook_regular);
  tcase_add_test(tc1, test_set_get_property_wizard_notebook_invalid);
  suite_add_tcase(s, tc1);

  /* Return created suite */
  return s;             
}


