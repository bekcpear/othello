/**
 * @file     lib/othello/board.cpp
 * @summary  play board class implementation
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: board.cpp,v 1.4 2008/07/26 16:28:44 hellwolfmisty Exp $
 */

#include <string.h>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "board.hpp"

namespace othello{
  Board::Board(){
  }

  Board::Board(const Board &board)
    : Subject<BoardObserver>(){
    memcpy(b, board.b, sizeof(b));
    current_side = board.current_side;
    white_num = board.white_num;
    black_num = board.black_num;
    step = board.step;
  }

  void Board::reset(){
    for(int	i=0; i < 64; i++)b[i]=NONE;
    white_num = black_num = 0;
    step = 0;
  }

  Board::GO_RESULT Board::try_go(int x, int y,
                                 CHESS_SIDE myside,
                                 bool continue_when_ok,
                                 std::vector<int> *plist)const{
    int rt = 0;
    int p = xy2flat(x, y);
    if(p >= 64 || p < 0)return GO_INVAL;
    int i;
    CHESS_SIDE otherside = getotherside(myside);

    if(b[p] != NONE)return GO_GRID_OCCUPIED;

    if((p-8) > -1 && b[p-8] == otherside){
      for(i = p-16; i > -1 && b[i] == otherside; i -= 8){};
      if(i > -1 && b[i] == myside){
        if(continue_when_ok){
          for(int j = p-8; j > i; j -= 8){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //downward search
    if((p+8) < 56 && b[p+8] == otherside){
      for(i = p+16; i < 64 && b[i] == otherside; i += 8){};
      if(i < 64 && b[i] == myside){
        if(continue_when_ok){
          for(int j = p+8; j < i; j+=8){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //leftward search
    if(p%8 > 1 && b[p-1] == otherside){
      for(i = p-2; i >= int((p/8)*8) &&  b[i] == otherside; --i){};
      if(i >= int((p/8)*8) && b[i] == myside){
        if(continue_when_ok){
          for(int j = p-1; j > i; --j){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //rightward search
    if(p%8 < 6 && p+1 < 64 && b[p+1] == otherside){
      for(i= p+2; i < int(((p/8)+1)*8)  && b[i] == otherside; ++i){};
      if(i < int(((p/8)+1)*8) && b[i] == myside){
        if(continue_when_ok){
          for(int j=p+1; j < i; ++j){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //downleftward search
    if(p%8 > 1 && p+7 < 64 && b[p+7] == otherside){
      for(i = p+14; i < 64 && i%8 < p%8 && b[i] == otherside; i += 7){};
      if(i < 64 && i%8 < p%8  && b[i] == myside){
        if(continue_when_ok){
          for(int j = p+7; j < i; j += 7){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //downrightward search
    if(p%8 < 6 && p+9 < 64 && b[p+9] == otherside){
      for(i = p+18; i < 64 && i%8 > p%8 && b[i] == otherside; i += 9){};
      if(i < 64 && i%8 > p%8 && b[i] == myside){
        if(continue_when_ok){
          for(int j = p+9; j < i; j += 9){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //uprightward search
    if(p%8 < 6 && p-7 >= 0 && b[p-7] == otherside){
      for(i = p-14; i > -1 && i%8 > p%8 && b[i] == otherside; i -= 7){};
      if(i > -1 && i%8 > p%8 && b[i] == myside){
        if(continue_when_ok){
          for(int j = p-7; j > i;j -= 7){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }
    //upleftward search
    if(p%8 > 1 && p-9 >= 0 && b[p-9] == otherside){
      for(i= p-18; i > -1 && i%8 < p%8 &&  b[i] == otherside; i -= 9){};
      if(i > -1 && i%8 < p%8 && b[i] == myside){
        if(continue_when_ok){
          for(int j = p-9; j > i;j -= 9){
            ++rt;
            if(plist)plist->push_back(j);
          }
        }else return GO_OK;
      }
    }

    return rt == 0?GO_NO_GRID_TO_TURN:GO_OK;
  }

  int Board::vital_grids(CHESS_SIDE c)const{
    int rt = 0;
    for(int i=0; i < 64; i++)
      if(b[i] == NONE)
        if(try_go(flat2x(i), flat2y(i), c) == GO_OK)++rt;
    return rt;
  }

  void Board::set(CHESS_GRID *g, CHESS_SIDE nextside, int stepnow){
    black_num = 0;
    white_num = 0;
    for(int	i=0; i < 64; i++)b[i]=NONE;

    for(int i = 0; i < 64; ++i){
      if(g[i] == NONE)continue;
      int x = flat2x(i);
      int y = flat2y(i);
      b[i] = g[i];
      BEGIN_OBSERVERS_FOREACH(ob, observers)
        (*ob)->board_grid_set(x, y, b[i]);
      END_OBSERVERS_FOREACH;
      incside(b[i]);
    }
    current_side = nextside;
    step = stepnow;
  }

#define X BLACK
#define O WHITE
#define _ NONE
  static CHESS_GRID default_board[] = {
    _,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,
    _,_,_,O,X,_,_,_,
    _,_,_,X,O,_,_,_,
    _,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,
    _,_,_,_,_,_,_,_,
  };
#undef X
#undef O
#undef _

  void Board::prepare(){
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->board_prepare_begin();
    END_OBSERVERS_FOREACH;
    set(default_board, BLACK, 1);
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->board_prepare_end();
    END_OBSERVERS_FOREACH;
  }

  Board::GO_RESULT Board::go(int x, int y){
    int p = xy2flat(x, y);
    if(p >= 64 || p < 0)return GO_INVAL;
    std::vector<int> tlist;//grids will be turned

    if(b[p] != NONE){
      return GO_GRID_OCCUPIED;
    }

    GO_RESULT r = try_go(flat2x(p), flat2y(p), current_side, true, &tlist);
    if(r != GO_OK){
      return r;
    }

    b[p] = current_side;
    incside(current_side);
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->board_grid_set(x, y, current_side);
    END_OBSERVERS_FOREACH;
    for(std::vector<int>::iterator iter = tlist.begin();
        iter != tlist.end();
        ++iter){
      b[*iter] = current_side;
      incside(current_side);
      decside(getotherside(current_side));
      BEGIN_OBSERVERS_FOREACH(ob, observers)
        (*ob)->board_grid_turn(flat2x(*iter), flat2y(*iter), current_side);
      END_OBSERVERS_FOREACH;
    }

    ++step;
    current_side = getotherside(current_side);
    return GO_OK;
  }

  void Board::pass(){
    ++step;
    current_side = getotherside(current_side);
  }

}//namespace othello

