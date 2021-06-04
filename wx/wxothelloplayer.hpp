/**
 * @file     wx/wxothelloplayer.hpp
 * @summary  wx othello human player
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: wxothelloplayer.hpp,v 1.3 2007/06/22 06:50:32 hellwolfmisty Exp $
 */

#ifndef WX_WXOTHELLOPLAYER_HPP
#define WX_WXOTHELLOPLAYER_HPP

#include <wx/wx.h>

#include "othello/player.hpp"

#include "thememgr.hpp"

Player* Create_wxOthelloPlayer(wxWindow *window_board, ThemeManager *);

#endif

