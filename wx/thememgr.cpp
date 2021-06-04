/**
 * @file     wx/theme.cpp
 * @summary  wx othello theme manager
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: thememgr.cpp,v 1.4 2007/06/22 06:50:31 hellwolfmisty Exp $
 */

#include <wx/xrc/xmlres.h>

#include "thememgr.hpp"
#include "configmgr.hpp"

using namespace othello;

class ThemeManagerImpl : public ThemeManager{
  ConfigManager *config;
  wxXmlResource xrc;
  int init_ok;

public:
  ThemeManagerImpl(ConfigManager *_config){
    config = _config;
    wxInitAllImageHandlers();
    xrc.InitAllHandlers();
    wxFileName xrcpath(wxString(config->get_datadir().c_str(), wxConvUTF8), wxT("frame.xrc"));
    if(!xrc.Load(xrcpath.GetFullPath())){
      init_ok = 0;
      return;
    }
    init_ok = 1;
  }

  int is_ok(){
    return init_ok;
  }

public:
  virtual int ReloadTheme(std::string &err){
    wxFileName filename;

#define LOADIMG(name)                                                   \
    filename = wxFileName(wxString(config->get_themedir().c_str(), wxConvUTF8), \
                          wxT(#name".png"));                            \
    image_##name.LoadFile(filename.GetFullPath());                      \
    if(!image_##name.Ok()){                                             \
      err += "Load "#name" Failed";                                     \
      return 1;                                                         \
    }                                                                   \
    image_##name.SetMaskColour(89, 88, 87);

    LOADIMG(background);
    LOADIMG(white);
    LOADIMG(black);
    if(image_white.GetWidth() != image_black.GetWidth() ||
       image_white.GetHeight() != image_black.GetHeight()){
      err += "Black and white theme image do not have the same size";
      return 1;
    }
    int h = image_white.GetHeight();
    if(image_white.GetWidth() % h){
      err += "Width of black or white theme image should be N times of its height";
      return 1;
    }
    margin_x = (image_background.GetWidth() - 8*h)/2;
    margin_y = (image_background.GetHeight() - 8*h)/2;
    for(int i = 0; i < int(image_white.GetWidth()/h); ++i)
      images_white.push_back(image_white.GetSubImage(wxRect(i*h, 0, h, h)));
    for(int i = 0; i < int(image_black.GetWidth()/h); ++i)
      images_black.push_back(image_black.GetSubImage(wxRect(i*h, 0, h, h)));

    return 0;
  }

  virtual int Load_wxOthelloFrame(wxFrame *frame, wxWindow *parent){
    return xrc.LoadFrame(frame, parent, wxT("frame_othello"));
  }

  virtual int Load_ChoosePlayersDialog(wxDialog* dialog, wxWindow *parent){
    return xrc.LoadDialog(dialog, parent, wxT("dialog_choose_players"));
  }

  virtual int Load_PreferenceDialog(wxDialog* dialog, wxWindow *parent){
    return xrc.LoadDialog(dialog, parent, wxT("preference_dialog"));
  }
};

ThemeManager* ThemeManager::Create(ConfigManager *config){
  ThemeManagerImpl *rt = new ThemeManagerImpl(config);
  if(rt->is_ok()){
    return rt;
  }else{
    delete rt;
    return NULL;
  }
}

