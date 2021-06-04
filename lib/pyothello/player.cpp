/**
 * @file     lib/pyothello/player.cpp
 * @summary  othello::Player python wrapper
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player.cpp,v 1.3 2007/06/20 14:46:46 hellwolfmisty Exp $
 */

#include <stdexcept>
#include <boost/python.hpp>

#include "utility.hpp"
#include "player.hpp"

namespace othello{
  pyPlayer::pyPlayer() : player(new _Player(this)){
  }

  pyPlayer::pyPlayer(const pyPlayer& pyplayer){
    player = pyplayer.player;
  }

  pyPlayer::~pyPlayer(){
  }

  class heldPlayer : public pyPlayer{
    PyObject *self;

  public:
    heldPlayer(PyObject* _self):self(_self){}

  public:
    virtual void do_set_myside(CHESS_SIDE myside){
      try{
        python::call_method<void>(self, "do_set_myside", myside);
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    void do_set_myside_default(CHESS_SIDE myside){
      player->do_set_myside_default(myside);
    }
    virtual void do_prepare(){
      try{
        python::call_method<void>(self, "do_prepare");
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    void do_prepare_default(){
      player->do_prepare_default();
    }
    virtual void do_get_turn(Board &board){
      try{
        python::call_method<void>(self, "do_get_turn", pyBoard(board));
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    void do_get_turn_default(Board &board){
      player->do_get_turn_default(board);
    }
    virtual void do_end_game(){
      try{
        python::call_method<void>(self, "do_end_game");
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    void do_end_game_default(){
      player->do_end_game_default();
    }
  };

  void wrapper_Player(){
    python::class_<pyPlayer, heldPlayer, boost::noncopyable>
      ("Player")
      .def("is_ready", &pyPlayer::is_ready)
      .def("get_myside", &pyPlayer::get_myside)
      .def("ready", &pyPlayer::ready)
      .def("go", &pyPlayer::go)
      .def("pass", &pyPlayer::pass)
      .def("quit_game", &pyPlayer::quit_game)
      .def("do_set_myside", &heldPlayer::do_set_myside_default)
      .def("do_prepare", &heldPlayer::do_prepare_default)
      .def("do_get_turn", &heldPlayer::do_get_turn_default)
      .def("do_end_game", &heldPlayer::do_end_game_default)
      ;
  }
}

