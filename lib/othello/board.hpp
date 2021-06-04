/**
 * @file     lib/othello/board.hpp
 * @summary  play board class
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: board.hpp,v 1.3 2007/06/20 03:44:45 hellwolfmisty Exp $
 */

#ifndef LIB_OTHELLO_BOARD_HPP
#define LIB_OTHELLO_BOARD_HPP

#include <vector>

#include "observer.hpp"

namespace othello{
  enum CHESS_GRID{NONE, BLACK, WHITE};
  typedef CHESS_GRID CHESS_SIDE;

  class BoardObserver : public Observer{
  public:
    virtual void board_prepare_begin() = 0;
    virtual void board_prepare_end() = 0;
    virtual void board_grid_set(int x, int y, CHESS_SIDE side) = 0;
    virtual void board_grid_turn(int x, int y, CHESS_SIDE newside) = 0;
  };

  class Board : public Subject<BoardObserver>{
  private:
    CHESS_GRID b[64];
    CHESS_SIDE current_side;
    int white_num;
    int black_num;
    int step;

  private:
    inline void incside(CHESS_SIDE side){
      if(side == BLACK)++black_num;
      if(side == WHITE)++white_num;
    }
    inline void decside(CHESS_SIDE side){
      if(side == BLACK)--black_num;
      if(side == WHITE)--white_num;
    }

  public:
    Board();

    Board(const Board&);

  public:
    static inline int xy2flat(int x, int y){
      return 8*y+x;
    }
    static inline int flat2x(int p){
      return p%8;
    }
    static inline int flat2y(int p){
      return p/8;
    }

    static inline CHESS_SIDE getotherside(CHESS_SIDE side){
      return side == BLACK?WHITE:BLACK;
    }

  public:
    inline int gettotalnum()const{
      return black_num+white_num;
    }

    inline int getnum(CHESS_GRID side)const{
      if(side == BLACK) return black_num;
      else if(side == WHITE) return white_num;
      else return 64 - black_num - white_num;
    }

    inline CHESS_GRID getgrid(int x, int y)const{
      return b[xy2flat(x, y)];
    }

    inline CHESS_SIDE whose_turn()const{
      return current_side;
    }

    inline int getstep()const{
      return step;
    }

    /* how many grids can someone to go remains */
    int vital_grids(CHESS_SIDE)const;

  public:
    /*
     * @args
     *  x,y      position to try
     *  myside   side to try
     *  continue_when_ok  continue to try if found the position is ok to go
     *  plist    the grids will be turned over
     * @return
     *  GO_RESULT
     */
    enum GO_RESULT{
      GO_OK = 0,
      GO_GRID_OCCUPIED,
      GO_NO_GRID_TO_TURN,
      GO_INVAL
    };
    GO_RESULT try_go(int x, int y,
                     CHESS_SIDE myside,
                     bool continue_when_ok = false,
                     std::vector<int> *plist = NULL)const;

    void reset();

    void set(CHESS_GRID *, CHESS_SIDE nextside, int stepnow);

    void prepare();

    GO_RESULT go(int x, int y);

    void pass();
  };
}

#endif

