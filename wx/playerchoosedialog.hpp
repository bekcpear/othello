/**
 * @file     wx/playerchoosedialog.hpp
 * @summary  choose player dialog
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: playerchoosedialog.hpp,v 1.3 2007/06/16 03:41:59 hellwolfmisty Exp $
 */

#ifndef WX_PLAYERCHOOSEDIALOG_HPP
#define WX_PLAYERCHOOSEDIALOG_HPP

#include <string>
#include <map>

#include <wx/window.h>

#include "othello/player_plugin_manager.hpp"
#include "thememgr.hpp"

using namespace othello;

typedef std::map<std::string, std::string> argmap;

void ChoosePlayers(PlayerPluginManager *ppm,
                   ThemeManager *theme, wxWindow* parent,
                   std::string &player_a, std::string &player_b,
                   argmap &a_args, argmap &b_args);

#endif
