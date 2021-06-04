###########################################################
##          Some definitions of Version macros.          ##
###########################################################
define([othello_major_version], [0])
define([othello_minor_version], [2])
define([othello_micro_version], [2])
define([othello_version], othello_major_version.othello_minor_version.othello_micro_version)
ifelse(m4_eval(othello_minor_version%2), [1],
        [
                #deval branch
                define([othello_apiversion], othello_major_version.othello_minor_version.othello_micro_version)
                define([lt_current], m4_eval(othello_major_version+othello_minor_version+othello_micro_version))
                define([lt_revision], 0)
        ],
        [
                #stable branch
                define([othello_apiversion], othello_major_version.othello_minor_version)
                define([lt_current], m4_eval(othello_major_version+othello_minor_version))
                define([lt_revision], othello_micro_version)
        ]
)
define([lt_age], [0])

###########################################################
##                  Start Configuration.                 ##
###########################################################
AC_INIT([othello], [othello_version])

# program check
AC_PROG_CXX
AC_LANG(C++)
AC_DISABLE_STATIC
AM_PROG_LIBTOOL
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])
IT_PROG_INTLTOOL

#gettext support
GETTEXT_PACKAGE=othello
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", "definition of GETTEXT_PACKAGE")

# version prepare
OTHELLO_MAJOR_VERSION=othello_major_version
OTHELLO_MINOR_VERSION=othello_minor_version
OTHELLO_MICRO_VERSION=othello_micro_version
OTHELLO_APIVERSION=othello_apiversion
OTHELLO_VERSION=othello_version
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
AC_SUBST(OTHELLO_MAJOR_VERSION)
AC_SUBST(OTHELLO_MINOR_VERSION)
AC_SUBST(OTHELLO_MICOR_VERSION)
AC_SUBST(OTHELLO_APIVERSION)
AC_DEFINE_UNQUOTED(OTHELLO_APIVERSION, "$OTHELLO_APIVERSION", [api version of othello lib])
AC_SUBST(OTHELLO_VERSION)
AC_DEFINE_UNQUOTED(OTHELLO_VERSION, "$OTHELLO_VERSION", [version of othello lib])
AC_SUBST(LT_VERSION_INFO)

# options
AC_ARG_ENABLE(mingw32, [  --enable-mingw32    enable mingw32 support], [enable_mingw32=yes], [])
AM_CONDITIONAL(SUPPORT_MINGW32, test "x$enable_mingw32" == "xyes")
if test "x$enable_mingw32" == "xyes";then
    AC_LIBTOOL_WIN32_DLL
    AM_DISABLE_STATIC
    AC_CHECK_TOOL(WINDRES, windres)
    MINGW32_DLL_LDFLAGS="-no-undefined"
    LDFLAGS=$LDFLAGS" -Xlinker --enable-auto-import"
    AC_SUBST(MINGW32_DLL_LDFLAGS)
fi

AC_ARG_ENABLE(cli, [  --disable-cli    disable the command line game interface], [], [enable_cli=yes])
if test "x$enable_cli" = "xyes"; then
    OTHELLO_CLI_SUBDIR=cli
    AC_SUBST(OTHELLO_CLI_SUBDIR)
fi

AC_ARG_ENABLE(wx, [  --disable-wx    disable the wxwidget game interface], [], [enable_wx=yes])
if test "x$enable_wx" = "xyes"; then
    OTHELLO_WX_SUBDIR=wx
    AC_SUBST(OTHELLO_WX_SUBDIR)
fi

AC_ARG_ENABLE(python, [  --disable-python    disable python othello lib and python ai], [], [enable_python=yes])
if test "x$enable_python" = "xyes"; then
    OTHELLO_LIB_PYOTHELLO_SUBDIR=pyothello
    AC_SUBST(OTHELLO_LIB_PYOTHELLO_SUBDIR)
fi

# check boost lib
AC_ARG_VAR([LIBBOOST_SUFFIX], [Some distribution provides boost library
with certain release suffix, e.g libboost_python-mt-1_34_1.so instead of
libboost.so. You can specify the release suffix by setting this variable.
])
AC_CHECK_HEADER([boost/thread.hpp],,AC_MSG_ERROR([boost::thread is not available]))
AC_CHECK_HEADER([boost/regex.hpp],,AC_MSG_ERROR([boost::regex is not available]))
AC_CHECK_HEADER([boost/asio.hpp],,AC_MSG_ERROR([boost::asio is not available]))

# wx check
if test "x$enable_wx" == "xyes"; then

WX_CONFIG=wx-config
AC_ARG_WITH(wx-config,
[[  --with-wx-config=FILE
    Use the given path to wx-config when determining
    wxWidgets configuration; defaults to "wx-config"]],
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WX_CONFIG=$withval
    fi
])
AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WX_CONFIG --version`; then
    AC_MSG_RESULT([$wxversion])
else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
fi
WX_CPPFLAGS="`$WX_CONFIG --cppflags`"
WX_CXXFLAGS="`$WX_CONFIG --cxxflags`"
WX_LIBS="`$WX_CONFIG --libs`"
AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS)
AC_SUBST(WX_LIBS)

fi # wx check

# python check
AM_CONDITIONAL(ENABLE_PYTHON, test "x$enable_python" == "xyes")
if test "x$enable_python" == "xyes"; then
    if test "x$enable_mingw32" != "xyes";then
        AC_PYTHON_DEVEL
    fi

    # boost::python is needed when python is enabled
    CPPFLAGS_SAVE=$CPPFLAGS
    CPPFLAGS="$CPPFLAGS $PYTHON_CPPFLAGS"
    AC_CHECK_HEADER(boost/python.hpp,,AC_MSG_ERROR([boost::python is not available while python support is needed]))
    CPPFLAGS=$CPPFLAGS_SAVE
fi # python check

#directories
OTHELLO_LOCALEDIR="$prefix/$DATADIRNAME/locale"
AC_SUBST(OTHELLO_LOCALEDIR)

OTHELLO_INCLUDEDIR="$includedir/othello/$OTHELLO_VERSION"
AC_SUBST(OTHELLO_INCLUDEDIR)

OTHELLO_LIBDIR="$libdir/othello/$OTHELLO_VERSION"
AC_SUBST(OTHELLO_LIBDIR)

OTHELLO_PYLIBDIR="$OTHELLO_LIBDIR/python"
AC_SUBST(OTHELLO_PYLIBDIR)

OTHELLO_PLAYERPLUGINSDIR="$OTHELLO_LIBDIR/players"
AC_SUBST(OTHELLO_PLAYERPLUGINSDIR)

OTHELLO_COMPATIBLE_PLAYERPLUGINSDIR="$libdir/othello/$OTHELLO_APIVERSION/players"
AC_SUBST(OTHELLO_COMPATIBLE_PLAYERPLUGINSDIR)

OTHELLO_WX_DATADIR="$datadir/wxothello/$OTHELLO_VERSION"
AC_SUBST(OTHELLO_WX_DATADIR)

AC_DEFINE_UNQUOTED(CONFIGURE_ARGS, "$ac_configure_args", [configure arguments])

###########################################################
##                      Output files.                    ##
###########################################################
AC_CONFIG_FILES([
Makefile
include/Makefile
lib/Makefile
lib/othello/Makefile
lib/othello/othello.pc
lib/pyothello/Makefile
players/Makefile
cli/Makefile
wx/Makefile
wx/resources/Makefile
po/Makefile.in
docs/Makefile
m4/Makefile
])
AC_OUTPUT

