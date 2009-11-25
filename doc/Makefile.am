MAINTAINERCLEANFILES 				= Makefile.in				\
						  hildon-sections.txt			\
						  hildon-undocumented.txt		\
						  version.xml				\
						  tmpl/*.sgml

DOC_MODULE					= hildon

DOC_MAIN_SGML_FILE				= $(DOC_MODULE)-docs.sgml

SCAN_OPTIONS					=--deprecated-guards="HILDON_DISABLE_DEPRECATED"

SCANOBJ_OPTIONS					=--type-init-func="gtk_type_init(0)"

DOC_SOURCE_DIR					= $(top_srcdir)/hildon
DOC_BUILD_DIR 					= $(top_builddir)/hildon


HFILE_GLOB					= $(top_srcdir)/hildon/*.h

CFILE_GLOB					= $(top_srcdir)/hildon/*.c

IGNORE_HFILES 					= hildon-calendar-popup-private.h	\
						  hildon-caption-private.h 		\
						  hildon-code-dialog-private.h 		\
						  hildon-color-button-private.h 	\
						  hildon-color-chooser-dialog-private.h \
						  hildon-color-chooser-private.h 	\
						  hildon-controlbar-private.h 		\
						  hildon-date-editor-private.h 		\
						  hildon-find-toolbar-private.h 	\
						  hildon-font-selection-dialog-private.h\
						  hildon-get-password-dialog-private.h 	\
						  hildon-login-dialog-private.h 	\
						  hildon-note-private.h 		\
						  hildon-number-editor-private.h 	\
						  hildon-private.c 			\
						  hildon-private.h 			\
						  hildon-program-private.h 		\
						  hildon-range-editor-private.h 	\
						  hildon-seekbar-private.h 		\
						  hildon-set-password-dialog-private.h 	\
						  hildon-sort-dialog-private.h 		\
						  hildon-time-editor-private.h 		\
						  hildon-time-picker-private.h 		\
						  hildon-volumebar-private.h 		\
						  hildon-weekday-picker-private.h 	\
						  hildon-window-private.h 		\
						  hildon-app-menu-private.h		\
						  hildon-calendar-private.h		\
						  hildon-stackable-window-private.h	\
						  hildon-wizard-dialog-private.h

INCLUDES 					= $(GTK_CFLAGS) -I$(top_srcdir)

MKDB_OPTIONS					=--sgml-mode --output-format=xml --default-includes=hildon/hildon.h

GTKDOC_CFLAGS 					= $(HILDON_OBJ_CFLAGS)

GTKDOC_LIBS 					= $(HILDON_OBJ_LIBS)

version.xml: ../configure.ac
	@echo $(VERSION) > version.xml

html: html-build.stamp

HTML_IMAGES 					= $(srcdir)/images/calendar-popup.png		\
						  $(srcdir)/images/caption.png			\
						  $(srcdir)/images/code-dialog.png		\
						  $(srcdir)/images/color-button.png		\
						  $(srcdir)/images/color-chooser-dialog.png	\
						  $(srcdir)/images/color-chooser.png		\
						  $(srcdir)/images/controlbar.png		\
						  $(srcdir)/images/date-editor.png		\
						  $(srcdir)/images/font-selection-dialog.png	\
						  $(srcdir)/images/get-password-dialog.png	\
						  $(srcdir)/images/hvolumebar.png		\
						  $(srcdir)/images/login-dialog.png		\
						  $(srcdir)/images/note.png			\
						  $(srcdir)/images/number-editor.png		\
						  $(srcdir)/images/range-editor.png		\
						  $(srcdir)/images/seekbar.png			\
						  $(srcdir)/images/set-password-dialog.png	\
						  $(srcdir)/images/sort-dialog.png		\
						  $(srcdir)/images/time-editor.png		\
						  $(srcdir)/images/time-picker.png		\
						  $(srcdir)/images/vvolumebar.png		\
						  $(srcdir)/images/weekday-picker.png		\
						  $(srcdir)/images/bread-crumb-trail.png

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE)
content_files =				\
	migration.sgml			\
	running.sgml			\
	building.sgml			\
	compiling.sgml			\
	resources.sgml			\
	version.xml			\
	visual_index.xml

FIXXREF_OPTIONS					=

include $(top_srcdir)/doc/gtk-doc.make

