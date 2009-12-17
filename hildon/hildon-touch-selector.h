/*
 * This file is a part of hildon
 *
 * Copyright (C) 2005, 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version. or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef                                         __HILDON_TOUCH_SELECTOR_H__
#define                                         __HILDON_TOUCH_SELECTOR_H__

#include                                        <gtk/gtk.h>
#include                                        "hildon-touch-selector-column.h"

G_BEGIN_DECLS

#define                                         HILDON_TYPE_TOUCH_SELECTOR \
                                                (hildon_touch_selector_get_type ())

#define                                         HILDON_TOUCH_SELECTOR(obj) \
                                                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                HILDON_TYPE_TOUCH_SELECTOR, HildonTouchSelector))

#define                                         HILDON_TOUCH_SELECTOR_CLASS(vtable) \
                                                (G_TYPE_CHECK_CLASS_CAST ((vtable), \
                                                HILDON_TYPE_TOUCH_SELECTOR, HildonTouchSelectorClass))

#define                                         HILDON_IS_TOUCH_SELECTOR(obj) \
                                                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                HILDON_TYPE_TOUCH_SELECTOR))

#define                                         HILDON_IS_TOUCH_SELECTOR_CLASS(vtable) \
                                                (G_TYPE_CHECK_CLASS_TYPE ((vtable), HILDON_TYPE_TOUCH_SELECTOR))

#define                                         HILDON_TOUCH_SELECTOR_GET_CLASS(inst) \
                                                (G_TYPE_INSTANCE_GET_CLASS ((inst), \
                                                HILDON_TYPE_TOUCH_SELECTOR, HildonTouchSelectorClass))

typedef struct                                  _HildonTouchSelector HildonTouchSelector;
typedef struct                                  _HildonTouchSelectorClass HildonTouchSelectorClass;
typedef struct                                  _HildonTouchSelectorPrivate HildonTouchSelectorPrivate;

typedef gchar *(*HildonTouchSelectorPrintFunc)  (HildonTouchSelector * selector,
                                                 gpointer user_data);

struct                                          _HildonTouchSelector
{
  GtkVBox parent_instance;

  /*< private > */
  HildonTouchSelectorPrivate *priv;
};

struct                                          _HildonTouchSelectorClass
{
  GtkVBoxClass parent_class;

  /* signals */
  void (*changed)                               (HildonTouchSelector *selector,
                                                 gint column);

  /* virtual methods */
  void (*set_model)                             (HildonTouchSelector *selector,
                                                 gint column,
                                                 GtkTreeModel *model);

  gboolean (*has_multiple_selection)            (HildonTouchSelector *selector);
};

/**
 * HildonTouchSelectorSelectionMode:
 * @HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE: Users can select one item
 * @HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE: Users can select one to many items
 *
 * Describes the selection mode of a #HildonTouchSelector.
 **/
typedef enum
{
  HILDON_TOUCH_SELECTOR_SELECTION_MODE_SINGLE,
  HILDON_TOUCH_SELECTOR_SELECTION_MODE_MULTIPLE
} HildonTouchSelectorSelectionMode;

/* construction */
GType
hildon_touch_selector_get_type                  (void) G_GNUC_CONST;

GtkWidget *
hildon_touch_selector_new                       (void);

/* Simple (and recommended) API for one-text-column selectors. */
GtkWidget *
hildon_touch_selector_new_text                  (void);

void
hildon_touch_selector_append_text               (HildonTouchSelector *selector,
                                                 const gchar         *text);
void
hildon_touch_selector_prepend_text              (HildonTouchSelector *selector,
                                                 const gchar         *text);
void
hildon_touch_selector_insert_text               (HildonTouchSelector *selector,
                                                 gint                 position,
                                                 const gchar         *text);

/* column related  */
HildonTouchSelectorColumn*
hildon_touch_selector_append_text_column        (HildonTouchSelector *selector,
                                                 GtkTreeModel *       model,
                                                 gboolean             center);

HildonTouchSelectorColumn*
hildon_touch_selector_append_column             (HildonTouchSelector *selector,
                                                 GtkTreeModel        *model,
                                                 GtkCellRenderer     *cell_renderer,
                                                 ...);
