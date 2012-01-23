/*
 * This file is a part of hildon tests
 *
 * Copyright (C) 2009 Nokia Corporation, all rights reserved.
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
#include <hildon/hildon.h>

static HildonButton *button = NULL;
static HildonTouchSelector *selector = NULL;
static GtkWindow *window = NULL;

static void
fx_setup ()
{
    int argc = 0;

    gtk_init (&argc, NULL);

    window = GTK_WINDOW (hildon_window_new());

    fail_if (!HILDON_IS_WINDOW(window),
	     "hildon-picker-button: Window creation failed.");

    button = HILDON_BUTTON (hildon_picker_button_new (HILDON_SIZE_AUTO,
                              HILDON_BUTTON_ARRANGEMENT_VERTICAL));
    selector = HILDON_TOUCH_SELECTOR (hildon_touch_selector_new_text ());

    hildon_touch_selector_append_text (selector, "Row one");
    hildon_touch_selector_append_text (selector, "Row two");
    hildon_touch_selector_append_text (selector, "Row three");
    hildon_touch_selector_append_text (selector, "Row four");

    hildon_picker_button_set_selector (HILDON_PICKER_BUTTON (button), selector);

    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (button));
    gtk_widget_show (GTK_WIDGET (button));

    show_test_window (GTK_WIDGET (window));
}

static void
fx_teardown ()
{
    gtk_widget_destroy (GTK_WIDGET (window));
}

/**
   Purpose: test that programmatic changes in the selector update the
   value displayed in the button.

   Checks for:

   - Initial value must be empty.
   - Programmatically selecting an item in the selector should show it in the button.
   - Updating the contents of a selected row should update the displayed value accordingly.

*/
START_TEST (test_hildon_picker_button_value)
{
    const gchar *value;
    GtkTreeIter iter;
    GtkTreeModel *model;
    const gchar *new_value = "Something different";

    /* Test 1: getting initial value. */
    value = hildon_button_get_value (button);
    fail_if (strcmp (value, "") != 0,
             "hildon-picker-button: default value wrong: `%s'",
             value);

    /* Test 2: Selecting a particular row. */
    hildon_touch_selector_set_active (selector, 0, 1);
    value = hildon_button_get_value (button);
    fail_if (strcmp (value, "Row two") != 0,
             "hildon-picker-button: switched to second row in the selector, "
             "but button displays `%s'.", value);

    /* Test 3: Modifying a selected row. */
    model = hildon_touch_selector_get_model (selector, 0);
    hildon_touch_selector_get_selected (selector, 0, &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        0, new_value, -1);
    value = hildon_button_get_value (button);
    fail_if (strcmp (value, new_value) != 0,
             "hildon-picker-button: set the currently selected row to `%s' "
             "but button displays `%s'.", new_value, value);

}
END_TEST

Suite *create_hildon_picker_button_suite (void)
{
    Suite *s = suite_create ("HildonPickerButton");

    TCase *tc1 = tcase_create ("hildon_picker_button");
    tcase_add_checked_fixture (tc1, fx_setup, fx_teardown);
    tcase_add_test (tc1, test_hildon_picker_button_value);
    suite_add_tcase (s, tc1);

    return s;
}
