/**
 * @file     lib/pyothello/board.hpp
 * @summary  othello::Board python wrapper
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: board.hpp,v 1.3 2009/07/05 19:30:56 hellwolfmisty Exp $
 */


#ifndef LIB_PYOTHELLO_BOARD_HPP
#define LIB_PYOTHELLO_BOARD_HPP

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

#include "othello/board.hpp"

using namespace boost;

namespace othello{
  class pyBoard{
    shared_ptr<Board> board;

  public:
    /* in python constructor */
    pyBoard();
    /* exposed C++ python object to python object conversions */
    pyBoard(const pyBoard&);
    /* inexposed c++ object to python object */
    pyBoard(Board&);

    pyBoard copy();

    inline shared_ptr<Board> get_board(){
      return board;
    }

    /* const functions */
  public:
    inline int gettotalnum()const{
      return board->gettotalnum();
    }
    inline int getnum(CHESS_GRID side)const{
      return board->getnum(side);
    }
    inline CHESS_GRID getgrid(int x, int y)const{
      return board->getgrid(x, y);
    }
    inline CHESS_SIDE whose_turn()const{
      return board->whose_turn();
    }
    inline int getstep()const{
      return board->getstep();
    }
    inline int vital_grids(CHESS_SIDE side)const{
      return board->vital_grids(side);
    }

  public:
    Board::GO_RESULT try_go(int x, int y, CHESS_SIDE myside,
                            bool continue_when_ok,
                            python::list &pylist);
    inline void reset(){
      board->reset();
    }
    inline void prepare(){
      board->prepare();
    }
    void set(python::list, CHESS_SIDE nextside, int stepnow);
    inline Board::GO_RESULT go(int x, int y){
      return board->go(x, y);
    }
    inline void pass(){
      board->pass();
    }
    inline void attach_observer(shared_ptr<BoardObserver> obs){
      board->attach_observer(obs.get());
    }
    inline void detach_observer(shared_ptr<BoardObserver> obs){
      board->detach_observer(obs.get());
    }
  };

  void wrapper_Board();
}

#endif

