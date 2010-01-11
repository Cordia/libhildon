MAINTAINERCLEANFILES			= Makefile.in

if BUILD_EXAMPLES
EXAMPLES				= hildon-window-example				\
					  hildon-banner-example				\
					  hildon-banner-dnd-example			\
					  hildon-banner-long-example			\
					  hildon-caption-example			\
					  hildon-note-example				\
					  hildon-icon-sizes-example			\
					  hildon-window-menu-example			\
					  hildon-window-cmn-menu-example		\
					  hildon-toolbar-example			\
					  hildon-lookup-example				\
					  hildon-scrolled-window-example		\
					  hildon-finger-example				\
					  hildon-find-toolbar-example			\
					  hildon-edit-toolbar-example			\
					  hildon-wizard-dialog-example			\
					  hildon-live-search-example			\
					  hildon-live-search-grid-example		\
					  hildon-pannable-area-tree-view-example	\
					  hildon-pannable-area-tuning-example		\
					  hildon-pannable-area-scroll-jump-example	\
					  hildon-pannable-area-buttons-scroll-example	\
					  hildon-pannable-area-gesture-signals-example	\
					  hildon-pannable-area-initial-hint-example	\
					  hildon-logical-color-example			\
					  hildon-app-menu-example			\
					  hildon-stackable-window-example		\
					  hildon-entry-example				\
					  hildon-text-view-example			\
					  hildon-button-example				\
					  hildon-dialog-example				\
					  hildon-date-button-example			\
					  hildon-picker-button-example			\
					  hildon-picker-button-multicolumn-example	\
					  hildon-time-button-example			\
					  hildon-check-button-example			\
					  hildon-touch-selector-example			\
					  hildon-touch-selector-normal-mode-example	\
					  hildon-touch-selector-multi-cells-example	\
					  hildon-touch-selector-entry-example           \
					  hildon-progress-indicator-example		\
					  hildon-remote-texture-example			\
					  hildon-gtk-window-take-screenshot-sync
if USE_MAEMO_GTK
MAEMO_GTK_EXAMPLES                      = hildon-pannable-area-touch-list-example	\
					  hildon-pannable-area-touch-grid-example
else
MAEMO_GTK_EXAMPLES                      =
endif

DEPRECATED_EXAMPLES			= hildon-weekday-picker-example			\
					  hildon-sort-dialog-example			\
					  hildon-controlbar-example			\
					  hildon-time-picker-example			\
					  hildon-color-chooser-example			\
					  hildon-color-button-example			\
					  hildon-login-dialog-example			\
					  hildon-time-editor-example			\
					  hildon-get-password-dialog-example		\
					  hildon-set-password-dialog-example		\
					  hildon-vvolumebar-example			\
					  hildon-code-dialog-example			\
					  hildon-hvolumebar-example			\
					  hildon-hvolumebar-insensitive-example		\
					  hildon-hvolumebar-insensitive-map-example	\
					  hildon-number-editor-example			\
					  hildon-color-pop-example			\
					  hildon-thumb-scrollbar-example		\
					  hildon-date-editor-example			\
					  hildon-bread-crumb-trail-example		\
					  hildon-seekbar-example			\
					  hildon-toolbar-seekbar-example		\
					  hildon-hvolumebar-timer-example		\
					  hildon-insensitive-example


if HILDON_DISABLE_DEPRECATED
noinst_PROGRAMS   		 	= $(EXAMPLES) 				\
					  $(MAEMO_GTK_EXAMPLES)
else
noinst_PROGRAMS   		 	= $(EXAMPLES) 				\
			     		  $(DEPRECATED_EXAMPLES)		\
					  $(MAEMO_GTK_EXAMPLES)
endif

# Hildon window
hildon_window_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_window_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_window_example_SOURCES		= hildon-window-example.c

# Hildon stackable window
hildon_stackable_window_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_stackable_window_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_stackable_window_example_SOURCES = hildon-stackable-window-example.c

# Hildon logical color
hildon_logical_color_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_logical_color_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_logical_color_example_SOURCES	= hildon-logical-color-example.c

