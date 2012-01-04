/*
 * This file is a part of hildon
 *
 * Copyright (C) 2008, 2009 Nokia Corporation, all rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 */

/**
 * SECTION:hildon-entry
 * @short_description: Text entry in the Hildon framework.
 *
 * #HildonEntry is text entry derived from the #GtkEntry widget but
 * with a slightly different appearance designed for the Hildon 2.2
 * framework.
 *
 * Text entries in Hildon 2.2 can have a placeholder text, set with
 * hildon_gtk_entry_set_placeholder_text(). This text will be shown if
 * the entry is empty and doesn't have the input focus, but it's
 * otherwise ignored. Thus, calls to gtk_entry_get_text() will never
 * return the placeholder text, not even when it's being displayed.
 *
 * <example>
 * <title>Creating a HildonEntry with a placeholder</title>
 * <programlisting>
 * GtkWidget *
 * create_entry (void)
 * {
 *     GtkWidget *entry;
 * <!-- -->
 *     entry = hildon_entry_new (HILDON_SIZE_AUTO);
 *     hildon_gtk_entry_set_placeholder_text (GTK_ENTRY (entry), "First name");
 * <!-- -->
 *     return entry;
 * }
 * </programlisting>
 * </example>
 */

#include                                        "hildon-entry.h"
#include					"hildon-enum-types.h"

G_DEFINE_TYPE                                   (HildonEntry, hildon_entry, GTK_TYPE_ENTRY);

enum {
    PROP_SIZE = 1
};

static void
set_property                                    (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
    HildonSizeType size;

    switch (prop_id)
    {
    case PROP_SIZE:
	size = g_value_get_flags (value);
	/* If this is auto height, default to finger height. */
	if (!(size & (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_THUMB_HEIGHT)))
	  size |= HILDON_SIZE_FINGER_HEIGHT;
	hildon_gtk_widget_set_theme_size (GTK_WIDGET (object), size);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

/**
 * hildon_entry_new:
 * @size: The size of the entry
 *
 * Creates a new entry.
 *
 * Returns: a new #HildonEntry
 *
 * Since: 2.2
 */
GtkWidget *
hildon_entry_new                                (HildonSizeType size)
{
    return g_object_new (HILDON_TYPE_ENTRY, "size", size, NULL);
}

static void
hildon_entry_class_init                         (HildonEntryClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->set_property = set_property;

    g_object_class_install_property (
        gobject_class,
        PROP_SIZE,
        g_param_spec_flags (
            "size",
            "Size",
            "Size request for the entry",
            HILDON_TYPE_SIZE_TYPE,
            HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
            G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));
}

static void
hildon_entry_init                               (HildonEntry *self)
{
    self->priv = NULL;
}
