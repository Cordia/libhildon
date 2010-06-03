/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
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
 * SECTION:hildon-font-selection-dialog
 * @short_description: A widget used to allow users to select a font 
 * with certain properties.
 *
 * Font selection can be made using this widget. Users can select font name, 
 * size, style, etc. Since hildon 2.2, the previously available preview dialog
 * has been removed.
 */

#undef                                          HILDON_DISABLE_DEPRECATED

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <libintl.h>
#include                                        <stdlib.h>
#include                                        <string.h>

#include                                        <glib.h>
#include                                        <gdk/gdkkeysyms.h>

#include                                        "hildon-font-selection-dialog.h"
#include                                        "hildon-caption.h"
#include                                        "hildon-color-button.h"
#include                                        "hildon-font-selection-dialog-private.h"
#include                                        "hildon-stock.h"

/* These are what we use as the standard font sizes, for the size list */

static const guint16                            font_sizes[] = 
{
  6, 8, 10, 12, 16, 24, 32
};

enum
{
    PROP_0,
    PROP_FAMILY,
    PROP_FAMILY_SET,
    PROP_SIZE,
    PROP_SIZE_SET,
    PROP_COLOR,
    PROP_COLOR_SET,
    PROP_BOLD,
    PROP_BOLD_SET,
    PROP_ITALIC,
    PROP_ITALIC_SET,
    PROP_UNDERLINE,
    PROP_UNDERLINE_SET,
    PROP_STRIKETHROUGH,
    PROP_STRIKETHROUGH_SET,
    PROP_POSITION,
    PROP_POSITION_SET,
    PROP_PREVIEW_TEXT,
    PROP_FONT_SCALING
};

/* combo box active row indicator -2--inconsistent, -1--undefined 
 * please make sure that you use settings_init settings_apply
 * and settings_destroy, dont even try to touch this structure 
 * without using the three above interface functions, of course
 * if you know what you are doing, do as you please ;-)*/
typedef struct
{
    HildonFontSelectionDialog *fsd;             /* pointer to our font selection dialog */

    gint family;                                /* combo box indicator */
    gint size;                                  /* combo box indicator */
    GdkColor *color;                            /* free after read the setting */
    gboolean color_inconsist;
    gint weight;                                /* bit mask */
    gint style;                                 /* bit mask */
    gint underline;                             /* bit mask */
    gint strikethrough;                         /* bit mask */
    gint position;                              /* combo box indicator */

}                                               HildonFontSelectionDialogSettings;

#if 0
static gboolean
hildon_font_selection_dialog_preview_key_press  (GtkWidget *widget,
                                                 GdkEventKey *event,
                                                 gpointer unused);
#endif

static int
cmp_families                                    (const void *a, 
                                                 const void *b);
#if 0
static void   
hildon_font_selection_dialog_show_preview       (HildonFontSelectionDialog *fontsel);

static PangoAttrList*
hildon_font_selection_dialog_create_attrlist    (HildonFontSelectionDialog *fontsel, 
                                                 guint start_index,
                                                 guint len);
#endif

static void   
hildon_font_selection_dialog_show_available_positionings (HildonFontSelectionDialogPrivate *priv);

static void   
hildon_font_selection_dialog_show_available_fonts (HildonFontSelectionDialog *fontsel);

static void   
hildon_font_selection_dialog_show_available_sizes (HildonFontSelectionDialogPrivate *priv);

static void   
hildon_font_selection_dialog_class_init         (HildonFontSelectionDialogClass *klass);

static void   
hildon_font_selection_dialog_init               (HildonFontSelectionDialog *fontseldiag);

static void   
hildon_font_selection_dialog_finalize           (GObject *object);

static void   
hildon_font_selection_dialog_construct_notebook (HildonFontSelectionDialog *fontsel);

static void   
color_modified_cb                               (HildonColorButton *button,
                                                 GParamSpec *pspec,
                                                 gpointer fsd_priv);

#if 0
static void   
add_preview_text_attr                           (PangoAttrList *list, 
                                                 PangoAttribute *attr, 
                                                 guint start, 
                                                 guint len);
#endif

static void   
toggle_clicked                                  (GtkButton *button, 
                                                 gpointer unused);

static GtkDialogClass*                          parent_class = NULL;

#define                                         _(String) dgettext("hildon-libs", String)

#define                                         SUPERSCRIPT_RISE 3333

#define                                         SUBSCRIPT_LOW -3333

#define                                         ON_BIT  0x01

#define                                         OFF_BIT 0x02

/**
 * hildon_font_selection_dialog_get_type:
 *
 * Initializes and returns the type of a hildon font selection dialog
 *
 * Returns: GType of #HildonFontSelectionDialog
 */
GType G_GNUC_CONST
hildon_font_selection_dialog_get_type           (void)
{
    static GType font_selection_dialog_type = 0;

    if (! font_selection_dialog_type) {
        static const GTypeInfo fontsel_diag_info = {
            sizeof(HildonFontSelectionDialogClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) hildon_font_selection_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(HildonFontSelectionDialog),
            0,  /* n_preallocs */
            (GInstanceInitFunc) hildon_font_selection_dialog_init,
        };

        font_selection_dialog_type =
            g_type_register_static (GTK_TYPE_DIALOG,
                    "HildonFontSelectionDialog",
                    &fontsel_diag_info, 0);
    }

    return font_selection_dialog_type;
}