# Hildon wizard dialog
hildon_wizard_dialog_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_wizard_dialog_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_wizard_dialog_example_SOURCES	= hildon-wizard-dialog-example.c

# Hildon live search
hildon_live_search_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_live_search_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_live_search_example_SOURCES	= hildon-live-search-example.c

# Hildon live search in a grid
hildon_live_search_grid_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_live_search_grid_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_live_search_grid_example_SOURCES	= hildon-live-search-grid-example.c

# Hildon find toolbar
hildon_find_toolbar_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_find_toolbar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_find_toolbar_example_SOURCES	= hildon-find-toolbar-example.c

# Hildon edit toolbar
hildon_edit_toolbar_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_edit_toolbar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_edit_toolbar_example_SOURCES	= hildon-edit-toolbar-example.c

# Hildon seekbar
hildon_seekbar_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_seekbar_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_seekbar_example_SOURCES		= hildon-seekbar-example.c

# Hildon toolbar seekbar
hildon_toolbar_seekbar_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_toolbar_seekbar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_toolbar_seekbar_example_SOURCES	= hildon-toolbar-seekbar-example.c

# Hildon finger
hildon_finger_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_finger_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_finger_example_SOURCES		= hildon-finger-example.c

# Hildon banner
hildon_banner_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_banner_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_banner_example_SOURCES		= hildon-banner-example.c

# Hildon banner
hildon_banner_dnd_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_banner_dnd_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_banner_dnd_example_SOURCES	= hildon-banner-dnd-example.c

# Hildon banner long
hildon_banner_long_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_banner_long_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_banner_long_example_SOURCES	= hildon-banner-long-example.c

# Hildon caption
hildon_caption_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_caption_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_caption_example_SOURCES		= hildon-caption-example.c

# Hildon weekday picker
hildon_weekday_picker_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_weekday_picker_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_weekday_picker_example_SOURCES	= hildon-weekday-picker-example.c

# Hildon time picker
hildon_time_picker_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_time_picker_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_time_picker_example_SOURCES	= hildon-time-picker-example.c

# Hildon time editor
hildon_time_editor_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_time_editor_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_time_editor_example_SOURCES	= hildon-time-editor-example.c

# Hildon sort dialog
hildon_sort_dialog_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_sort_dialog_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_sort_dialog_example_SOURCES	= hildon-sort-dialog-example.c

# Hildon control bar
hildon_controlbar_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_controlbar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_controlbar_example_SOURCES	= hildon-controlbar-example.c

# Hildon note example
hildon_note_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_note_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_note_example_SOURCES		= hildon-note-example.c

# Hildon color chppser example
hildon_color_chooser_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_color_chooser_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_color_chooser_example_SOURCES	= hildon-color-chooser-example.c

# Hildon color button example
hildon_color_button_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_color_button_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_color_button_example_SOURCES	= hildon-color-button-example.c

# Hildon color pop example
hildon_color_pop_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_color_pop_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_color_pop_example_SOURCES	= hildon-color-pop-example.c

# Hildon color button example
hildon_login_dialog_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_login_dialog_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_login_dialog_example_SOURCES	= hildon-login-dialog-example.c

# HIldon window
hildon_icon_sizes_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_icon_sizes_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_icon_sizes_example_SOURCES	= hildon-icon-sizes-example.c

# HIldon insensitive example
hildon_insensitive_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_insensitive_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_insensitive_example_SOURCES	= hildon-insensitive-example.c

# HIldon insensitive map example
hildon_hvolumebar_insensitive_map_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_hvolumebar_insensitive_map_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
							  $(EXTRA_CFLAGS)
hildon_hvolumebar_insensitive_map_example_SOURCES	= hildon-hvolumebar-insensitive-map-example.c

# HIldon get password dialog example
hildon_get_password_dialog_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_get_password_dialog_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_get_password_dialog_example_SOURCES	= hildon-get-password-dialog-example.c

