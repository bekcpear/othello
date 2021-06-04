/**
 * @file     lib/pyothello/player.hpp
 * @summary  othello::Player python wrapper
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player.hpp,v 1.3 2009/07/05 19:30:56 hellwolfmisty Exp $
 */

#ifndef LIB_PYOTHELLO_PLAYER_HPP
#define LIB_PYOTHELLO_PLAYER_HPP

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

#include "othello/player.hpp"
#include "board.hpp"

using namespace boost;

namespace othello{
  class pyPlayer{
    class _Player : public Player{
    private:
      pyPlayer *pyplayer;

    public:
      _Player(pyPlayer* _pyplayer) : pyplayer(_pyplayer){}

    public:
      virtual void do_set_myside(CHESS_SIDE side){
        pyplayer->do_set_myside(side);
      }
      virtual void do_prepare(){
        pyplayer->do_prepare();
      }
      virtual void do_get_turn(Board &board){
        pyplayer->do_get_turn(board);
      }
      virtual void do_end_game(){
        pyplayer->do_end_game();
      }

    public:
      void do_set_myside_default(CHESS_SIDE side){
        Player::do_set_myside(side);
      }
      void do_prepare_default(){
        Player::do_prepare();
      }
      void do_get_turn_default(Board &board){
        Player::do_get_turn(board);
      }
      void do_end_game_default(){
        Player::do_end_game();
      }
    };

  protected:
    shared_ptr<_Player> player;

  public:
    pyPlayer();
    pyPlayer(const pyPlayer&);
    virtual ~pyPlayer();

    inline shared_ptr<Player> get_player(){
      return player;
    }

  public:
    inline int is_ready(){
      return player->is_ready();
    }
    inline CHESS_SIDE get_myside(){
      return player->get_myside();
    }

  public:
    inline void ready(){
      player->ready();
    }
    inline void go(int x, int y){
      player->go(x, y);
    }
    inline void pass(){
      player->pass();
    }
    inline void quit_game(){
      player->quit_game();
    }

  public:
    virtual void do_set_myside(CHESS_SIDE side) = 0;
    virtual void do_prepare() = 0;
    virtual void do_get_turn(Board &) = 0;
    virtual void do_end_game() = 0;
  }; 

  void wrapper_Player();
}

#endif

