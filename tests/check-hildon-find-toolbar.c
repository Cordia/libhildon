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
#include "test_suites.h"
#include "check_utils.h"
#include <string.h>

#include <hildon/hildon-find-toolbar.h>
#include <hildon/hildon-window.h>

/* -------------------- Fixtures -------------------- */

static HildonFindToolbar *find_toolbar = NULL;
static GtkWidget *showed_window = NULL;

static void 
fx_setup_default_find_toolbar ()
{
  int argc = 0;
  gtk_init(&argc, NULL);

  /* Creates a find toolbar */
  find_toolbar = HILDON_FIND_TOOLBAR(hildon_find_toolbar_new(TEST_STRING));
  /* Check find toolbar object has been created properly */
  fail_if(!HILDON_IS_FIND_TOOLBAR(find_toolbar), 
          "hildon-find-toolbar: Creation failed.");

  showed_window =  create_test_window ();
  
  hildon_window_add_toolbar(HILDON_WINDOW(showed_window), GTK_TOOLBAR(find_toolbar));

  /* Displays the widget and the window */
  show_all_test_window (showed_window);

}

static void 
fx_setup_model_find_toolbar ()
{
  int argc = 0;
  GtkListStore *model;
  GtkTreeIter iter;
  GValue value = {0,};

  gtk_init(&argc, NULL);

  /* Creates a find toolbar with a model */
  model = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_list_store_append(model, &iter);
  gtk_list_store_set (model, &iter, 0, "hildon", -1);
  gtk_list_store_append(model, &iter);
  gtk_list_store_set (model, &iter, 0, "nokia", -1);
  find_toolbar = HILDON_FIND_TOOLBAR(hildon_find_toolbar_new_with_model(NULL, model, 0));
  /* Check find toolbar object has been created properly */
  fail_if(!HILDON_IS_FIND_TOOLBAR(find_toolbar), 
          "hildon-find-toolbar: Creation failed.");

  /* Check model is set */
  g_value_init (&value, GTK_TYPE_LIST_STORE);
  g_object_get_property(G_OBJECT(find_toolbar), "list", &value);

  fail_if (model != g_value_get_object(&value),
           "hildon-find-toolbar: Creation with model failed, model retrieved is different from model set in new_with_model");

  showed_window =  create_test_window ();
  
  hildon_window_add_toolbar(HILDON_WINDOW(showed_window), GTK_TOOLBAR(find_toolbar));

  /* Displays the widget and the window */
  show_all_test_window (showed_window);
}

static void 
fx_teardown_find_toolbar()
{

  gtk_widget_destroy(GTK_WIDGET(showed_window)); 
  
}

/* -------------------- Test cases -------------------- */

/* ----- Test case for set/get_property "label"  -----*/

/**
 * Purpose: Check that property "label" is set and get properly.
 * Cases considered:
 *    - Set and get with a regular string
 *    - Set and get with value ""
 */
START_TEST (test_set_get_property_label_regular)
{
  GValue value_set = {0,};
  GValue value_get = {0,};

  g_value_init(&value_set,G_TYPE_STRING);
  g_value_init(&value_get,G_TYPE_STRING);

  /* Test1: set/get of a regular string */
  g_value_set_string(&value_set, TEST_STRING);
  g_object_set_property(G_OBJECT(find_toolbar), "label", &value_set);
  g_object_get_property(G_OBJECT(find_toolbar), "label", &value_get);
  fail_if (strcmp(g_value_get_string(&value_get), g_value_get_string(&value_set)) != 0,
	   "hildon-find-toolbar: Property \"label\" was set to value \"%s\", but get_property retrieved value \"%s\"", 
	   g_value_get_string(&value_set), g_value_get_string(&value_get));

  /* Test2: set/get of label "" */
  g_value_set_string(&value_set, "");
  g_object_set_property(G_OBJECT(find_toolbar), "label", &value_set);
  g_object_get_property(G_OBJECT(find_toolbar), "label", &value_get);
  fail_if (strcmp(g_value_get_string(&value_get), g_value_get_string(&value_set)) != 0,
	   "hildon-find-toolbar: Property \"label\" was set to value \"%s\", but get_property retrieved value \"%s\"", 
	   g_value_get_string(&value_set), g_value_get_string(&value_get));
}
END_TEST

/**
 * Purpose: Check handling of invalid values for property "label"
 * Cases considered:
 *    - Set and get with NULL value
 */
START_TEST (test_set_get_property_label_invalid)
{
  GValue value_set = {0,};
  GValue value_get = {0,};

  g_value_init(&value_set,G_TYPE_STRING);

  /* Test1: set/get of NULL label */
  g_value_set_string(&value_set, NULL);
  g_object_set_property(G_OBJECT(find_toolbar), "label", &value_set);
  g_object_get_property(G_OBJECT(find_toolbar), "label", &value_get);
  fail_if (g_value_get_string(&value_get) != NULL,
	   "hildon-find-toolbar: Property \"label\" was set to value NULL, but get_property retrieved value \"%s\"", 
	   g_value_get_string(&value_get));
}
END_TEST

/* ---------- Suite creation ---------- */

Suite *create_hildon_find_toolbar_suite()
{
  /* Create the suite */
  Suite *s = suite_create("HildonFindToolbar");

  /* Create test cases */
  TCase *tc1 = tcase_create("set_get_property_label");
  TCase *tc2 = tcase_create("model_set_get_property_label");

  /* Create unit tests for set/get of property "label" and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_find_toolbar, fx_teardown_find_toolbar);
  tcase_add_test(tc1, test_set_get_property_label_regular);
  tcase_add_test(tc1, test_set_get_property_label_invalid);
  suite_add_tcase (s, tc1);

  /* Create unit tests for set/get of property "label" and add it to the suite */
  tcase_add_checked_fixture(tc2, fx_setup_model_find_toolbar, fx_teardown_find_toolbar);
  tcase_add_test(tc2, test_set_get_property_label_regular);
  tcase_add_test(tc2, test_set_get_property_label_invalid);
  suite_add_tcase (s, tc2);

  /* Return created suite */
  return s;
}