# HIldon set password dialog example
hildon_set_password_dialog_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_set_password_dialog_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_set_password_dialog_example_SOURCES	= hildon-set-password-dialog-example.c

# HIldon window menu example
hildon_window_menu_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_window_menu_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_window_menu_example_SOURCES	= hildon-window-menu-example.c

# Hildon window common menu example
hildon_window_cmn_menu_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_window_cmn_menu_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_window_cmn_menu_example_SOURCES	= hildon-window-cmn-menu-example.c

# Hildon vvolume bar example
hildon_vvolumebar_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_vvolumebar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_vvolumebar_example_SOURCES	= hildon-vvolumebar-example.c

# Hildon hvolume bar example
hildon_hvolumebar_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_hvolumebar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_hvolumebar_example_SOURCES	= hildon-hvolumebar-example.c

# Hildon hvolume bar insensitive example
hildon_hvolumebar_insensitive_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_hvolumebar_insensitive_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_hvolumebar_insensitive_example_SOURCES	= hildon-hvolumebar-insensitive-example.c

# Hildon toolbar example
hildon_toolbar_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_toolbar_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_toolbar_example_SOURCES		= hildon-toolbar-example.c

# Hildon lock code dialog example
hildon_code_dialog_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_code_dialog_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_code_dialog_example_SOURCES	= hildon-code-dialog-example.c

# Hildon lookup example
hildon_lookup_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_lookup_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_lookup_example_SOURCES		= hildon-lookup-example.c

# Hildon number editor
hildon_number_editor_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_number_editor_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_number_editor_example_SOURCES	= hildon-number-editor-example.c

# Hildon scrolled window
hildon_scrolled_window_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_scrolled_window_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_scrolled_window_example_SOURCES	= hildon-scrolled-window-example.c

# Hildon thumb scrollbar example
hildon_thumb_scrollbar_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_thumb_scrollbar_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_thumb_scrollbar_example_SOURCES	= hildon-thumb-scrollbar-example.c

# Hildon date editor example
hildon_date_editor_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_date_editor_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_date_editor_example_SOURCES	= hildon-date-editor-example.c

# Hildon bread crumb trail example
hildon_bread_crumb_trail_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_bread_crumb_trail_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_bread_crumb_trail_example_SOURCES	= hildon-bread-crumb-trail-example.c

# Hildon volumebar timer example
hildon_hvolumebar_timer_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_hvolumebar_timer_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_hvolumebar_timer_example_SOURCES		= hildon-hvolumebar-timer-example.c

# Hildon pannable area tree view example
hildon_pannable_area_tree_view_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_pannable_area_tree_view_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
						  	   $(EXTRA_CFLAGS)
hildon_pannable_area_tree_view_example_SOURCES		= hildon-pannable-area-tree-view-example.c

# Hildon pannable area tuning example
hildon_pannable_area_tuning_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_pannable_area_tuning_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
						  	   $(EXTRA_CFLAGS)
hildon_pannable_area_tuning_example_SOURCES		= hildon-pannable-area-tuning-example.c

# Hildon pannable area scroll jump example
hildon_pannable_area_scroll_jump_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_pannable_area_scroll_jump_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)
hildon_pannable_area_scroll_jump_example_SOURCES	= hildon-pannable-area-scroll-jump-example.c

# Hildon pannable area buttons scroll example
hildon_pannable_area_buttons_scroll_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_pannable_area_buttons_scroll_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)
hildon_pannable_area_buttons_scroll_example_SOURCES		= hildon-pannable-area-buttons-scroll-example.c

# Hildon pannable area gesture signals example
hildon_pannable_area_gesture_signals_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_pannable_area_gesture_signals_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)
hildon_pannable_area_gesture_signals_example_SOURCES		= hildon-pannable-area-gesture-signals-example.c

# Hildon pannable area initial hint example
hildon_pannable_area_initial_hint_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_pannable_area_initial_hint_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)
hildon_pannable_area_initial_hint_example_SOURCES	= hildon-pannable-area-initial-hint-example.c

