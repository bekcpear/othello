/**
 * @file     wx/theme.hpp
 * @summary  wx othello theme manager
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: thememgr.hpp,v 1.3 2007/06/22 06:50:32 hellwolfmisty Exp $
 */

#ifndef WX_THEMEMGR_HPP
#define WX_THEMEMGR_HPP

#include <vector>
#include <wx/wx.h>
#include <wx/image.h>

#include "othello/board.hpp"

class ConfigManager;

class ThemeManager{
protected:
  wxWindow *window_board;

  wxImage image_background;
  wxImage image_white;
  wxImage image_black;

  std::vector<wxImage> images_white;
  std::vector<wxImage> images_black;

  int margin_x;
  int margin_y;

public:
  virtual ~ThemeManager(){}
  static ThemeManager* Create(ConfigManager *);

public:
  inline wxImage& get_background(){
    return image_background;
  }

  inline int get_black_chess_size(){
    return images_white.size();
  }
  inline int get_white_chess_size(){
    return images_black.size();
  }
  inline int get_chess_size(othello::CHESS_SIDE s){
    return s == othello::BLACK?get_black_chess_size():
      get_white_chess_size();
  }

  inline wxImage& get_black_chess(int i){
    return images_black[i];
  }
  inline wxImage& get_white_chess(int i){
    return images_white[i];
  }
  inline wxImage& get_chess(othello::CHESS_SIDE s, int num){
    return s == othello::BLACK?get_black_chess(num):
      get_white_chess(num);
  }

  inline int get_margin_x(){
    return margin_x;
  }

  inline int get_margin_y(){
    return margin_y;
  }

public:
  virtual int ReloadTheme(std::string &err) = 0;
  virtual int Load_wxOthelloFrame(wxFrame *frame, wxWindow *parent) = 0;
  virtual int Load_ChoosePlayersDialog(wxDialog* dialog, wxWindow *parent) = 0;
  virtual int Load_PreferenceDialog(wxDialog* dialog, wxWindow *parent) = 0;
};

#endif

