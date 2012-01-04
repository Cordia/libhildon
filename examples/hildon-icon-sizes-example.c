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

    g_debug ("Extra small pixel size: %d", HILDON_ICON_PIXEL_SIZE_XSMALL);
    g_debug ("Small pixel size: %d", HILDON_ICON_PIXEL_SIZE_SMALL);
    g_debug ("Stylus pixel size: %d", HILDON_ICON_PIXEL_SIZE_STYLUS);
    g_debug ("Finger pixel size: %d", HILDON_ICON_PIXEL_SIZE_FINGER);
    g_debug ("Thumb pixel size: %d", HILDON_ICON_PIXEL_SIZE_THUMB);
    g_debug ("Large pixel size: %d", HILDON_ICON_PIXEL_SIZE_LARGE);
    g_debug ("Extra large pixel size: %d", HILDON_ICON_PIXEL_SIZE_XLARGE);
    g_debug ("Toolbar pixel size: %d", HILDON_ICON_PIXEL_SIZE_FINGER);

    return 0;
}


