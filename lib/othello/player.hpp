/**
 * @file     lib/othello/player.hpp
 * @summary  player abstract class
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player.hpp,v 1.2 2007/06/20 03:44:45 hellwolfmisty Exp $
 */

#ifndef LIB_OTHELLO_PLAYER_HPP
#define LIB_OTHELLO_PLAYER_HPP

#include "observer.hpp"

#include "board.hpp"

namespace othello{
  class Player;

  class PlayerObserver : public Observer{
  public:
    virtual void player_ready(Player *) = 0;
    virtual void player_go(Player *, int /*x*/, int /*y*/) = 0;
    virtual void player_pass(Player *) = 0;
    virtual void player_quit(Player *) = 0;
  };

  class Player : public Subject<PlayerObserver>{
    int isready;
    CHESS_SIDE myside;

  public:
    Player();
    virtual ~Player();

  public:
    inline int is_ready(){return isready;}
    inline CHESS_SIDE get_myside(){return myside;}

    /* called by inherited player type (to inform owner) */
  public:
    void ready();
    void go(int x, int y);
    void pass();
    void quit_game();

    /* called by player owner (to inform player) */
  public:
    void set_myside(CHESS_SIDE);
    void prepare();
    void get_turn(Board board);
    void end_game();

  protected:
    virtual void do_set_myside(CHESS_SIDE){}
    virtual void do_prepare(){ready();}
    virtual void do_get_turn(Board &){};
    virtual void do_end_game(){};
  };
}

#endif

