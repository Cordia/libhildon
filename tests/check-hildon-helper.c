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
#include <gtk/gtk.h>
#include "test_suites.h"
#include <hildon/hildon-helper.h>

/* -------------------- Fixtures -------------------- */

static void 
fx_setup_default_helper ()
{
  int argc = 0;

  gtk_init(&argc, NULL);
}

static void 
fx_teardown_default_helper ()
{
}
/* -------------------- Test cases -------------------- */

/* ----- Test case for hildon_helper_set_logical_font -----*/

/**
 * Purpose: test setting a new logical font to a GtkWidget
 * Cases considered:
 *    - Set the font name "TimesNewRoman"
 */
START_TEST (test_hildon_helper_set_logical_font_regular)
{
  GtkWidget *label = NULL;
  gulong signum = G_MAXULONG;

  label = gtk_label_new ("test label");

  signum = hildon_helper_set_logical_font (label, "TimesNewRoman");
  gtk_widget_destroy (GTK_WIDGET (label));

  fail_if (signum <= 0,
	   "hildon-helper: the returned signal id is %ul and should be > 0",
	   signum);
}
END_TEST

/**
 * Purpose: test setting a logical font with invalid parameters
 * Cases considered:
 *    - Set the font name "TimesNewRoman" to a NULL Widget
 *    - Set a NULL font name to a valid Widget
 */
START_TEST (test_hildon_helper_set_logical_font_invalid)
{
  GtkWidget *label = NULL;
  gulong signum = G_MAXULONG;

  /* Test 1 */
  signum = hildon_helper_set_logical_font (NULL, "TimesNewRoman");
  fail_if (signum != 0,
	   "hildon-helper: the returned signal id is %ul and should be 0",
	   signum);

  /* Test 2 */
  label = gtk_label_new ("test label");

  signum = hildon_helper_set_logical_font (label, NULL);
  gtk_widget_destroy (GTK_WIDGET (label));

  fail_if (signum != 0,
	   "hildon-helper: the returned signal id is %ul and should be 0",
	   signum);
}
END_TEST


/* ----- Test case for hildon_helper_set_logical_color -----*/

/**
 * Purpose: test setting a new logical color to a GtkWidget
 * Cases considered:
 *    - Set the logical color "Blue"
 */
START_TEST (test_hildon_helper_set_logical_color_regular)
{
  GtkWidget *label = NULL;
  gulong signum = G_MAXULONG;

  label = gtk_label_new ("test label");

  signum = hildon_helper_set_logical_color (label, 
						GTK_RC_BG, 
						GTK_STATE_NORMAL, 
						"Blue");
  gtk_widget_destroy (GTK_WIDGET (label));

  fail_if (signum <= 0,
	   "hildon-helper: the returned signal id is %ul and should be > 0",
	   signum);
}
END_TEST

/**
 * Purpose: test setting a logical color with invalid parameters
 * Cases considered:
 *    - Set the color name "Blue" to a NULL Widget
 *    - Set a NULL color name to a valid Widget
 */
START_TEST (test_hildon_helper_set_logical_color_invalid)
{
  GtkWidget *label = NULL;
  gulong signum = G_MAXULONG;

  /* Test 1 */
  signum = hildon_helper_set_logical_color (NULL,
						GTK_RC_BG, 
						GTK_STATE_NORMAL, 
						"Blue");
  fail_if (signum != 0,
	   "hildon-helper: the returned signal id is %ul and should be 0",
	   signum);

  /* Create the widget */
  label = gtk_label_new ("test label");

  /* Test 2 */
  signum = hildon_helper_set_logical_color (label,
						GTK_RC_BG, 
						GTK_STATE_NORMAL, 
						NULL);
  gtk_widget_destroy (GTK_WIDGET (label));

  fail_if (signum != 0,
	   "hildon-helper: the returned signal id is %ul and should be 0",
	   signum);
}
END_TEST



/* ---------- Suite creation ---------- */

Suite *create_hildon_helper_suite()
{
  /* Create the suite */
  Suite *s = suite_create("Hildonhelper");

  /* Create test cases */
  TCase *tc1 = tcase_create("hildon_helper_set_logical_font");
  TCase *tc2 = tcase_create("hildon_helper_set_logical_color");

  /* Create test case for set_logical_font and add it to the suite */
  tcase_add_checked_fixture(tc1, fx_setup_default_helper, fx_teardown_default_helper);
  tcase_add_test(tc1, test_hildon_helper_set_logical_font_regular);
  tcase_add_test(tc1, test_hildon_helper_set_logical_font_invalid);
  suite_add_tcase (s, tc1);

  /* Create test case for set_logical_color and add it to the suite */
  tcase_add_checked_fixture(tc2, fx_setup_default_helper, fx_teardown_default_helper);
  tcase_add_test(tc2, test_hildon_helper_set_logical_color_regular);
  tcase_add_test(tc2, test_hildon_helper_set_logical_color_invalid);
  suite_add_tcase (s, tc2);

  /* Return created suite */
  return s;             
}
