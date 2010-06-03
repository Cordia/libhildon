/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2006 Nokia Corporation, all rights reserved.
 *
 * Contact: Rodrigo Novo <rodrigo.novo@nokia.com>
 *   Fixes: Michael Dominic Kostrzewa <michael.kostrzewa@nokia.com>
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
 * SECTION:hildon-wizard-dialog
 * @short_description: A widget to create a guided installation
 * process wizard.
 *
 * #HildonWizardDialog is a widget to create a guided installation
 * process. The dialog has three standard buttons, previous, next,
 * finish, and contains several pages.
 *
 * Response buttons are dimmed/undimmed automatically. The notebook
 * widget provided by users contains the actual wizard pages.
 * 
 * Usage of the API is very simple, it has only one function to create it
 * and the rest of it is handled by developers notebook.
 * Also, the response is returned, either cancel or finish.
 * Next and previous buttons are handled by the wizard dialog it self, by
 * switching the page either forward or backward in the notebook.
 *
 * It is possible to determinate whether users can go to the next page
 * by setting a #HildonWizardDialogPageFunc function with
 * hildon_wizard_dialog_set_forward_page_func()
 */

#ifdef                                          HAVE_CONFIG_H
#include                                        <config.h>
#endif

#include                                        <libintl.h>

#include                                        "hildon-wizard-dialog.h"
#include                                        "hildon-defines.h"
#include                                        "hildon-wizard-dialog-private.h"
#include                                        "hildon-stock.h"

#define                                         _(String) dgettext("hildon-libs", String)

static GtkDialogClass*                          parent_class;

static void 
hildon_wizard_dialog_class_init                 (HildonWizardDialogClass *wizard_dialog_class);

static void 
hildon_wizard_dialog_init                       (HildonWizardDialog *wizard_dialog);

static void 
create_title                                    (HildonWizardDialog *wizard_dialog);

static void 
hildon_wizard_dialog_set_property               (GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);

static void 
hildon_wizard_dialog_get_property               (GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec);

static void 
finalize                                        (GObject *object);

static void
destroy                                         (GtkObject *object);

static void 
response                                        (HildonWizardDialog *wizard, 
                                                 gint response_id,
                                                 gpointer unused);

static void 
make_buttons_sensitive                          (HildonWizardDialog *wizard_dialog,
                                                 gboolean previous,
                                                 gboolean finish,
                                                 gboolean next);

enum 
{
    PROP_0,
    PROP_NAME,
    PROP_NOTEBOOK,
    PROP_AUTOTITLE
};

/**
 * hildon_wizard_dialog_get_type:
 *
 * Initializes and returns the type of a hildon wizard dialog.
 *
 * Returns: GType of #HildonWzardDialog
 */
GType G_GNUC_CONST
hildon_wizard_dialog_get_type                   (void)
{
    static GType wizard_dialog_type = 0;

    if (! wizard_dialog_type) {

        static const GTypeInfo wizard_dialog_info = {
            sizeof (HildonWizardDialogClass),
            NULL,       /* base_init      */
            NULL,       /* base_finalize  */
            (GClassInitFunc) hildon_wizard_dialog_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data     */
            sizeof (HildonWizardDialog),
            0,          /* n_preallocs    */
            (GInstanceInitFunc) hildon_wizard_dialog_init,
        };

        wizard_dialog_type = g_type_register_static (GTK_TYPE_DIALOG,
                "HildonWizardDialog",
                &wizard_dialog_info,
                0);
    }

    return wizard_dialog_type;
}

