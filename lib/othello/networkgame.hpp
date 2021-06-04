/**
 * @file     lib/othello/networkgame.hpp
 * @summary  othello network game
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: networkgame.hpp,v 1.3 2008/07/26 16:29:02 hellwolfmisty Exp $
 */


#ifndef LIB_OTHELLO_NETWORKGAME_HPP
#define LIB_OTHELLO_NETWORKGAME_HPP

#include <boost/utility.hpp>

#include "game.hpp"
#include "player.hpp"

namespace othello{
  #define othello_protocol_version "1"
  #define othello_default_tcpport 26417

  class NetworkGameServerImpl;
  class NetworkGameServer :
    public GameCreator,
    public boost::noncopyable{
    friend class NetworkGameServerImpl;
    NetworkGameServerImpl *impl;
    int tcpport;
  public:
    NetworkGameServer(int _tcpport);
    ~NetworkGameServer();
  public:
    virtual int is_ok();
    virtual std::string get_last_error();
  };

  class NetworkGameClientImpl;
  class NetworkGameClient :
    public GameCreator,
    public boost::noncopyable{
    friend class NetworkGameClientImpl;
    NetworkGameClientImpl *impl;
    char *destaddr;
    int tcpport;
  public:
    NetworkGameClient(const char *_destaddr, int _tcpport, Player *_black, Player *_white);
    ~NetworkGameClient();
  public:
    virtual int is_ok();
    virtual std::string get_last_error();
  };
}

#endif

