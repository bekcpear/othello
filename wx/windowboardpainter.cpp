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
 * $Id: windowboardpainter.cpp,v 1.7 2007/06/22 06:50:32 hellwolfmisty Exp $
 */

#include <wx/wx.h>
#include <wx/dcbuffer.h>

#include "windowboardpainter.hpp"

enum {
  TIMER_ID = wxID_HIGHEST + 100
};

class WindowBoardPainterImpl :
  public WindowBoardPainter,
  public wxEvtHandler,
  public BoardObserver{
  wxTimer m_timer;
  int animation_flag;
  ThemeManager *theme;
  Game *game;
  wxWindow *w;

  CHESS_GRID board[64];
  int activegrid;
  wxImage *activeimg;
  int gamespeed;

  int margin_x, margin_y;
  int grid_size;

public:
  WindowBoardPainterImpl(wxWindow *_window_board, ThemeManager *_theme) :
    m_timer(this, TIMER_ID),
    animation_flag(0),
    game(NULL),
    w(_window_board),
    activegrid(-1){
    for(int i = 0; i < 64; ++i)board[i] = NONE;

    UpdateTheme(_theme);
    w->PushEventHandler(this);
    m_timer.Start(50);
  };

  ~WindowBoardPainterImpl(){
    w->PopEventHandler();
  }

  virtual void UpdateTheme(ThemeManager *_theme){
    theme = _theme;
    margin_x = theme->get_margin_x();
    margin_y = theme->get_margin_y();
    grid_size = theme->get_black_chess(0).GetWidth();
  }

  virtual void SetGame(Game *g){
    game = g;
    game->board_attach_observer(this);
    w->Refresh();
  }

  virtual void ClearGame(){
    game->board_detach_observer(this);
    game = NULL;
    w->Refresh();
    animation_flag = 0;
  }

  virtual void SetSpeed(int i){
    gamespeed = i;
  }

public:
  void UpdateBoard(){
    w->Refresh();
  }

  void AnimationBoard(int i){
    w->Refresh();
    animation_flag = 1;
    wxMilliSleep(gamespeed/i);
  }

  void OnPaint(wxPaintEvent &WXUNUSED(event)){
    wxBufferedPaintDC dc(w);
    dc.DrawBitmap(theme->get_background(), 0, 0);
    if(game){
      for(int y = 0; y < 8; ++y){
        for(int x = 0; x < 8; ++x){
          CHESS_SIDE side = board[Board::xy2flat(x, y)];
          if(side == BLACK){
            wxImage &img = Board::xy2flat(x, y) == activegrid?
              *activeimg:theme->get_black_chess(0);
            dc.DrawBitmap(img, grid_size*x+margin_x, grid_size*y+margin_y, true);
          }else if(side == WHITE){
            wxImage &img = Board::xy2flat(x, y) == activegrid?
              *activeimg:theme->get_white_chess(0);
            dc.DrawBitmap(img, grid_size*x+margin_x, grid_size*y+margin_y, true);
          }
        }
      }
    }
  }

  void OnTimer(wxTimerEvent &WXUNUSED(event)){
    if(animation_flag){
      w->Refresh();
      animation_flag = 0;
    }
  }

public:
  virtual void board_prepare_begin(){
    for(int i = 0; i < 64; ++i)board[i] = NONE;
    UpdateBoard();
  }

  virtual void board_prepare_end(){
  }

  virtual void board_grid_set(int x, int y, CHESS_SIDE side){
    board[Board::xy2flat(x, y)] = side;
    AnimationBoard(1);
  }

  virtual void board_grid_turn(int x, int y, CHESS_SIDE side){
    activegrid = Board::xy2flat(x, y);
    CHESS_SIDE cs[] = {Board::getotherside(side), side};
    for(int j = 0; j < 2; ++j){
      CHESS_SIDE &s = cs[j];
      for(int i = 0; i < int(theme->get_chess_size(s)); ++i){
        activeimg = &theme->get_chess(s, j == 0?i:(theme->get_chess_size(s)-i-1));
        AnimationBoard(theme->get_white_chess_size() + theme->get_black_chess_size());
      }
    }
    activegrid = -1;
    board[Board::xy2flat(x, y)] = side;
  }

protected:
  DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(WindowBoardPainterImpl, wxEvtHandler)
EVT_PAINT(WindowBoardPainterImpl::OnPaint)
EVT_TIMER(TIMER_ID, WindowBoardPainterImpl::OnTimer)
END_EVENT_TABLE();


WindowBoardPainter* WindowBoardPainter::Create(wxWindow *_window_board, ThemeManager *t){
  return new WindowBoardPainterImpl(_window_board, t);
}
