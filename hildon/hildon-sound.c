/*
 * This file is a part of libhildon
 *
 * Copyright (C) 2005-2008 Nokia Corporation. All rights reserved.
 *
 * Contact: Kimmo Hämäläinen <kimmo.hamalainen@nokia.com>
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

/**
 * SECTION:hildon-sound
 * @short_description: libcanberra-based utility function for playing a sound.
 *
 * Please note that this method is only provided for backwards compatibility,
 * but we highly recommend you to use canberra-gtk directly instead.
 * 
 */

#include <unistd.h>
#include <gconf/gconf-client.h>
#include <canberra.h>

#include "hildon-sound.h"

#define ALARM_GCONF_PATH "/apps/osso/sound/system_alert_volume"

static ca_context *hildon_ca_context_get (void);

/*
 * hildon_ca_context_get:
 *
 * hildon maintains a single application-global ca_context object.
 *
 * This functions is based on ca_gtk_context_get
 *
 * Returns: a ca_context object
 */
static ca_context *
hildon_ca_context_get (void)
{
    static GStaticPrivate context_private = G_STATIC_PRIVATE_INIT;
    ca_context *c = NULL;
    const gchar *name = NULL;
    gint ret;

    if ((c = g_static_private_get(&context_private)))
        return c;

    if ((ret = ca_context_create(&c)) != CA_SUCCESS) {
        g_warning("ca_context_create: %s\n", ca_strerror(ret));
        return NULL;
    }
    if ((ret = ca_context_open(c)) != CA_SUCCESS) {
        g_warning("ca_context_open: %s\n", ca_strerror(ret));
        ca_context_destroy(c);
        return NULL;
    }

    if ((name = g_get_application_name()))
        ca_context_change_props(c, CA_PROP_APPLICATION_NAME, name, NULL);

    g_static_private_set(&context_private, c, (GDestroyNotify) ca_context_destroy);

    return c;
}

/**
 * hildon_play_system_sound:
 * @sample: sound file to play
 * 
 * Plays the given sample using libcanberra.
 * Volume level is received from gconf.
 *
 * This method sets the "dialog-information" role for the sound played,
 * so you need to keep this into account when using it. For any purpose, it
 * is highly recommended that you use canberra-gtk instead of this method.
 */
void 
hildon_play_system_sound(const gchar *sample)
{
    int ret;
    ca_context *ca_con = NULL;
    ca_proplist *pl = NULL;

    ca_con = hildon_ca_context_get ();

    ca_proplist_create(&pl);
    ca_proplist_sets(pl, CA_PROP_MEDIA_FILENAME, sample);
    ca_proplist_sets(pl, CA_PROP_MEDIA_ROLE, "dialog-information");
    ca_proplist_sets(pl, "module-stream-restore.id", "x-maemo-system-sound");

    ret = ca_context_play_full(ca_con, 0, pl, NULL, NULL);

    ca_proplist_destroy(pl);
}
