/**
 * @file     wx/configmgr.hpp
 * @summary  wx othello config manager
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: configmgr.hpp,v 1.1 2007/06/22 06:55:14 hellwolfmisty Exp $
 */

#ifndef WX_CONFIGMGR_HPP
#define WX_CONFIGMGR_HPP

#include <wx/wx.h>
#include <vector>
#include <string>

class ThemeManager;

class ConfigManager{
public:
  virtual ~ConfigManager(){}
  static ConfigManager* Create();

public:

public:
  virtual int get_langcode() = 0;

  virtual std::string get_datadir() = 0;

  virtual std::string get_themesdir() = 0;

  virtual void reset_themename() = 0;
  virtual std::string get_themename() = 0;
  virtual std::string get_themedir() = 0;

  virtual void get_plugindirs(std::vector<std::string> &) = 0;

  virtual unsigned short get_serverport()= 0;

  virtual int get_gamespeed() = 0;

public:
  virtual int show_config_dialog(ThemeManager *theme, wxWindow *parent) = 0;
};

#endif