# Hildon pannable area touch list
hildon_pannable_area_touch_list_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_pannable_area_touch_list_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_pannable_area_touch_list_example_SOURCES	= hildon-pannable-area-touch-list-example.c

# Hildon pannable area touch grid
hildon_pannable_area_touch_grid_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_pannable_area_touch_grid_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_pannable_area_touch_grid_example_SOURCES	= hildon-pannable-area-touch-grid-example.c

# Hildon app menu
hildon_app_menu_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_app_menu_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_app_menu_example_SOURCES		= hildon-app-menu-example.c

# Hildon entry
hildon_entry_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_entry_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_entry_example_SOURCES		= hildon-entry-example.c

# Hildon text view
hildon_text_view_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_text_view_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_text_view_example_SOURCES	= hildon-text-view-example.c

# Hildon button
hildon_button_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_button_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_button_example_SOURCES		= hildon-button-example.c

# Hildon dialog
hildon_dialog_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_dialog_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_dialog_example_SOURCES		= hildon-dialog-example.c

# Hildon date button
hildon_date_button_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_date_button_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_date_button_example_SOURCES	= hildon-date-button-example.c

# Hildon picker button
hildon_picker_button_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_picker_button_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_picker_button_example_SOURCES	= hildon-picker-button-example.c

# Hildon picker button with multicolumn picker
hildon_picker_button_multicolumn_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_picker_button_multicolumn_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
						  $(EXTRA_CFLAGS)
hildon_picker_button_multicolumn_example_SOURCES= hildon-picker-button-multicolumn-example.c

# Hildon time button
hildon_time_button_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_time_button_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_time_button_example_SOURCES	= hildon-time-button-example.c

# Hildon check button
hildon_check_button_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_check_button_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_check_button_example_SOURCES	= hildon-check-button-example.c

# Hildon touch picker with multiselection
hildon_touch_selector_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_touch_selector_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  $(EXTRA_CFLAGS)
hildon_touch_selector_example_SOURCES	= hildon-touch-selector-example.c

# Hildon touch picker using HILDON_UI_MODE_NORMAL
hildon_touch_selector_normal_mode_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_touch_selector_normal_mode_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
							  $(EXTRA_CFLAGS)
hildon_touch_selector_normal_mode_example_SOURCES	= hildon-touch-selector-normal-mode-example.c

# Hildon touch picker with several cells per column
hildon_touch_selector_multi_cells_example_LDADD		= $(HILDON_OBJ_LIBS)
hildon_touch_selector_multi_cells_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					  	          $(EXTRA_CFLAGS)
hildon_touch_selector_multi_cells_example_SOURCES	= hildon-touch-selector-multi-cells-example.c

# Hildon touch selector with entry
hildon_touch_selector_entry_example_LDADD	= $(HILDON_OBJ_LIBS)
hildon_touch_selector_entry_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					          $(EXTRA_CFLAGS)
hildon_touch_selector_entry_example_SOURCES	= hildon-touch-selector-entry-example.c

# Hildon progress indicator
hildon_progress_indicator_example_LDADD	        = $(HILDON_OBJ_LIBS)
hildon_progress_indicator_example_CFLAGS	= $(HILDON_OBJ_CFLAGS)			\
					          $(EXTRA_CFLAGS)
hildon_progress_indicator_example_SOURCES	= hildon-progress-indicator-example.c

# Hildon remote texture
hildon_remote_texture_example_LDADD	        = $(HILDON_OBJ_LIBS)
hildon_remote_texture_example_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					          $(EXTRA_CFLAGS)
hildon_remote_texture_example_SOURCES		= hildon-remote-texture-example.c

# Hildon remote texture
hildon_gtk_window_take_screenshot_sync_LDADD	        = $(HILDON_OBJ_LIBS)
hildon_gtk_window_take_screenshot_sync_CFLAGS		= $(HILDON_OBJ_CFLAGS)			\
					          $(EXTRA_CFLAGS)
hildon_gtk_window_take_screenshot_sync_SOURCES		= hildon-gtk-window-take-screenshot-sync.c

endif
