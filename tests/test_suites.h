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

#ifndef _TEST_SUITES_H_
#define _TEST_SUITES_H_

#define TEST_STRING "aBcD0123-_$%\\/(]{=?! <:;.,*+'^`\"|@#~"

Suite *create_hildon_range_editor_suite(void);
Suite *create_hildon_date_editor_suite(void);
Suite *create_hildon_number_editor_suite(void);
Suite *create_hildon_time_editor_suite(void);
Suite *create_hildon_time_picker_suite(void);
Suite *create_hildon_calendar_popup_suite(void);
Suite *create_hildon_weekday_picker_suite(void);
Suite *create_hildon_controlbar_suite(void);
Suite *create_hildon_color_button_suite(void);
Suite *create_hildon_color_chooser_suite(void);
Suite *create_hildon_seekbar_suite(void);
Suite *create_hildon_dialoghelp_suite(void);
Suite *create_hildon_caption_suite(void);
Suite *create_hildon_helper_suite(void);
Suite *create_hildon_find_toolbar_suite(void);
Suite *create_hildon_name_password_dialog_suite(void);
Suite *create_hildon_get_password_dialog_suite(void);
Suite *create_hildon_set_password_dialog_suite(void);
Suite *create_hildon_sort_dialog_suite(void);
Suite *create_hildon_code_dialog_suite(void);
Suite *create_hildon_note_suite(void);
Suite *create_hildon_volumebar_suite(void);
Suite *create_hildon_volumebar_range_suite(void);
Suite *create_hildon_wizard_dialog_suite(void);
Suite *create_hildon_font_selection_dialog_suite(void);
Suite *create_hildon_system_sound_suite(void);
Suite *create_hildon_scroll_area_suite(void);
Suite *create_hildon_window_suite(void);
Suite *create_hildon_program_suite(void);
Suite *create_hildon_composite_widget_suite(void);
Suite *create_hildon_picker_button_suite (void);

#endif
