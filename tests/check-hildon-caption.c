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
#include <string.h>
#include <check.h>
#include "test_suites.h"
#include "check_utils.h"
#include <hildon/hildon-caption.h>

#include <hildon/hildon-window.h>

#include <libintl.h>

#define _(String) dgettext("hildon-libs", String)

/* -------------------- Fixtures -------------------- */

static HildonCaption *caption = NULL;
static const gchar *DEFAULT_TEST_CAPTION = "Default caption";
static GtkWidget *showed_window = NULL;

static void 
fx_setup_default_caption()
{
  int argc = 0;
  GtkWidget *control = NULL;
  GtkWidget *icon = NULL;
  GtkSizeGroup *group = NULL;

  gtk_init(&argc, NULL);

  control = gtk_entry_new();
  caption = HILDON_CAPTION(hildon_caption_new(group, 
					      DEFAULT_TEST_CAPTION, 
					      control, 
					      icon,
					      HILDON_CAPTION_OPTIONAL));

  showed_window =  create_test_window ();
 
  /* This packs the widget into the window (a gtk container). */
  gtk_container_add (GTK_CONTAINER (showed_window), GTK_WIDGET (caption));
  
  /* Displays the widget and the window */
  show_test_window (showed_window);
  
  show_test_window (GTK_WIDGET(caption));

  /* Check that the hildon-caption object has been created properly */
  fail_if (!HILDON_IS_CAPTION (caption), "hildon-caption: Creation failed.");
   
}

static void 
fx_teardown_default_caption()
{
  /* Destroy caption */
  gtk_widget_destroy (GTK_WIDGET(caption));

  /* Destroy the window */
  gtk_widget_destroy (showed_window);

}


/* -------------------- Test cases -------------------- */

/* ----- Test case for is_mandatory -----*/

/**
 * Purpose: test getting the mandatory property of a HildonCaption
 * Cases considered:
 *    - check an optional captioned control
 *    - check a mandatory captioned control
 */

START_TEST (test_is_mandatory_regular)
{
  GtkWidget *control = NULL, *icon = NULL;
  GtkSizeGroup *group = NULL;

  /* Test 1: check an optional (i.e. NOT mandatory) caption */
  fail_if (hildon_caption_is_mandatory (caption) == TRUE, 
	   "hildon-caption: The created HildonCaption has a mandatory captioned control and it shouldn't");
 
  gtk_widget_destroy (GTK_WIDGET (caption));

  /* Test 2: create a mandatory caption */
  control = GTK_WIDGET (gtk_entry_new());
  caption = HILDON_CAPTION (hildon_caption_new(group, 
					       "Default caption", 
					       control, 
					       icon,
					       HILDON_CAPTION_MANDATORY));
  
  fail_if (hildon_caption_is_mandatory (caption) == FALSE, 
	   "hildon-caption: The created HildonCaption has a mandatory captioned control and it shouldn't");
}
END_TEST

/**
 * Purpose: test is_mandatory with invalid arguments
 * Cases considered:
 *    - use a NULL HildonCaption
 *    - use an object that is not a HildonCaption (a HBox)
 */
START_TEST (test_is_mandatory_invalid)
{
  GtkWidget *hbox = NULL;

  /* Test 1: NULL HildonCaption */
  fail_if (hildon_caption_is_mandatory (NULL) == TRUE, 
	   "hildon-caption: a NULL HildonCaption should fail and return FALSE");

  /* Test 2: invalid HildonCaption */
  hbox = gtk_hbox_new (FALSE, 0);
  if (hildon_caption_is_mandatory ((HildonCaption *) (hbox)) == TRUE) 
    {
      gtk_widget_destroy (GTK_WIDGET (hbox));
      fail ("hildon-caption: an invalid HildonCaption should return FALSE");
    }

  gtk_widget_destroy (GTK_WIDGET (hbox));
}
END_TEST

/* ----- Test case for set_status -----*/
	
/**
 * Purpose: test setting the status of a HildonCaption
 * Cases considered:
 *    - Set the status HILDON_CAPTION_MANDATORY
 */
