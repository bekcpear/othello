/**
 * @file     lib/othello/player.cpp
 * @summary  player abstract class
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player.cpp,v 1.2 2007/06/20 03:44:45 hellwolfmisty Exp $
 */

#include <stdexcept>

#include "player.hpp"

namespace othello{
  Player::Player(){
    isready = 0;
  }

  Player::~Player(){
  }

  /* called by inherited player type (to inform owner) */
  void Player::ready(){
    isready = 1;
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->player_ready(this);
    END_OBSERVERS_FOREACH;
  }

  void Player::go(int x, int y){
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->player_go(this, x, y);
    END_OBSERVERS_FOREACH;
  }

  void Player::pass(){
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->player_pass(this);
    END_OBSERVERS_FOREACH;
  }

  void Player::quit_game(){
    isready = 0;
    BEGIN_OBSERVERS_FOREACH(ob, observers)
      (*ob)->player_quit(this);
    END_OBSERVERS_FOREACH;
  }

  /* called by player owner (to inform player) */
  void Player::set_myside(CHESS_SIDE s){
    myside = s;
    do_set_myside(s);
  }

  void Player::prepare(){
    isready = 0;
    do_prepare();
  }

  void Player::get_turn(Board board){
    do_get_turn(board);
  }

  void Player::end_game(){
    isready = 0;
    do_end_game();
  }
}

