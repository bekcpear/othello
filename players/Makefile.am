playersdir=$(OTHELLO_PLAYERPLUGINSDIR)
players_LTLIBRARIES = lowai.la 

players_CXXFLAGS = \
	-I$(top_srcdir)/lib

players_LDFLAGS = \
	-avoid-version \
	-Xlinker -export-dynamic \
	-module

if SUPPORT_MINGW32
players_LDFLAGS += $(MINGW32_DLL_LDFLAGS)
endif

players_LIBADD = \
	$(top_builddir)/lib/othello/libothello.la

# player lowai
lowai_la_SOURCES = \
	lowai.hpp \
	lowai.cpp \
	lowai_plugin.cpp

lowai_la_CXXFLAGS = $(players_CXXFLAGS)

lowai_la_LDFLAGS = $(players_LDFLAGS)

lowai_la_LIBADD = $(players_LIBADD)

if ENABLE_PYTHON
players_LTLIBRARIES += python.la
# player python
python_la_SOURCES = \
	python.cpp

python_la_CXXFLAGS = $(players_CXXFLAGS) \
	$(PYTHON_CPPFLAGS)

python_la_LDFLAGS = $(players_LDFLAGS) \
	$(PYTHON_LDFLAGS) \
	$(PYTHON_EXTRA_LDFLAGS)

python_la_LIBADD = $(players_LIBADD) \
	$(PYTHON_EXTRA_LIBS) \
	-lboost_python$(LIBBOOST_SUFFIX) \
	$(top_builddir)/lib/pyothello/libothello-python.la

pyplayersdir=$(OTHELLO_PLAYERPLUGINSDIR)/python
pyplayers_SCRIPTS = \
	python/silverfish.py

EXTRA_DIST = $(pyplayers_SCRIPTS)

endif

install-data-hook:
	install -d $(DESTDIR)/$(OTHELLO_COMPATIBLE_PLAYERPLUGINSDIR)
	rm -f $(DESTDIR)/$(OTHELLO_PLAYERPLUGINSDIR)/*.la

