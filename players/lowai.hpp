/**
 * @file     player/lowai.hpp
 * @summary  player with extremely low ai
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: lowai.hpp,v 1.3 2007/06/20 03:44:45 hellwolfmisty Exp $
 */

#ifndef PLAYER_LOWAI_H
#define PLAYER_LOWAI_H

#include "othello/player.hpp"

namespace othello{
  class PlayerLowai:public Player{
  protected:
    virtual void do_get_turn(Board &);
  public:
    PlayerLowai(){}
  };
}

#endif