static void
hildon_font_selection_dialog_get_property       (GObject *object,
                                                 guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    gint i;
    GdkColor color;

    HildonFontSelectionDialogPrivate *priv =
        HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE(object);

    g_assert (priv);

    switch (prop_id)
    {
        case PROP_FAMILY:
            i = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->cbx_font_type));
            if(i >= 0 && i < priv->n_families)
                g_value_set_string(value, 
                        pango_font_family_get_name (priv->families[i]));
            else
                g_value_set_string (value, "Sans");
                /* FIXME Bad hardcoding here */
            break;

        case PROP_FAMILY_SET:
            i = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->cbx_font_type));
            if(i >= 0 && i < priv->n_families)
                g_value_set_boolean (value, TRUE);
            else
                g_value_set_boolean (value, FALSE);
            break;

        case PROP_SIZE:
            i = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->cbx_font_size));
            if(i >= 0 && i < G_N_ELEMENTS (font_sizes))
                g_value_set_int (value, font_sizes[i]);
            else
                g_value_set_int (value, 16);
            break;

        case PROP_SIZE_SET:
            i = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->cbx_font_size));
            if(i >= 0 && i < G_N_ELEMENTS (font_sizes))
                g_value_set_boolean (value, TRUE);
            else
                g_value_set_boolean (value, FALSE);
            break;

        case PROP_COLOR:
            hildon_color_button_get_color
                (HILDON_COLOR_BUTTON (priv->font_color_button), &color);
            g_value_set_boxed (value, (gconstpointer) &color);
            break;

        case PROP_COLOR_SET:
            g_value_set_boolean (value, priv->color_set);
            break;

        case PROP_BOLD:
            g_value_set_boolean (value, 
                    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->chk_bold)));
            break;

        case PROP_BOLD_SET:
            g_value_set_boolean (value,
                    ! gtk_toggle_button_get_inconsistent
                    (GTK_TOGGLE_BUTTON (priv->chk_bold)));
            break;

        case PROP_ITALIC:
            g_value_set_boolean (value, 
                    gtk_toggle_button_get_active
                    (GTK_TOGGLE_BUTTON (priv->chk_italic)));
            break;

        case PROP_ITALIC_SET:
            g_value_set_boolean (value,
                    ! gtk_toggle_button_get_inconsistent
                    (GTK_TOGGLE_BUTTON (priv->chk_italic)));
            break;

        case PROP_UNDERLINE:
            g_value_set_boolean (value, 
                    gtk_toggle_button_get_active
                    (GTK_TOGGLE_BUTTON (priv->chk_underline)));
            break;

        case PROP_UNDERLINE_SET:
            g_value_set_boolean (value,
                    ! gtk_toggle_button_get_inconsistent
                    (GTK_TOGGLE_BUTTON (priv->chk_underline)));
            break;

        case PROP_STRIKETHROUGH:
            g_value_set_boolean(value, 
                    gtk_toggle_button_get_active
                    (GTK_TOGGLE_BUTTON (priv->chk_strikethrough)));
            break;

        case PROP_STRIKETHROUGH_SET:
            g_value_set_boolean(value,
                    ! gtk_toggle_button_get_inconsistent
                    (GTK_TOGGLE_BUTTON (priv->chk_strikethrough)));
            break;

        case PROP_POSITION:
            i = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->cbx_positioning));
            if(i == 1) /* super */
                g_value_set_int (value, 1);
            else if(i == 2)/* sub */
                g_value_set_int (value, -1);
            else
                g_value_set_int (value, 0);
            break;

        case PROP_FONT_SCALING:
            g_value_set_double (value, priv->font_scaling);
            break;

        case PROP_POSITION_SET:
            i = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->cbx_positioning));
            if(i >= 0 && i < 3)
                g_value_set_boolean (value, TRUE);
            else
                g_value_set_boolean (value, FALSE);
            break;

        case PROP_PREVIEW_TEXT:
            g_value_set_string (value,
                    hildon_font_selection_dialog_get_preview_text (HILDON_FONT_SELECTION_DIALOG (object)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void 
hildon_font_selection_dialog_set_property       (GObject *object,
                                                 guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec)
{
    gint i, size;
    const gchar *family;
    gboolean b;
    GdkColor *color = NULL;
    GdkColor black;

    HildonFontSelectionDialogPrivate *priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (object);
    g_assert (priv);

    black.red = black.green = black.blue = 0;

    switch (prop_id)
    {
        case PROP_FAMILY:
            family = g_value_get_string (value);
            g_return_if_fail (family != NULL);
            for(i = 0; i < priv->n_families; i++)
            {
                if (strcmp (family, pango_font_family_get_name (priv->families[i]))
                        == 0)
                {
                    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->cbx_font_type), i);
                    break;
                }
            }
            break;

        case PROP_FAMILY_SET:
            b = g_value_get_boolean (value);
            if(!b)
                gtk_combo_box_set_active (GTK_COMBO_BOX (priv->cbx_font_type), -1);
            break;

        case PROP_SIZE:
            size = g_value_get_int (value);
            for(i = 0; i < G_N_ELEMENTS (font_sizes); i++)
            {
                if(size == font_sizes[i])
                {
                    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->cbx_font_size), i);
                    break;
                }
            }
            break;

        case PROP_SIZE_SET:
            b = g_value_get_boolean (value);
            if(!b)
                gtk_combo_box_set_active (GTK_COMBO_BOX (priv->cbx_font_size), -1);
            break;

        case PROP_COLOR:
            color = (GdkColor *) g_value_get_boxed (value);
            if(color != NULL)
                hildon_color_button_set_color (HILDON_COLOR_BUTTON
                        (priv->font_color_button),
                        color);
            else
                hildon_color_button_set_color (HILDON_COLOR_BUTTON
                        (priv->font_color_button),
                        &black);
            break;

        case PROP_COLOR_SET:
            priv->color_set = g_value_get_boolean (value);
            if(! priv->color_set)
            {
                /* set color to black, but block our signal handler */
                g_signal_handler_block ((gpointer) priv->font_color_button,
                        priv->color_modified_signal_handler);

                hildon_color_button_set_color (HILDON_COLOR_BUTTON
                        (priv->font_color_button), 
                        &black);

                g_signal_handler_unblock ((gpointer) priv->font_color_button,
                        priv->color_modified_signal_handler);
            }
            break;

        case PROP_BOLD:
            /* this call will make sure that we dont get extra clicked signal */
            gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON(priv->chk_bold), FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->chk_bold), g_value_get_boolean (value));
            break;

        case PROP_BOLD_SET:
            gtk_toggle_button_set_inconsistent (GTK_TOGGLE_BUTTON (priv->chk_bold),! g_value_get_boolean(value));
            break;

        case PROP_ITALIC:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(priv->chk_italic),
                    FALSE);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->chk_italic),
                    g_value_get_boolean(value));
            break;

        case PROP_ITALIC_SET:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(priv->chk_italic),
                    !g_value_get_boolean(value));
            break;

        case PROP_UNDERLINE:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON
                    (priv->chk_underline),
                    FALSE);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->chk_underline),
                    g_value_get_boolean(value));
            break;

        case PROP_UNDERLINE_SET:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(priv->chk_underline),
                    !g_value_get_boolean(value));
            break;

        case PROP_STRIKETHROUGH:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON
                    (priv->chk_strikethrough),
                    FALSE);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->chk_strikethrough),
                    g_value_get_boolean(value));
            break;

        case PROP_STRIKETHROUGH_SET:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON
                    (priv->chk_strikethrough),
                    !g_value_get_boolean(value));
            break;

        case PROP_POSITION:
            i = g_value_get_int(value);
            if( i == 1 )
                gtk_combo_box_set_active(GTK_COMBO_BOX(priv->cbx_positioning), 1);
            else if(i == -1)
                gtk_combo_box_set_active(GTK_COMBO_BOX(priv->cbx_positioning), 2);
            else
                gtk_combo_box_set_active(GTK_COMBO_BOX(priv->cbx_positioning), 0);
            break;

        case PROP_FONT_SCALING:
            priv->font_scaling = g_value_get_double(value);
            break;

        case PROP_POSITION_SET:
            b = g_value_get_boolean(value);
            if(!b)
                gtk_combo_box_set_active(GTK_COMBO_BOX(priv->cbx_positioning), -1);
            break;

        case PROP_PREVIEW_TEXT:
            hildon_font_selection_dialog_set_preview_text(
                    HILDON_FONT_SELECTION_DIALOG(object),
                    g_value_get_string(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
hildon_font_selection_dialog_class_init         (HildonFontSelectionDialogClass *klass)
{
    GObjectClass *gobject_class;

    parent_class = g_type_class_peek_parent (klass);
    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize         = hildon_font_selection_dialog_finalize;
    gobject_class->get_property     = hildon_font_selection_dialog_get_property;
    gobject_class->set_property     = hildon_font_selection_dialog_set_property;

    /* Install properties to the class */

    /**
     * HildonFontSelectionDialog:family:
     *
     * Font family used.
     */
    g_object_class_install_property (gobject_class, PROP_FAMILY,
            g_param_spec_string ("family",
                "Font family", "String defines"
                " the font family", "Sans",
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:family-set:
     *
     * Is font family set or inconsistent.
     */
    g_object_class_install_property (gobject_class, PROP_FAMILY_SET,
            g_param_spec_boolean ("family-set",
                "family inconsistent state",
                "Whether the family property"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:size:
     *
     * Font size.
     */
    g_object_class_install_property (gobject_class, PROP_SIZE,
            g_param_spec_int ("size",
                "Font size",
                "Font size in Pt",
                6, 32, 16,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:size-set:
     *
     * Is font size set or inconsistent.
     */
    g_object_class_install_property (gobject_class, PROP_SIZE_SET,
            g_param_spec_boolean ("size-set",
                "size inconsistent state",
                "Whether the size property"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:color:
     *
     * GdkColor for the text.
     */
    g_object_class_install_property (gobject_class, PROP_COLOR,
            g_param_spec_boxed ("color",
                "text color",
                "gdk color for the text",
                GDK_TYPE_COLOR,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:color-set:
     *
     * Is font color set or inconsistent.
     */
    g_object_class_install_property (gobject_class, PROP_COLOR_SET,
            g_param_spec_boolean ("color-set",
                "color inconsistent state",
                "Whether the color property"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:color-set:
     *
     * Is font set as bold.
     */
    g_object_class_install_property (gobject_class, PROP_BOLD,
            g_param_spec_boolean ("bold",
                "text weight",
                "Whether the text is bold",
                FALSE,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:color-set:
     *
     * Is font bold status set or inconsistent.
     */
    g_object_class_install_property (gobject_class, PROP_BOLD_SET,
            g_param_spec_boolean ("bold-set",
                "bold inconsistent state",
                "Whether the bold"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:italic:
     *
     * Is font set as italic.
     */
    g_object_class_install_property (gobject_class, PROP_ITALIC,
            g_param_spec_boolean ("italic",
                "text style",
                "Whether the text is italic",
                FALSE,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:italic-set:
     *
     * Is font italic status set or inconsistent.
     */
    g_object_class_install_property (gobject_class, PROP_ITALIC_SET,
            g_param_spec_boolean ("italic-set",
                "italic inconsistent state",
                "Whether the italic"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:underline:
     *
     * Is the font underlined.
     */
    g_object_class_install_property (gobject_class, PROP_UNDERLINE,
            g_param_spec_boolean ("underline",
                "text underline",
                "Whether the text is underlined",
                FALSE,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:underline:
     *
     * Is font underline status set or inconsistent.
     */
    g_object_class_install_property (gobject_class, PROP_UNDERLINE_SET,
            g_param_spec_boolean ("underline-set",
                "underline inconsistent state",
                "Whether the underline"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:strikethrough:
     *
     * Is the font striken-through.
     */
    g_object_class_install_property (gobject_class, PROP_STRIKETHROUGH,
            g_param_spec_boolean ("strikethrough",
                "strikethroughed text",
                "Whether the text is strikethroughed",
                FALSE,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:strikethrough-set:
     *
     * Is the font strikenthrough status set.
     */
    g_object_class_install_property (gobject_class, PROP_STRIKETHROUGH_SET,
            g_param_spec_boolean ("strikethrough-set",
                "strikethrough inconsistent state",
                "Whether the strikethrough"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:position:
     *
     * The font positioning versus baseline.
     */
    g_object_class_install_property (gobject_class, PROP_POSITION,
            g_param_spec_int ("position",
                "Font position",
                "Font position super or subscript",
                -1, 1, 0,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:position-set:
     *
     * Is the font positioning set.
     */
    g_object_class_install_property (gobject_class, PROP_POSITION_SET,
            g_param_spec_boolean ("position-set",
                "position inconsistent state",
                "Whether the position"
                " is inconsistent", FALSE,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    /**
     * HildonFontSelectionDialog:font-scaling:
     *
     * The font scaling factor applied to the preview dialog.
     */
    g_object_class_install_property (gobject_class, PROP_FONT_SCALING,
            g_param_spec_double ("font-scaling",
                "Font scaling",
                "Font scaling for the preview dialog",
                0, 10, 1,
                G_PARAM_READWRITE));

    /**
     * HildonFontSelectionDialog:preview-text:
     *
     * The text used for the preview dialog.
     *
     * Deprecated: this property is unused since hildon 2.2
     */
    g_object_class_install_property (gobject_class, PROP_PREVIEW_TEXT,
            g_param_spec_string("preview-text",
                "Preview Text", 
                "the text in preview dialog, which does" 
                "not include the reference text",
                "",
                G_PARAM_READWRITE));

    g_type_class_add_private (klass, 
            sizeof (struct _HildonFontSelectionDialogPrivate));
}

static void 
hildon_font_selection_dialog_init               (HildonFontSelectionDialog *fontseldiag)
{
    HildonFontSelectionDialogPrivate *priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fontseldiag);
    /* GtkWidget *preview_button; */

    g_assert (priv);
    priv->notebook = GTK_NOTEBOOK (gtk_notebook_new ());

    hildon_font_selection_dialog_construct_notebook (fontseldiag);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (fontseldiag)->vbox),
            GTK_WIDGET (priv->notebook), TRUE, TRUE, 0);

    /* Add dialog buttons */
    gtk_dialog_add_button (GTK_DIALOG (fontseldiag),
			   HILDON_STOCK_DONE, GTK_RESPONSE_OK);

#if 0
    preview_button = gtk_button_new_with_label (_("ecdg_bd_font_dialog_preview"));
    gtk_box_pack_start (GTK_BOX(GTK_DIALOG (fontseldiag)->action_area),
            preview_button, FALSE, TRUE, 0);

    g_signal_connect_swapped (preview_button, "clicked",
            G_CALLBACK
            (hildon_font_selection_dialog_show_preview),
            fontseldiag);
    gtk_widget_show(preview_button);
#endif

    /*Set default preview text*/
    priv->preview_text = g_strdup (_("ecdg_fi_preview_font_preview_text"));

    gtk_window_set_title (GTK_WINDOW (fontseldiag), _("ecdg_ti_font"));
    /*here is the line to make sure that notebook has the default focus*/
    gtk_container_set_focus_child (GTK_CONTAINER (GTK_DIALOG (fontseldiag)->vbox),
            GTK_WIDGET (priv->notebook));
}

static void 
hildon_font_selection_dialog_construct_notebook (HildonFontSelectionDialog *fontsel)
{
    gint i;
    GtkWidget *vbox_tab[3];
    GtkWidget *font_color_box;
    GtkWidget *caption_control;
    GtkSizeGroup *group;

    HildonFontSelectionDialogPrivate *priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fontsel);
    g_assert (priv);

    for (i = 0; i < 3; i++)
        vbox_tab[i] = gtk_vbox_new (TRUE, 0);

    group = GTK_SIZE_GROUP (gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL));

    /* Build the first page of the GtkNotebook: font style */
    priv->cbx_font_type = gtk_combo_box_new_text ();
    hildon_font_selection_dialog_show_available_fonts (fontsel);
    caption_control = hildon_caption_new (group,
            _("ecdg_fi_font_font"),
            priv->cbx_font_type,
            NULL,
            HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[0]), caption_control, FALSE, FALSE, 0);

    priv->cbx_font_size = gtk_combo_box_new_text ();
    hildon_font_selection_dialog_show_available_sizes (priv);
    caption_control = hildon_caption_new (group,
            _("ecdg_fi_font_size"),
            priv->cbx_font_size,
            NULL,
            HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[0]), caption_control, FALSE, FALSE, 0);

    font_color_box = gtk_hbox_new (FALSE, 0);
    priv->font_color_button = hildon_color_button_new ();
    priv->color_set = FALSE;
    priv->font_scaling = 1.0;
    priv->color_modified_signal_handler = 
        g_signal_connect (G_OBJECT (priv->font_color_button), "notify::color",
                G_CALLBACK (color_modified_cb), (gpointer) priv);

    gtk_box_pack_start (GTK_BOX (font_color_box), priv->font_color_button, FALSE, FALSE, 0);

    caption_control =
        hildon_caption_new (group, _("ecdg_fi_font_colour_selector"),
                font_color_box,
                NULL, HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[0]), caption_control, FALSE, FALSE, 0);

    /* Build the second page of the GtkNotebook: font formatting */ 
    priv->chk_bold = gtk_check_button_new ();
    caption_control = hildon_caption_new (group,
            _("ecdg_fi_font_bold"),
            priv->chk_bold,
            NULL,
            HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[1]), caption_control, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (priv->chk_bold), "clicked", 
            G_CALLBACK(toggle_clicked), NULL);

    priv->chk_italic = gtk_check_button_new ();
    caption_control = hildon_caption_new (group, _("ecdg_fi_font_italic"),
                priv->chk_italic,
                NULL, HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[1]), caption_control, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(priv->chk_italic), "clicked", 
            G_CALLBACK(toggle_clicked), NULL);

    priv->chk_underline = gtk_check_button_new();
    caption_control =
        hildon_caption_new (group, _("ecdg_fi_font_underline"),
                priv->chk_underline, NULL,
                HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[1]), caption_control, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT(priv->chk_underline), "clicked", 
            G_CALLBACK (toggle_clicked), NULL);

    /* Build the third page of the GtkNotebook: other font properties */
    priv->chk_strikethrough = gtk_check_button_new ();
    caption_control = hildon_caption_new(group, _("ecdg_fi_font_strikethrough"),
                priv->chk_strikethrough, NULL,
                HILDON_CAPTION_OPTIONAL);

    gtk_box_pack_start (GTK_BOX (vbox_tab[2]), caption_control, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT(priv->chk_strikethrough), "clicked", 
            G_CALLBACK (toggle_clicked), NULL);

    priv->cbx_positioning = gtk_combo_box_new_text ();
    hildon_font_selection_dialog_show_available_positionings (priv);
    caption_control =
        hildon_caption_new(group, _("ecdg_fi_font_special"),
                priv->cbx_positioning, NULL,
                HILDON_CAPTION_OPTIONAL);

    g_object_unref (group);

    gtk_box_pack_start (GTK_BOX (vbox_tab[2]), caption_control, FALSE, FALSE, 0);

    /* Populate notebook */
    gtk_notebook_insert_page (priv->notebook, vbox_tab[0], NULL, 0);
    gtk_notebook_insert_page (priv->notebook, vbox_tab[1], NULL, 1);
    gtk_notebook_insert_page (priv->notebook, vbox_tab[2], NULL, 2);

    gtk_notebook_set_tab_label_text (priv->notebook, vbox_tab[0],
            _("ecdg_ti_font_dialog_style"));
    gtk_notebook_set_tab_label_text (priv->notebook, vbox_tab[1],
            _("ecdg_ti_font_dialog_format"));
    gtk_notebook_set_tab_label_text (priv->notebook, vbox_tab[2],
            _("ecdg_ti_font_dialog_other"));

    gtk_widget_show_all (GTK_WIDGET (priv->notebook));
}

static void 
color_modified_cb                               (HildonColorButton *button, 
                                                 GParamSpec *pspec, 
                                                 gpointer fsd_priv)
{
    HildonFontSelectionDialogPrivate *priv = (HildonFontSelectionDialogPrivate *) fsd_priv;
    g_assert (priv);

    priv->color_set = TRUE;
}

static void 
hildon_font_selection_dialog_finalize           (GObject *object)
{
    HildonFontSelectionDialogPrivate *priv;
    HildonFontSelectionDialog *fontsel;

    g_assert (HILDON_IS_FONT_SELECTION_DIALOG (object));
    fontsel = HILDON_FONT_SELECTION_DIALOG (object);

    priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fontsel);
    g_assert (priv);

    if (priv->preview_text != NULL) {
        g_free (priv->preview_text);
        priv->preview_text = NULL;
    }

    if (priv->families != NULL) {
        g_free (priv->families);
        priv->families = NULL;
    }

    if (G_OBJECT_CLASS (parent_class)->finalize)
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static int 
cmp_families                                    (const void *a, 
                                                 const void *b)
{
    const char *a_name =
        pango_font_family_get_name (* (PangoFontFamily **) a);

    const char *b_name =
        pango_font_family_get_name (* (PangoFontFamily **) b);

    return g_utf8_collate (a_name, b_name);
}

#if 0
/* Exits the preview dialog with GTK_RESPONSE_CANCEL if Esc key
 * was pressed 
 * FIXME This should be handled automatically */
static gboolean
hildon_font_selection_dialog_preview_key_press  (GtkWidget *widget,
                                                 GdkEventKey *event,
                                                 gpointer unused)
{
    g_assert (widget);
    g_assert (event);

    if (event->keyval == GDK_Escape)
    {
        gtk_dialog_response (GTK_DIALOG (widget), GTK_RESPONSE_CANCEL);
        return TRUE;
    }

    return FALSE;
}

static void
add_preview_text_attr                           (PangoAttrList *list, 
                                                 PangoAttribute *attr, 
                                                 guint start, 
                                                 guint len)
{
    attr->start_index = start;
    attr->end_index = start + len;
    pango_attr_list_insert (list, attr);
}

static PangoAttrList*
hildon_font_selection_dialog_create_attrlist    (HildonFontSelectionDialog *fontsel, 
                                                 guint start_index, 
                                                 guint len)
{
    PangoAttrList *list;
    PangoAttribute *attr;
    gint size, position;
    gboolean family_set, size_set, color_set, bold, bold_set,
             italic, italic_set, underline, underline_set,
             strikethrough, strikethrough_set, position_set;
    GdkColor *color = NULL;
    gchar *family = NULL;
    gdouble font_scaling = 1.0;

    list = pango_attr_list_new ();

    g_object_get (G_OBJECT (fontsel),
            "family", &family, "family-set", &family_set,
            "size", &size, "size-set", &size_set,
            "color", &color, "color-set", &color_set,
            "bold", &bold, "bold-set", &bold_set,
            "italic", &italic, "italic-set", &italic_set,
            "underline", &underline, "underline-set", &underline_set,
            "strikethrough", &strikethrough, "strikethrough-set", 
            &strikethrough_set, "position", &position, 
            "position-set", &position_set, 
            "font-scaling", &font_scaling,
            NULL);

    /* family */
    if (family_set)
    {
        attr = pango_attr_family_new (family);
        add_preview_text_attr (list, attr, start_index, len);
    }
    g_free (family);

    /* size */
    if (size_set)
    {
        attr = pango_attr_size_new (size * PANGO_SCALE);
        add_preview_text_attr (list, attr, start_index, len);
    }

    /*color*/
    if (color_set)
    {
        attr = pango_attr_foreground_new (color->red, color->green, color->blue);
        add_preview_text_attr (list, attr, start_index, len);
    }

    if (color != NULL)
        gdk_color_free (color);

    /*weight*/
    if (bold_set)
    {
        if (bold)
            attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
        else
            attr = pango_attr_weight_new (PANGO_WEIGHT_NORMAL);

        add_preview_text_attr(list, attr, start_index, len);
    }

    /* style */
    if (italic_set)
    {
        if (italic)
            attr = pango_attr_style_new (PANGO_STYLE_ITALIC);
        else
            attr = pango_attr_style_new (PANGO_STYLE_NORMAL);

        add_preview_text_attr(list, attr, start_index, len);
    }

    /* underline */
    if (underline_set)
    {
        if (underline)
            attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
        else
            attr = pango_attr_underline_new (PANGO_UNDERLINE_NONE);

        add_preview_text_attr(list, attr, start_index, len);
    }

    /* strikethrough */
    if (strikethrough_set)
    {
        if (strikethrough)
            attr = pango_attr_strikethrough_new (TRUE);
        else
            attr = pango_attr_strikethrough_new (FALSE);

        add_preview_text_attr(list, attr, start_index, len);
    }

    /* position */
    if (position_set)
    {
        switch (position)
        {
            case 1: /*super*/
                attr = pango_attr_rise_new (SUPERSCRIPT_RISE);
                break;
            case -1: /*sub*/
                attr = pango_attr_rise_new (SUBSCRIPT_LOW);
                break;
            default: /*normal*/
                attr = pango_attr_rise_new (0);
                break;
        }

        add_preview_text_attr (list, attr, start_index, len);
    }

    /* font scaling for preview */
    if (font_scaling)
    {
        attr = pango_attr_scale_new(font_scaling);
        add_preview_text_attr(list, attr, 0, len + start_index);
    }

    return list;
}

static void
hildon_font_selection_dialog_show_preview       (HildonFontSelectionDialog *fontsel)
{
    HildonFontSelectionDialogPrivate *priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fontsel);
    gint size;
    gboolean family_set, size_set;
    PangoAttribute *attr;
    PangoAttrList *list;
    GtkWidget *preview_dialog;
    GtkWidget *preview_label;
    gchar *str = NULL;
    gboolean position_set = FALSE;
    gint position = 0;
    gboolean show_ref = FALSE;

    g_assert (priv);

    g_object_get (G_OBJECT (fontsel), "position-set", &position_set, NULL);

    if (position_set) {
        g_object_get (G_OBJECT (fontsel), "position", &position, NULL);
        if (position == 1 || position == -1)
            show_ref = TRUE;
    }

    /* preview dialog init */
    preview_dialog =
        gtk_dialog_new_with_buttons (_("ecdg_ti_preview_font"), NULL,
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                HILDON_STOCK_DONE, GTK_RESPONSE_ACCEPT,
                NULL);

    str = (show_ref) ? g_strconcat (_("ecdg_fi_preview_font_preview_reference"), priv->preview_text, 0) :
        g_strdup (priv->preview_text);

    preview_label = gtk_label_new (str);
    gtk_label_set_line_wrap (GTK_LABEL(preview_label), TRUE);

    if (str) 
        g_free (str);

    str = NULL;

    /* set keypress handler (ESC hardkey) */
    g_signal_connect (G_OBJECT (preview_dialog), "key-press-event",
            G_CALLBACK(hildon_font_selection_dialog_preview_key_press),
            NULL);

    /* Set the font */
    list = (show_ref) ? hildon_font_selection_dialog_create_attrlist (fontsel, 
            strlen (_("ecdg_fi_preview_font_preview_reference")),
            strlen (priv->preview_text)) :
        hildon_font_selection_dialog_create_attrlist (fontsel, 0, strlen(priv->preview_text));

    g_object_get (G_OBJECT (fontsel), "family", &str, "family-set",
            &family_set, "size", &size, "size-set", &size_set,
            NULL);

    /* A smallish hack to add scrollbar when font size is really big */

    if (size_set && size > 24) {
        GtkScrolledWindow *scrolled = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
        gtk_scrolled_window_set_policy (scrolled, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_add_with_viewport (scrolled, GTK_WIDGET (preview_label));
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(preview_dialog)->vbox), GTK_WIDGET (scrolled));
        gtk_widget_set_size_request (GTK_WIDGET (scrolled), -1, 400);
    } else 
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(preview_dialog)->vbox), GTK_WIDGET (preview_label));

    /* make reference text to have the same fontface and size */
    if (family_set)
    {
        attr = pango_attr_family_new (str);
        add_preview_text_attr (list, attr, 0, strlen (_("ecdg_fi_preview_font_preview_reference")));
    }
    if (str != NULL)
        g_free (str);

    str = NULL;

    /* size */
    if (size_set)
    {
        attr = pango_attr_size_new (size * PANGO_SCALE);
        add_preview_text_attr (list, attr, 0, strlen (_("ecdg_fi_preview_font_preview_reference")));
    }

    gtk_label_set_attributes (GTK_LABEL (preview_label), list);
    pango_attr_list_unref (list);

    /*And show the dialog*/
    gtk_window_set_transient_for (GTK_WINDOW (preview_dialog), 
            GTK_WINDOW (fontsel));

    gtk_widget_show_all (preview_dialog);
    gtk_dialog_set_default_response (GTK_DIALOG (preview_dialog), GTK_RESPONSE_OK);
    
    GtkBox *action_area = (GtkBox *) GTK_DIALOG (preview_dialog)->action_area;
    GtkWidget *button = ((GtkBoxChild *) ((GSList *) action_area->children)->data)->widget;
    gtk_widget_grab_focus (button);

    gtk_dialog_run (GTK_DIALOG (preview_dialog));
    gtk_widget_destroy (preview_dialog);
}
#endif

static gboolean 
is_internal_font                                (const gchar * name)
{
    /* FIXME Extremally BAD BAD BAD way of doing things */

    return strcmp(name, "DeviceSymbols") == 0
        || strcmp(name, "Nokia Smiley" ) == 0
        || strcmp(name, "NewCourier" ) == 0
        || strcmp(name, "NewTimes" ) == 0
        || strcmp(name, "SwissA" ) == 0
        || strcmp(name, "Nokia Sans"   ) == 0
        || strcmp(name, "Nokia Sans Cn") == 0;
}

static void 
filter_out_internal_fonts                       (PangoFontFamily **families, 
                                                 int *n_families)
{
    int i;
    int n; /* counts valid fonts */
    const gchar * name = NULL;

    for(i = 0, n = 0; i < * n_families; i++){

        name = pango_font_family_get_name (families[i]);

        if(!is_internal_font(name))
        {

            if (i!=n){ /* there are filtered out families */
                families[n] = families[i]; /* shift the current family */
            }

            n++; /* count one more valid */
        }
    } /* foreach font family */

    *n_families = n;  
}

static void
hildon_font_selection_dialog_show_available_fonts (HildonFontSelectionDialog *fontsel)

{
    gint i;

    HildonFontSelectionDialogPrivate *priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fontsel);
    g_assert (priv);

    pango_context_list_families (gtk_widget_get_pango_context
            (GTK_WIDGET (fontsel)), &priv->families,
            &priv->n_families);

    filter_out_internal_fonts (priv->families, &priv->n_families);

    qsort (priv->families, priv->n_families, sizeof(PangoFontFamily *), cmp_families);

    for (i = 0; i < priv->n_families; i++) 
    {
        const gchar *name = pango_font_family_get_name (priv->families[i]);
        gtk_combo_box_append_text (GTK_COMBO_BOX (priv->cbx_font_type), name);
    }
}

static void
hildon_font_selection_dialog_show_available_positionings (HildonFontSelectionDialogPrivate *priv)
{
    gtk_combo_box_append_text (GTK_COMBO_BOX (priv->cbx_positioning), _("ecdg_va_font_printpos_1"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (priv->cbx_positioning), _("ecdg_va_font_printpos_2"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (priv->cbx_positioning), _("ecdg_va_font_printpos_3"));
}

/* Loads the sizes from a pre-allocated table */
static void
hildon_font_selection_dialog_show_available_sizes (HildonFontSelectionDialogPrivate *priv)
{
    gchar *size_str;
    gint i;

    g_assert (priv);

    for (i = 0; i < G_N_ELEMENTS (font_sizes); i++) 
    {
        size_str = g_strdup_printf ("%i%s",
                font_sizes[i],
                _("ecdg_va_font_size_trailer"));

        gtk_combo_box_append_text (GTK_COMBO_BOX (priv->cbx_font_size), size_str);
        g_free (size_str);
    }
}

static void
toggle_clicked                                  (GtkButton *button, 
                                                 gpointer unused)
{
    GtkToggleButton *t_b = GTK_TOGGLE_BUTTON (button);

    /* we have to remove the inconsistent state ourselves */
    if (gtk_toggle_button_get_inconsistent (t_b))
    {
        gtk_toggle_button_set_inconsistent (t_b, FALSE);
        gtk_toggle_button_set_active (t_b, FALSE);
    }
}

/**
 * hildon_font_selection_dialog_new:
 * @parent: the parent window
 * @title: the title of font selection dialog
 *
 * If NULL is passed for title, then default title
 * "Font" will be used.
 *
 * Returns: a new #HildonFontSelectionDialog
 */
GtkWidget*
hildon_font_selection_dialog_new                (GtkWindow *parent,
                                                 const gchar *title)
{
    HildonFontSelectionDialog *fontseldiag;

    fontseldiag = g_object_new (HILDON_TYPE_FONT_SELECTION_DIALOG,
            "has-separator", FALSE, NULL);

    if (title)
        gtk_window_set_title (GTK_WINDOW (fontseldiag), title);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (fontseldiag), parent);

    return GTK_WIDGET (fontseldiag);
}

/**
 * hildon_font_selection_dialog_get_preview_text:
 * @fsd: the font selection dialog
 *
 * Gets the text in preview dialog, which does not include the
 * reference text. The returned string must be freed by the user.
 * Please note that since hildon 2.2, the preview has been discontinued,
 * so this setting has no effect.
 *
 * Returns: a string pointer
 */
gchar*
hildon_font_selection_dialog_get_preview_text   (HildonFontSelectionDialog * fsd)
{
    /* FIXME Return a const pointer? */
    HildonFontSelectionDialogPrivate *priv;

    g_return_val_if_fail (HILDON_IS_FONT_SELECTION_DIALOG (fsd), NULL);
        
    priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fsd);
    g_assert (priv);

    return g_strdup (priv->preview_text);
}

/**
 * hildon_font_selection_dialog_set_preview_text:
 * @fsd: the font selection dialog
 * @text: the text to be displayed in the preview dialog
 *
 * The default preview text is
 * "The quick brown fox jumped over the lazy dogs". Please note that since
 * hildon 2.2, the preview has been discontinued, so this setting has no effect.
 *
 */
void
hildon_font_selection_dialog_set_preview_text   (HildonFontSelectionDialog *fsd, 
                                                 const gchar * text)
{
    HildonFontSelectionDialogPrivate *priv = NULL;

    g_return_if_fail (HILDON_IS_FONT_SELECTION_DIALOG (fsd));
    g_return_if_fail (text);

    priv = HILDON_FONT_SELECTION_DIALOG_GET_PRIVATE (fsd);
    g_assert (priv);

    g_free (priv->preview_text);
    priv->preview_text = g_strdup (text);
    g_object_notify (G_OBJECT (fsd), "preview-text");
}