START_TEST (test_set_status_regular)
{

  hildon_caption_set_status (caption, HILDON_CAPTION_MANDATORY);

  fail_if (hildon_caption_get_status (caption) != HILDON_CAPTION_MANDATORY,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (caption), HILDON_CAPTION_MANDATORY);
}
END_TEST

/**
 * Purpose: test setting some status passing invalid parameters
 * Cases considered:
 *    - set the status 8
 *    - set the status to a NULL HildonCaption
 *    - set the status to an invalid HildonCaption (use a HBox instead)
 */
START_TEST (test_set_status_invalid)
{
  const guint INVALID_ENUM = 8;
  GtkWidget *hbox = NULL;

  /* Test 1 */
  hildon_caption_set_status (caption, INVALID_ENUM);

  fail_if (hildon_caption_get_status (caption) != HILDON_CAPTION_OPTIONAL,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (caption), HILDON_CAPTION_OPTIONAL);

  /* Test 2 */
  hildon_caption_set_status (NULL, HILDON_CAPTION_MANDATORY);

  fail_if (hildon_caption_get_status (caption) != HILDON_CAPTION_OPTIONAL,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (caption), HILDON_CAPTION_OPTIONAL);

  /* Test 3 */
  hbox = gtk_hbox_new (FALSE, 0);
  hildon_caption_set_status ((HildonCaption *) (hbox), HILDON_CAPTION_MANDATORY);
  if (hildon_caption_get_status (caption) != HILDON_CAPTION_OPTIONAL) 
    {
      gtk_widget_destroy (GTK_WIDGET (hbox));
      fail ("hildon-caption: the returned status is %d and should be %d",
            hildon_caption_get_status (caption), HILDON_CAPTION_OPTIONAL);
    }

  gtk_widget_destroy (GTK_WIDGET (hbox));
}
END_TEST

/* ----- Test case for get_status -----*/

/**
 * Purpose: test getting the status of a HildonCaption
 * Cases considered:
 *    - Get HILDON_CAPTION_OPTIONAL set in the method creation
 *    - Get HILDON_CAPTION_MANDATORY set with set_status
 *    - Get HILDON_CAPTION_OPTIONAL set with set_property
 */
START_TEST (test_get_status_regular)
{
  GValue value = {0, };

  /* Test 1: set status in object creation */
  fail_if (hildon_caption_get_status (caption) != HILDON_CAPTION_OPTIONAL,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (caption), HILDON_CAPTION_OPTIONAL);
  
  /* Test 2: set status with set_status */
  hildon_caption_set_status (caption, HILDON_CAPTION_MANDATORY);
  fail_if (hildon_caption_get_status (caption) != HILDON_CAPTION_MANDATORY,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (caption), HILDON_CAPTION_MANDATORY);
  
  /* Test 3: set status with set_property */
  g_value_init (&value, G_TYPE_UINT);
  g_value_set_uint (&value, HILDON_CAPTION_OPTIONAL);

  g_object_set_property (G_OBJECT (caption), "status", &value);
  fail_if (hildon_caption_get_status (caption) != HILDON_CAPTION_OPTIONAL,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (caption), HILDON_CAPTION_OPTIONAL);
}
END_TEST

/**
 * Purpose: test get_status with invalid values
 * Cases considered:
 *    - Get the status with a NULL HildonCaption
 *    - Get the status with an invalid HildonCaption (used a HBox)
 */
START_TEST (test_get_status_invalid)
{
  GtkWidget *hbox = NULL;

  /* Test 1: check a NULL caption */
  fail_if (hildon_caption_get_status (NULL) != HILDON_CAPTION_OPTIONAL,
	   "hildon-caption: the returned status is %d and should be %d",
	   hildon_caption_get_status (NULL), HILDON_CAPTION_OPTIONAL);

  /* Test 2: check an invalid HildonCaption */
  hbox = gtk_hbox_new (FALSE, 0);
  if (hildon_caption_get_status ((HildonCaption *) (hbox)) != HILDON_CAPTION_OPTIONAL) 
    {
      gtk_widget_destroy (GTK_WIDGET (hbox));
      fail ("hildon-caption: an invalid HildonCaption should return HILDON_CAPTION_OPTIONAL");
    }

  gtk_widget_destroy (GTK_WIDGET (hbox));
}
END_TEST

