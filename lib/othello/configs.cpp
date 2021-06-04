/**
 * @file     lib/othello/configs.cpp
 * @summary  othello lib configs
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: configs.cpp,v 1.4 2009/07/05 05:15:56 hellwolfmisty Exp $
 */

#include "config.h"

#include "configs.hpp"

namespace othello{
  //in config.h
  const char* package = PACKAGE;
  const char* package_name = PACKAGE_NAME;
  const char* package_string = PACKAGE_STRING;
  const char* package_version = PACKAGE_VERSION;
  const char* apiversion = OTHELLO_APIVERSION;
  //pass as compiler flags
  const char* locale_dir = OTHELLO_LOCALEDIR;
  const char* libdir = OTHELLO_LIBDIR;
  const char* pylibdir = OTHELLO_PYLIBDIR;
  const char* playerpluginsdir = OTHELLO_PLAYERPLUGINSDIR;
  const char* compatible_playerpluginsdir = OTHELLO_COMPATIBLE_PLAYERPLUGINSDIR;
  const char* wx_datadir = OTHELLO_WX_DATADIR;
};

