/**
 * @file     wx/wxothello.cpp
 * @summary  wx othello frontend
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: wxothello.cpp,v 1.7 2009/07/05 19:30:56 hellwolfmisty Exp $
 */

#include <exception>
#include <wx/wx.h>

#include "wxothelloframe.hpp"

class wxOthello : public wxApp
{
public:
  virtual bool OnInit(){
    Create_wxOthelloFrame();
    return true;
  }

  virtual int OnRun(){
    try{
      return wxApp::OnRun();
    }catch(std::exception& e){
      wxLogError(wxT("std::exception catched : %s\n"),
                 wxString(e.what(), wxConvUTF8).c_str());
      return 1;
    }catch(...){
      wxLogError(wxT("Unknown exception catched"));
      return 1;
    }
  }
};

IMPLEMENT_APP(wxOthello)

