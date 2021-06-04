/**
 * @file     wx/windowboardpainter.hpp
 * @summary  paint the othello window board
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: windowboardpainter.hpp,v 1.3 2007/06/22 06:50:32 hellwolfmisty Exp $
 */


#ifndef WX_WINDOWBOARDPAINTER_HPP
#define WX_WINDOWBOARDPAINTER_HPP

#include "othello/game.hpp"

#include "thememgr.hpp"

using namespace othello;

class WindowBoardPainter{
public:
  static WindowBoardPainter* Create(wxWindow *_window_board, ThemeManager *);
public:
  virtual ~WindowBoardPainter(){}
  virtual void SetGame(Game *g) = 0;
  virtual void ClearGame() = 0;
  virtual void UpdateTheme(ThemeManager *_theme) = 0;
  virtual void SetSpeed(int i) = 0;
};

#endif

