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
#include                                        "hildon.h"

int
main                                            (int argc, 
                                                 char **args)
{
    gtk_init (&argc, &args);

    g_debug ("Small pixel size: %d", HILDON_ICON_PIXEL_SIZE_SMALL);
    g_debug ("Wizard pixel size: %d", HILDON_ICON_PIXEL_SIZE_WIZARD);
    g_debug ("Toolbar pixel size: %d", HILDON_ICON_PIXEL_SIZE_TOOLBAR);
    g_debug ("Note pixel size: %d", HILDON_ICON_PIXEL_SIZE_NOTE);
    g_debug ("Big note pixel size: %d", HILDON_ICON_PIXEL_SIZE_BIG_NOTE);
    
    return 0;
}