static void
hildon_wizard_dialog_class_init                 (HildonWizardDialogClass *wizard_dialog_class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (wizard_dialog_class);
    GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (wizard_dialog_class);
    parent_class = g_type_class_peek_parent (wizard_dialog_class);

    g_type_class_add_private (wizard_dialog_class, sizeof (HildonWizardDialogPrivate));

    /* Override virtual methods */
    object_class->set_property = hildon_wizard_dialog_set_property;
    object_class->get_property = hildon_wizard_dialog_get_property;
    object_class->finalize     = finalize;
    gtk_object_class->destroy  = destroy;

    /**
     * HildonWizardDialog:wizard-name:
     *
     * The name of the wizard.
     */
    g_object_class_install_property (object_class, PROP_NAME,
            g_param_spec_string 
            ("wizard-name",
             "Wizard Name",
             "The name of the HildonWizardDialog",
             NULL,
             G_PARAM_READWRITE));

    /**
     * HildonWizardDialog:wizard-notebook:
     *
     * The notebook object, which is used by the HildonWizardDialog.
     */
    g_object_class_install_property (object_class, PROP_NOTEBOOK,
            g_param_spec_object 
            ("wizard-notebook",
             "Wizard Notebook",
             "GtkNotebook object to be used in the "
             "HildonWizardDialog",
             GTK_TYPE_NOTEBOOK, G_PARAM_READWRITE));

    /**
     * HildonWizardDialog:autotitle
     *
     * If the wizard should automatically try to change the window title when changing steps. 
     * Set to FALSE if you'd like to override the default behaviour. 
     *
     * Since: 0.14.5 
     */
    g_object_class_install_property (object_class, PROP_AUTOTITLE,
            g_param_spec_boolean 
            ("autotitle",
             "AutoTitle",
             "If the wizard should autotitle itself",
             TRUE, 
             G_PARAM_READWRITE));
}

