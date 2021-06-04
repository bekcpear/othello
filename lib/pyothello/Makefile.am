#pylibdir=$(OTHELLO_PYLIBDIR)
lib_LTLIBRARIES = libothello-python.la

libothello_python_la_SOURCES = \
	utility.hpp \
	utility.cpp \
	board.hpp \
	board.cpp \
	player.hpp \
	player.cpp \
	intl.cpp \
	init.hpp \
	init.cpp

libothello_python_la_CXXFLAGS = \
	-I$(top_srcdir)/lib \
	$(PYTHON_CPPFLAGS)

libothello_python_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO) \
	$(PYTHON_LDFLAGS) \
	$(PYTHON_EXTRA_LDFLAGS)

if SUPPORT_MINGW32
libothello_python_la_LDFLAGS += $(MINGW32_DLL_LDFLAGS)
endif

libothello_python_la_LIBADD = \
	$(PYTHON_EXTRA_LIBS) \
	-lboost_python$(LIBBOOST_SUFFIX) \
	$(top_builddir)/lib/othello/libothello.la

