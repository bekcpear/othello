noinst_LTLIBRARIES = libothello_configs.la
lib_LTLIBRARIES = libothello.la

libothello_configs_la_SOURCES = \
	configs.hpp \
	configs.cpp

if SUPPORT_MINGW32
libothello_configs_la_CXXFLAGS = \
	-DOTHELLO_LOCALEDIR=\"../lib/locale\" \
	-DOTHELLO_LIBDIR=\"\" \
	-DOTHELLO_PYLIBDIR=\"\" \
	-DOTHELLO_PLAYERPLUGINSDIR=\"../lib/othello/$(OTHELLO_VERSION)/players\" \
	-DOTHELLO_COMPATIBLE_PLAYERPLUGINSDIR=\"../lib/othello/$(OTHELLO_APIVERSION)/players\" \
	-DOTHELLO_WX_DATADIR=\"../share/wxothello/$(OTHELLO_VERSION)\"
else
libothello_configs_la_CXXFLAGS = \
	-DOTHELLO_LOCALEDIR=\"$(OTHELLO_LOCALEDIR)\" \
	-DOTHELLO_LIBDIR=\"$(OTHELLO_LIBDIR)\" \
	-DOTHELLO_PYLIBDIR=\"$(OTHELLO_PYLIBDIR)\" \
	-DOTHELLO_PLAYERPLUGINSDIR=\"$(OTHELLO_PLAYERPLUGINSDIR)\" \
	-DOTHELLO_COMPATIBLE_PLAYERPLUGINSDIR=\"$(OTHELLO_COMPATIBLE_PLAYERPLUGINSDIR)\" \
	-DOTHELLO_WX_DATADIR=\"$(OTHELLO_WX_DATADIR)\"
endif

libothelloincludedir = $(OTHELLO_INCLUDEDIR)/othello
libothelloinclude_HEADERS = \
	observer.hpp \
	board.hpp  \
	player.hpp \
	game.hpp   \
	networkgame.hpp \
	intl.hpp \
	player_plugin_manager.hpp \
	intl_config.hpp \
	player_plugin.hpp

libothello_la_SOURCES = \
	board.cpp  \
	game.cpp   \
	player.cpp \
	networkgame_server.cpp \
	networkgame_client.cpp \
	intl_config.cpp \
	player_plugin_manager.cpp

libothello_la_CXXFLAGS = \
	-I$(top_srcdir)/include

if SUPPORT_MINGW32
libothello_la_CXXFLAGS += \
	-DBOOST_THREAD_POSIX
endif

libothello_la_LDFLAGS = \
	-version-info	$(LT_VERSION_INFO)

if SUPPORT_MINGW32
libothello_la_LDFLAGS += $(MINGW32_DLL_LDFLAGS)
libothello_la_LIBADD = \
	libothello_configs.la \
	-lws2_32 \
	-lwsock32 \
	-lpthread \
	-lboost_system$(LIBBOOST_SUFFIX) \
	-lboost_thread$(LIBBOOST_SUFFIX)
else
libothello_la_LIBADD = \
	libothello_configs.la \
	-ldl \
	-lpthread \
	-lboost_system$(LIBBOOST_SUFFIX) \
	-lboost_thread$(LIBBOOST_SUFFIX)
endif

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = othello.pc

DISTCLEANFILES = $(pkgconfig_DATA)
