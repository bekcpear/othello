/**
 * @file     player/lowai.cpp
 * @summary  player with extremely low ai
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: lowai.cpp,v 1.2 2007/06/20 03:44:45 hellwolfmisty Exp $
 */

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <functional>

#include "lowai.hpp"

namespace othello{
  using namespace std;

  struct eval_chess{
    int p;/*Chess's flat place*/
    int c;/*How many chessman can eat*/
    int s;/*score*/
    eval_chess(int pp, int cc):p(pp),c(cc){
      s = cc;
      if(p%8 == 0 || p%8 == 7)s*=2;
      if(p/8 == 0 || p/8 == 7)s*=2;
      if(p%8 == 1 || p%8 == 6 )s/=2;
      if(p/8 == 1 || p/8 == 6)s/=2;
    }
    bool operator < (const eval_chess& e)const{return s > e.s;}
    bool operator ==(const eval_chess& e)const{return s == e.s;}
  };

  void PlayerLowai::do_get_turn(Board &board){
    vector<eval_chess> activep;
    int i(64);
    while(i--){
      std::vector<int> a;
      if(board.try_go(Board::flat2x(i), Board::flat2y(i), get_myside(), true, &a) == Board::GO_OK)
        activep.push_back(eval_chess(i, a.size()));
    }
    sort(activep.begin(), activep.end());
    i = 0;
    while(i+1 < (int)activep.size() && activep[i] == activep[i+1])++i;
    random_shuffle(activep.begin(), activep.begin()+i);
    go(Board::flat2x(activep[0].p), Board::flat2y(activep[0].p));
  }
}