static void 
finalize                                        (GObject *object)
{
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (object);

    g_assert (priv);

    if (priv->wizard_name != NULL)
        g_free (priv->wizard_name);

    if (G_OBJECT_CLASS (parent_class)->finalize)
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
destroy                                         (GtkObject *object)
{
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (object);

    g_assert (priv);

    if (priv->forward_function)
    {
      if (priv->forward_function_data &&
	  priv->forward_data_destroy)
	priv->forward_data_destroy (priv->forward_function_data);

      priv->forward_function = NULL;
      priv->forward_function_data = NULL;
      priv->forward_data_destroy = NULL;
    }
}

/* Disable or enable the Previous, Next and Finish buttons */
static void
make_buttons_sensitive                          (HildonWizardDialog *wizard_dialog,
                                                 gboolean previous,
                                                 gboolean finish,
                                                 gboolean next)
{
    gtk_dialog_set_response_sensitive (GTK_DIALOG (wizard_dialog),
            HILDON_WIZARD_DIALOG_PREVIOUS,
            previous);

    gtk_dialog_set_response_sensitive (GTK_DIALOG (wizard_dialog),
            HILDON_WIZARD_DIALOG_FINISH,
            finish);

    gtk_dialog_set_response_sensitive (GTK_DIALOG (wizard_dialog),
            HILDON_WIZARD_DIALOG_NEXT,
            next);
}

static void 
hildon_wizard_dialog_init                       (HildonWizardDialog *wizard_dialog)
{
    /* Initialize private structure for faster member access */
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (wizard_dialog);
    g_assert (priv);

    GtkDialog *dialog = GTK_DIALOG (wizard_dialog);

    /* Init internal widgets */
    gtk_dialog_set_has_separator (dialog, FALSE);

    /* Default values for user provided properties */
    priv->notebook = NULL;
    priv->wizard_name = NULL;
    priv->autotitle = TRUE;

    priv->forward_function = NULL;
    priv->forward_function_data = NULL;
    priv->forward_data_destroy = NULL;

    /* Add response buttons: finish, previous, next */
    gtk_dialog_add_button (dialog, HILDON_STOCK_FINISH, HILDON_WIZARD_DIALOG_FINISH);
    gtk_dialog_add_button (dialog, HILDON_STOCK_PREVIOUS, HILDON_WIZARD_DIALOG_PREVIOUS);
    gtk_dialog_add_button (dialog, HILDON_STOCK_NEXT, HILDON_WIZARD_DIALOG_NEXT);

    /* Set initial button states: previous and finish buttons are disabled */
    make_buttons_sensitive (wizard_dialog, FALSE, FALSE, TRUE);

    /* Show all the internal widgets */
    gtk_widget_show_all (GTK_WIDGET (dialog->vbox));

    /* connect to dialog's response signal */
    g_signal_connect (G_OBJECT (dialog), "response",
            G_CALLBACK (response), NULL);
}

static void
hildon_wizard_dialog_set_property               (GObject *object, 
                                                 guint property_id,
                                                 const GValue *value, 
                                                 GParamSpec *pspec)
{
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (object);
    GtkDialog *dialog = GTK_DIALOG (object);

    g_assert (priv);

    switch (property_id) {

        case PROP_AUTOTITLE:

            priv->autotitle = g_value_get_boolean (value);

            if (priv->autotitle && 
                    priv->wizard_name && 
                    priv->notebook) 
                create_title (HILDON_WIZARD_DIALOG (object));
            else if (priv->wizard_name)
                gtk_window_set_title (GTK_WINDOW (object), priv->wizard_name);
            break;

        case PROP_NAME: 

            /* Set new wizard name. This name will appear in titlebar */
            if (priv->wizard_name)
                g_free (priv->wizard_name);

            gchar *str = (gchar *) g_value_get_string (value);
            g_return_if_fail (str != NULL);

            priv->wizard_name = g_strdup (str);

            /* We need notebook in order to create title, since page information
               is used in title generation */

            if (priv->notebook && priv->autotitle)
                create_title (HILDON_WIZARD_DIALOG (object));
            break;

        case PROP_NOTEBOOK: {

            GtkNotebook *book = GTK_NOTEBOOK (g_value_get_object (value));
            g_return_if_fail (book != NULL);

            priv->notebook = book;

            /* Set the default properties for the notebook (disable tabs,
             * and remove borders) to make it look like a nice wizard widget */
            gtk_notebook_set_show_tabs (priv->notebook, FALSE);
            gtk_notebook_set_show_border (priv->notebook, FALSE);
            gtk_box_pack_start_defaults (GTK_BOX (dialog->vbox), GTK_WIDGET (priv->notebook));

            /* Show the notebook so that a gtk_widget_show on the dialog is
             * all that is required to display the dialog correctly */
            gtk_widget_show (GTK_WIDGET (priv->notebook));

            /* Update dialog title to reflect current page stats etc */        
            if (priv->wizard_name && priv->autotitle)
                create_title (HILDON_WIZARD_DIALOG (object));

        } break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
hildon_wizard_dialog_get_property               (GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (object);
    g_assert (priv);

    switch (property_id) {

        case PROP_NAME:
            g_value_set_string (value, priv->wizard_name);
            break;

        case PROP_NOTEBOOK:
            g_value_set_object (value, priv->notebook);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

/* Creates the title of the dialog taking into account the current 
 * page of the notebook. */
static void
create_title                                    (HildonWizardDialog *wizard_dialog)
{
    gint pages, current;
    gchar *str = NULL;
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (wizard_dialog);
    GtkNotebook *notebook;

    g_assert (priv);
    notebook = priv->notebook;

    if (! notebook)
        return;

    /* Get page information, we'll need that when creating title */
    pages = gtk_notebook_get_n_pages (notebook);
    current = gtk_notebook_get_current_page (priv->notebook);
    if (current < 0)
        current = 0;

    /* the welcome title on the initial page */
    if (current == 0) {
        str = g_strdup_printf (_("ecdg_ti_wizard_welcome"),
                priv->wizard_name);
    } else {
        const gchar *steps = gtk_notebook_get_tab_label_text (notebook,
                gtk_notebook_get_nth_page (notebook, current));

        if (steps) {
          str = g_strdup_printf ("%s%s %s", priv->wizard_name, _("ecdg_ti_caption_separator"), steps);
        } else {
          str = g_strdup (priv->wizard_name);
        }
    }

    /* Update the dialog to display the generated title */
    gtk_window_set_title (GTK_WINDOW (wizard_dialog), str);
    g_free (str);
}

/* Response signal handler. This function is needed because GtkDialog's 
 * handler for this signal closes the dialog and we don't want that, we 
 * want to change pages and, dimm certain response buttons. Overriding the 
 * virtual function would not work because that would be called after the 
 * signal handler implemented by GtkDialog.
 * FIXME: There is a much saner way to do that [MDK] */
static void 
response                                        (HildonWizardDialog *wizard_dialog,
                                                 gint response_id,
                                                 gpointer unused)
{
    HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (wizard_dialog);
    GtkNotebook *notebook = priv->notebook;
    gint current = 0;
    gint last = gtk_notebook_get_n_pages (notebook) - 1;
    gboolean is_first, is_last;

    g_assert (priv);

    current = gtk_notebook_current_page (notebook);

    switch (response_id) {

        case HILDON_WIZARD_DIALOG_PREVIOUS:
            --current;
            is_last = (current == last);
            is_first = (current == 0);
            make_buttons_sensitive (wizard_dialog,
                                    !is_first, !is_first, !is_last); 
            gtk_notebook_prev_page (notebook); /* go to previous page */
            break;

        case HILDON_WIZARD_DIALOG_NEXT:

          if (!priv->forward_function ||
                (*priv->forward_function) (priv->notebook, current, priv->forward_function_data)) {
              ++current;
              is_last = (current == last);
              is_first = (current == 0);
              make_buttons_sensitive (wizard_dialog,
                                      !is_first, !is_first, !is_last);
              gtk_notebook_next_page (notebook); /* go to next page */
            }
            break;

        case HILDON_WIZARD_DIALOG_FINISH:
            return;

    }

    current = gtk_notebook_get_current_page (notebook);
    is_last = current == last;
    is_first = current == 0;

    /* Don't let the dialog close */
    g_signal_stop_emission_by_name (wizard_dialog, "response");

    /* New page number may appear in the title, update it */
    if (priv->autotitle) 
        create_title (wizard_dialog);
}

/**
 * hildon_wizard_dialog_new:
 * @parent: a #GtkWindow
 * @wizard_name: the name of dialog
 * @notebook: the notebook to be shown on the dialog
 *
 * Creates a new #HildonWizardDialog.
 *
 * Returns: a new #HildonWizardDialog
 */
GtkWidget*
hildon_wizard_dialog_new                        (GtkWindow *parent,
                                                 const char *wizard_name,
                                                 GtkNotebook *notebook)
{
    GtkWidget *widget;

    g_return_val_if_fail (GTK_IS_NOTEBOOK (notebook), NULL);

    widget = GTK_WIDGET (g_object_new
            (HILDON_TYPE_WIZARD_DIALOG,
             "wizard-name", wizard_name,
             "wizard-notebook", notebook, NULL));

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (widget), parent);

    return widget;
}

/**
 * hildon_wizard_dialog_set_forward_page_func:
 * @wizard_dialog: a #HildonWizardDialog
 * @page_func: the #HildonWizardDialogPageFunc
 * @data: user data for @page_func
 * @destroy: destroy notifier for @data
 *
 * Sets the page forwarding function to be @page_func. This function
 * will be used to determine whether it is possible to go to the next page
 * when the user presses the forward button. Setting @page_func to %NULL
 * wil make the wizard to simply go always to the next page.
 *
 * Since: 2.2
 **/
void
hildon_wizard_dialog_set_forward_page_func      (HildonWizardDialog *wizard_dialog,
                                                 HildonWizardDialogPageFunc page_func,
                                                 gpointer data,
                                                 GDestroyNotify destroy)
{
  g_return_if_fail (HILDON_IS_WIZARD_DIALOG (wizard_dialog));

  HildonWizardDialogPrivate *priv = HILDON_WIZARD_DIALOG_GET_PRIVATE (wizard_dialog);

  if (priv->forward_data_destroy &&
      priv->forward_function_data) {
    (*priv->forward_data_destroy) (priv->forward_function_data);
  }

  priv->forward_function = page_func;
  priv->forward_function_data = data;
  priv->forward_data_destroy = destroy;
}