#ifndef HILDON_DISABLE_DEPRECATED
void
hildon_touch_selector_set_column_attributes     (HildonTouchSelector *selector,
                                                 gint                 num_column,
                                                 GtkCellRenderer     *cell_renderer,
                                                 ...);
#endif

gboolean
hildon_touch_selector_remove_column             (HildonTouchSelector *selector,
                                                 gint                 column);

gint
hildon_touch_selector_get_num_columns           (HildonTouchSelector *selector);

void
hildon_touch_selector_set_column_selection_mode (HildonTouchSelector              *selector,
                                                 HildonTouchSelectorSelectionMode  mode);

HildonTouchSelectorSelectionMode
hildon_touch_selector_get_column_selection_mode (HildonTouchSelector *selector);

HildonTouchSelectorColumn *
hildon_touch_selector_get_column                (HildonTouchSelector *selector,
                                                 gint                 column);

/* get/set active item */
void
hildon_touch_selector_set_active                (HildonTouchSelector *selector,
                                                 gint                 column,
                                                 gint                 index);
gint
hildon_touch_selector_get_active                (HildonTouchSelector *selector,
                                                 gint                 column);

gboolean
hildon_touch_selector_get_selected              (HildonTouchSelector *selector,
                                                 gint                 column,
                                                 GtkTreeIter         *iter);

void
hildon_touch_selector_select_iter               (HildonTouchSelector *selector,
                                                 gint                 column,
                                                 GtkTreeIter         *iter,
                                                 gboolean             scroll_to);

void
hildon_touch_selector_unselect_iter             (HildonTouchSelector *selector,
                                                 gint                 column,
                                                 GtkTreeIter         *iter);

void
hildon_touch_selector_unselect_all              (HildonTouchSelector * selector,
                                                 gint                  column);

GList *
hildon_touch_selector_get_selected_rows         (HildonTouchSelector *selector,
                                                 gint                 column);
/* model  */
void
hildon_touch_selector_set_model                 (HildonTouchSelector *selector,
                                                 gint                 column,
                                                 GtkTreeModel        *model);
GtkTreeModel *
hildon_touch_selector_get_model                 (HildonTouchSelector *selector,
                                                 gint                 column);

/* show the current element selected */
gchar *
hildon_touch_selector_get_current_text          (HildonTouchSelector *selector);

void
hildon_touch_selector_set_print_func            (HildonTouchSelector          *selector,
                                                 HildonTouchSelectorPrintFunc  func);

void
hildon_touch_selector_set_print_func_full       (HildonTouchSelector          *selector,
                                                 HildonTouchSelectorPrintFunc  func,
                                                 gpointer                      user_data,
                                                 GDestroyNotify                destroy_func);

HildonTouchSelectorPrintFunc
hildon_touch_selector_get_print_func            (HildonTouchSelector *selector);

gboolean
hildon_touch_selector_has_multiple_selection    (HildonTouchSelector *selector);

void
hildon_touch_selector_center_on_selected        (HildonTouchSelector *selector);

void
hildon_touch_selector_center_on_index           (HildonTouchSelector *selector,
                                                 gint column,
                                                 gint index);
void
hildon_touch_selector_optimal_size_request      (HildonTouchSelector *selector,
                                                 GtkRequisition *requisition);

HildonUIMode
hildon_touch_selector_get_hildon_ui_mode        (HildonTouchSelector *selector);

gboolean
hildon_touch_selector_set_hildon_ui_mode        (HildonTouchSelector *selector,
                                                 HildonUIMode         mode);
GtkTreePath*
hildon_touch_selector_get_last_activated_row    (HildonTouchSelector *selector,
                                                 gint                 column);
void
hildon_touch_selector_set_live_search           (HildonTouchSelector *selector,
                                                 gboolean live_search);

gboolean
hildon_touch_selector_get_live_search           (HildonTouchSelector *selector);

G_END_DECLS

#endif /* __HILDON_TOUCH_SELECTOR_H__ */