/* ----- Test case for set_label -----*/

/**
 * Purpose: test setting labels for a HildonCaption
 * Cases considered:
 *    - Set a test label
 *    - Set an empty label
 */
START_TEST (test_set_label_regular)
{
  const gchar *TEST_LABEL = TEST_STRING;
  gchar * expected_ret_label=NULL;
  
  /* We control i18n so we will never set it properly because apparently it will not be useful for testing */
  /* so _("ecdg_ti_caption_separato") should return the same result that "ecdg_ti_caption_separator" */
  /* If in the future we decide activate internationalization we must modify test implementation */
  expected_ret_label = g_strconcat(TEST_LABEL,_("ecdg_ti_caption_separator"),NULL);

  /* Test 1 */
  hildon_caption_set_label (caption, TEST_LABEL);

  /*  fail_if (strcmp (hildon_caption_get_label (caption), TEST_LABEL) != 0,
      "hildon-caption: the returned label is %s and should be %s",
      hildon_caption_get_label (caption), TEST_LABEL);
  */
  fail_if (strcmp (hildon_caption_get_label (caption), expected_ret_label) != 0,
	   "hildon-caption: the returned label is %s and should be %s",
	   hildon_caption_get_label (caption), expected_ret_label);
  
  g_free(expected_ret_label);

  /* Test 2 */
  hildon_caption_set_label (caption, "");
  
  fail_if (strcmp (hildon_caption_get_label (caption),_("ecdg_ti_caption_separator")) != 0,
           "hildon-caption: the returned label is %s and should be default separator",
           hildon_caption_get_label (caption));
}
END_TEST

/**
 * Purpose: test setting labels with invalid parameters
 * Cases considered:
 *    - Set a NULL label
 *    - set a label to a NULL HildonCaption
 *    - set a label to an invalid HildonCaption (use a GtkHbox instead)
 */
START_TEST (test_set_label_invalid)
{
  const gchar *TEST_LABEL = TEST_STRING;
  GtkWidget *hbox = NULL;

  /* Test 1 */
  hildon_caption_set_label (caption, NULL);

  fail_if (strcmp (hildon_caption_get_label (caption), "") != 0,
	   "hildon-caption: the returned label is %s and should be empty",
	   hildon_caption_get_label (caption));

  /* Test 2 */
  hildon_caption_set_label (NULL, TEST_LABEL);

  fail_if (strcmp (hildon_caption_get_label (caption), "") != 0,
	   "hildon-caption: the returned label is %s and should be default separator",
	   hildon_caption_get_label (caption), "");

  /* Test 3 */
  hbox = gtk_hbox_new (FALSE, 0);
  hildon_caption_set_label ((HildonCaption *) (hbox), TEST_LABEL);
  if (strcmp (hildon_caption_get_label ((HildonCaption *) (hbox)), "")!=0) 
    {
      gtk_widget_destroy (GTK_WIDGET (hbox));
      fail ("hildon-caption: an invalid HildonCaption should return NULL");
    }

  gtk_widget_destroy (GTK_WIDGET (hbox));
}
END_TEST

/* ----- Test case for get_label -----*/

/**
 * Purpose: test getting a valid value for a label previously set
 * Cases considered:
 *    - get label set with set_label
 *    - get empty label set with set_label
 */
