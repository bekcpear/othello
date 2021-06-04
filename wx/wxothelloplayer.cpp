/**
 * @file     wx/wxothelloplayer.cpp
 * @summary  wx othello human player
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: wxothelloplayer.cpp,v 1.4 2007/06/22 06:50:32 hellwolfmisty Exp $
 */

#include <wx/wx.h>

#include "windowboardpainter.hpp"
#include "wxothelloplayer.hpp"

class wxOthelloPlayer :
  public wxEvtHandler,
  public Player{
  int is_my_turn;
  ThemeManager *theme;
  wxWindow *window_board;
  wxSemaphore sem;
  enum {CMD_NONE, CMD_GO, CMD_PASS, CMD_ENDGAME} cmd;
  int xx, yy;
  wxMutex cmdmutex;

  int margin_x, margin_y;
  int grid_size;

public:
  wxOthelloPlayer(wxWindow *_window_board, ThemeManager *_theme){
    window_board = _window_board;
    is_my_turn = 0;
    theme = _theme;
    window_board->PushEventHandler(this);
    cmd = CMD_NONE;

    margin_x = theme->get_margin_x();
    margin_y = theme->get_margin_y();
    grid_size = theme->get_black_chess(0).GetWidth();
  }
  ~wxOthelloPlayer(){
    window_board->PopEventHandler();
  }
  virtual void do_prepare(){
    cmd = CMD_NONE;
    ready();
  }
  virtual void do_get_turn(Board &board){
    is_my_turn = 1;
    while(1){
      sem.Wait();
      cmdmutex.Lock();
      if(cmd == CMD_GO){
        int x = xx;
        int y = yy;
        if(board.try_go(x, y, get_myside()) == Board::GO_OK){
          cmdmutex.Unlock();
          go(x, y);
          break;
        }
      }else if(cmd == CMD_PASS){
        cmdmutex.Unlock();
        pass();
        break;
      }else if(cmd == CMD_ENDGAME){
        cmdmutex.Unlock();
        break;
      }
      cmdmutex.Unlock();
    }
    is_my_turn = 0;
    cmd = CMD_NONE;
  }  
  virtual void do_end_game(){
    wxMutexLocker lock(cmdmutex);
    cmd = CMD_ENDGAME;
    sem.Post();
  }
  void OnLeftClick(wxMouseEvent& event){
    if(is_my_turn){
      int x = (event.GetX() - margin_x)/grid_size;
      int y = (event.GetY() - margin_y)/grid_size;
      if(x >= 8 || y >= 8){
        x = y = -1;
      }else{
        wxMutexLocker lock(cmdmutex);
        cmd = CMD_GO;
        xx = x;
        yy = y;
        sem.Post();
      }
    }
    event.Skip();
  }
  void OnRightClick(wxMouseEvent& event){
    if(is_my_turn){
      wxMutexLocker lock(cmdmutex);
      cmd = CMD_PASS;
      sem.Post();
    }
    event.Skip();
  }
protected:
  DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxOthelloPlayer, wxEvtHandler)
EVT_LEFT_DOWN(wxOthelloPlayer::OnLeftClick)
EVT_RIGHT_DOWN(wxOthelloPlayer::OnRightClick)
END_EVENT_TABLE();

Player* Create_wxOthelloPlayer(wxWindow *window_board, ThemeManager *t){
  return new wxOthelloPlayer(window_board, t);
}
