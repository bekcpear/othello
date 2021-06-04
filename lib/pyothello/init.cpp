/**
 * @file     lib/pyothello/init.cpp
 * @summary  othello lib python wrapper init
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: init.cpp,v 1.4 2009/07/05 19:30:56 hellwolfmisty Exp $
 */

#include <boost/python.hpp>

#include "board.hpp"
#include "player.hpp"
#include "intl.hpp"
#include "init.hpp"

namespace othello{
  void init_pyothello(){
    wrapper_Board();
    wrapper_Player();
    wrapper_Intl();
  }
}

