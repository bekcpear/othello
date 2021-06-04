SUBDIRS = resources

bin_PROGRAMS = wxothello

wxothello_SOURCES = \
	configmgr.hpp \
	configmgr.cpp \
	thememgr.hpp \
	thememgr.cpp \
	wxothelloframe.hpp \
	wxothelloframe.cpp \
	playerchoosedialog.hpp \
	playerchoosedialog.cpp \
	windowboardpainter.hpp \
	windowboardpainter.cpp \
	wxothelloplayer.hpp \
	wxothelloplayer.cpp \
	wxothello.cpp

if SUPPORT_MINGW32
.rc.o:
	$(WINDRES) -o "$@" "$<"

wxothello_SOURCES += win32.rc
endif

wxothello_CXXFLAGS = \
	$(WX_CXXFLAGS) \
	-I$(top_srcdir)/lib

wxothello_LDADD = \
	$(top_builddir)/lib/othello/libothello.la \
	$(WX_LIBS)