START_TEST (test_get_label_regular)
{
  const gchar *TEST_LABEL = TEST_STRING;
  gchar * expected_ret_label=NULL;

  /* Test 1 */
  hildon_caption_set_label (caption, TEST_LABEL);
  expected_ret_label = g_strconcat(TEST_LABEL, _("ecdg_ti_caption_separator"),NULL);

  fail_if (strcmp (hildon_caption_get_label (caption), expected_ret_label) != 0,
	   "hildon-caption: the returned label is %s and should be %s",
	   hildon_caption_get_label (caption), expected_ret_label);

  g_free(expected_ret_label);

  /* Test 2 */
  hildon_caption_set_label (caption, "");

  fail_if (strcmp (hildon_caption_get_label (caption), _("ecdg_ti_caption_separator")) != 0,
	   "hildon-caption: the returned label is %s and should be default separator",
	   hildon_caption_get_label (caption));
}
END_TEST

/**
 * Purpose: test getting labels with invalid arguments
 * Cases considered:
 *    - get NULL label set with set_property
 *    - get the default label with a NULL HildonCaption
 *    - get the default label with an invalid HildonCaption (used a GtkHBox instead)
 */
START_TEST (test_get_label_invalid)
{
  GValue value = {0, };
  GtkWidget *hbox = NULL;
  const gchar *EMPTY_STRING = "";

  /* Test 1 */
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, NULL);
  g_object_set_property (G_OBJECT (caption), "label", &value);

  fail_if (strcmp(hildon_caption_get_label (caption), EMPTY_STRING) != 0,
	   "hildon-caption: the returned label is %s and should be empty",
	   hildon_caption_get_label (caption));

  /* Test 2: check a NULL caption */
  fail_if (strcmp (hildon_caption_get_label (NULL), EMPTY_STRING) != 0,
	   "hildon-caption: the returned label is %s and should be empty",
	   hildon_caption_get_label (NULL));

  /* Test 3: check an invalid HildonCaption */
  hbox = gtk_hbox_new (FALSE, 0);
  if (strcmp (hildon_caption_get_label ((HildonCaption *) (hbox)), EMPTY_STRING)) 
    {
      gtk_widget_destroy (GTK_WIDGET (hbox));
      fail ("hildon-caption: an invalid HildonCaption should return %s", EMPTY_STRING);
    }

  gtk_widget_destroy (GTK_WIDGET (hbox));
}
END_TEST

/* ---------- Suite creation ---------- */

Suite *create_hildon_caption_suite()
{
  /* Create the suite */
  Suite *s = suite_create("HildonCaption");

  /* Create test cases */
  TCase *tc1 = tcase_create("is_mandatory");
  TCase *tc2 = tcase_create("set_status");
  TCase *tc3 = tcase_create("get_status");
  TCase *tc4 = tcase_create("set_label");
  TCase *tc5 = tcase_create("get_label");

  /* Create test case for is_mandatory and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_caption, fx_teardown_default_caption);
  tcase_add_test(tc1, test_is_mandatory_regular);
  tcase_add_test(tc1, test_is_mandatory_invalid);
  suite_add_tcase (s, tc1);

  /* Create test case for set_status and add it to the suite */
  tcase_add_checked_fixture(tc2, fx_setup_default_caption, fx_teardown_default_caption);
  tcase_add_test(tc2, test_set_status_regular);
  tcase_add_test(tc2, test_set_status_invalid);
  suite_add_tcase (s, tc2);

  /* Create test case for get_status and add it to the suite */
  tcase_add_checked_fixture(tc3, fx_setup_default_caption, fx_teardown_default_caption);
  tcase_add_test(tc3, test_get_status_regular);
  tcase_add_test(tc3, test_get_status_invalid);
  suite_add_tcase (s, tc3);

  /* Create test case for set_label and add it to the suite */
  tcase_add_checked_fixture(tc4, fx_setup_default_caption, fx_teardown_default_caption);
  tcase_add_test(tc4, test_set_label_regular);
  tcase_add_test(tc4, test_set_label_invalid);
  suite_add_tcase (s, tc4);

  /* Create test case for get_label and add it to the suite */
  tcase_add_checked_fixture(tc5, fx_setup_default_caption, fx_teardown_default_caption);
  tcase_add_test(tc5, test_get_label_regular);
  tcase_add_test(tc5, test_get_label_invalid);
  suite_add_tcase (s, tc5);

  /* Return created suite */
  return s;             
}
