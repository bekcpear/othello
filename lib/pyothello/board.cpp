/**
 * @file     lib/pyothello/board.cpp
 * @summary  othello::Board python wrapper
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: board.cpp,v 1.3 2007/06/20 14:46:46 hellwolfmisty Exp $
 */

#include <stdexcept>

#include "utility.hpp"
#include "board.hpp"

namespace othello{
  pyBoard::pyBoard() : board(new Board()){
  }

  pyBoard::pyBoard(const pyBoard& pyboard){
    board = pyboard.board;
  }

  pyBoard::pyBoard(Board &b) : board(new Board(b)){
  }

  pyBoard pyBoard::copy(){
    return *board;
  }

  void pyBoard::set(python::list lst, CHESS_SIDE nextside, int stepnow){
    if(PyList_Size(lst.ptr()) == 64){
      CHESS_GRID grids[64];
      for(int i = 0; i < 64; ++i){
        python::extract<CHESS_GRID> g(lst[i]);
        if(!g.check()){
      throw std::runtime_error("pyBoard::set invalid argument lst, "
                               "all element should be type CHESS_GRID");
          return;
        }
        grids[i] = g();
      }
      board->set(grids, nextside, stepnow);
    }else{
      throw std::runtime_error("pyBoard::set invalid argument lst, "
                               "invalid size, should be 64");
    }
  }

  Board::GO_RESULT pyBoard::try_go(int x, int y, CHESS_SIDE myside,
                                   bool continue_when_ok,
                                   python::list &pylist){
    Board::GO_RESULT result;
    std::vector<int> list;
    result = board->try_go(x, y, myside, continue_when_ok, &list);
    if(continue_when_ok){
      for(std::vector<int>::iterator iter = list.begin();
          iter != list.end();
          ++iter)pylist.append(*iter);
    }
    return result;
  }

  class heldBoardObserver : public BoardObserver{
    PyObject *self;

  public:
    heldBoardObserver(PyObject* _self):self(_self){}
  public:
    virtual void board_prepare_begin(){
      try{
        python::call_method<void>(self, "board_prepare_begin");
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    virtual void board_prepare_end(){
      try{
        python::call_method<void>(self, "board_prepare_end");
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    virtual void board_grid_set(int x, int y, CHESS_SIDE side){
      try{
        python::call_method<void>(self, "board_grid_set", x, y, side);
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
    virtual void board_grid_turn(int x, int y, CHESS_SIDE newside){
      try{
        python::call_method<void>(self, "board_grid_turn", x, y, newside);
      }catch(...){
        python::handle_exception();
        throw std::runtime_error(python_get_traceback_string());
      }
    }
  };

  void wrapper_Board(){
    python::enum_<CHESS_GRID>("CHESS_GRID")
      .value("NONE", NONE)
      .value("BLACK", BLACK)
      .value("WHITE", WHITE)
      ;

    python::enum_<Board::GO_RESULT>("GO_RESULT")
      .value("GO_OK", Board::GO_OK)
      .value("GO_GRID_OCCUPIED", Board::GO_GRID_OCCUPIED)
      .value("GO_NO_GRID_TO_TURN", Board::GO_NO_GRID_TO_TURN)
      .value("GO_INVAL", Board::GO_INVAL)
      ;

    python::class_<pyBoard>
      ("Board")
      .def("copy", &pyBoard::copy)
      .def("xy2flat", &Board::xy2flat).staticmethod("xy2flat")
      .def("flat2x", &Board::flat2x).staticmethod("flat2x")
      .def("flat2y", &Board::flat2y).staticmethod("flat2y")
      .def("getotherside", &Board::getotherside).staticmethod("getotherside")
      .def("gettotalnum", &pyBoard::gettotalnum)
      .def("getnum", &pyBoard::getnum)
      .def("getgrid", &pyBoard::getgrid)
      .def("whose_turn", &pyBoard::whose_turn)
      .def("getstep", &pyBoard::getstep)
      .def("vital_grids", &pyBoard::vital_grids)
      .def("try_go", &pyBoard::try_go)
      .def("reset", &pyBoard::reset)
      .def("prepare", &pyBoard::prepare)
      .def("set", &pyBoard::set)
      .def("go", &pyBoard::go)
      .def("pass", &pyBoard::pass)
      .def("attach_observer", &pyBoard::attach_observer)
      .def("detach_observer", &pyBoard::detach_observer)
      ;

    python::class_<BoardObserver, heldBoardObserver, boost::noncopyable>
      ("BoardObserver")
      ;
  }
};

