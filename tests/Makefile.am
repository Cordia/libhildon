MAINTAINERCLEANFILES 					= Makefile.in
INCLUDES						= -I$(top_srcdir)

if BUILD_TESTS

noinst_PROGRAMS				= check_test
TESTS					= check_test

tests					= check_test.c 				\
					  check_utils.c 			\
					  check-hildon-caption.c 		\
					  check-hildon-helper.c 		\
					  check-hildon-note.c 			\
					  check-hildon-wizard-dialog.c 		\
					  check-hildon-find-toolbar.c 		\
					  check-hildon-window.c 		\
					  check-hildon-program.c		\
					  check-hildon-picker-button.c

check_test_SOURCES   		 	= $(tests)

check_test_SOURCES		       += test_suites.h				\
					  check_utils.h

check_test_LDADD			= $(HILDON_OBJ_LIBS)
check_test_LDFLAGS			= -module -avoid-version
check_test_CFLAGS			= $(HILDON_OBJ_CFLAGS)

endif
