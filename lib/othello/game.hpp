/**
 * @file     lib/othello/game.hpp
 * @summary  othello game
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: game.hpp,v 1.3 2008/07/26 16:28:53 hellwolfmisty Exp $
 */

#ifndef LIB_OTHELLO_GAME_HPP
#define LIB_OTHELLO_GAME_HPP

#include <string>
#include <boost/utility.hpp>

#include "observer.hpp"
#include "board.hpp"
#include "player.hpp"

namespace othello{

  class GameStateObserver : public Observer{
  public:
    virtual void game_state_wait_players() = 0;
    virtual void game_state_player_join(CHESS_SIDE side) = 0;
    virtual void game_state_wait_players_ready() = 0;
    virtual void game_state_player_ready(CHESS_SIDE side) = 0;
    virtual void game_state_roll() = 0;
    virtual void game_state_player_foul(CHESS_SIDE side, const std::string &reason) = 0;
    virtual void game_state_players_exchange() = 0;
    virtual void game_state_player_go(CHESS_SIDE, int x, int y) = 0;
    virtual void game_state_player_pass(CHESS_SIDE side) = 0;
    virtual void game_state_player_quit(CHESS_SIDE side) = 0;
    virtual void game_state_restart() = 0;
    virtual void game_state_stop() = 0;
    virtual void game_state_player_detached(CHESS_SIDE side) = 0;
  };

  class GameAuxinfoObserver : public Observer{
  public:
    virtual void game_auxinfo_start() = 0;
    virtual void game_auxinfo_turn_of(CHESS_SIDE side) = 0;
    virtual void game_auxinfo_black_win() = 0;
    virtual void game_auxinfo_white_win() = 0;
    virtual void game_auxinfo_draw() = 0;
    virtual void game_auxinfo_ahead_over() = 0;
    virtual void game_auxinfo_gameover() = 0;
    virtual void game_auxinfo_force_stop() = 0;
    virtual void game_auxinfo_no_vital_grids_for(CHESS_SIDE side) = 0;
    virtual void game_auxinfo_player_force_pass(CHESS_SIDE side) = 0;
  };

  class GameImpl;

  class Game :
    public Subject<GameStateObserver>,
    public Subject<GameAuxinfoObserver>{
    friend class GameImpl;
  private:
    GameImpl *impl;
    Player *black;
    Player *white;
    Board board;

  public:
    inline const Board* get_board()const{
      return &board;
    }

    inline Player* get_white_player(){
      return white;
    }

    inline Player* get_black_player(){
      return black;
    }

  public:
    Game();
    ~Game();

    int attach_player(CHESS_SIDE, Player *);
    inline int attach_black_player(Player *p){return attach_player(BLACK, p);}
    inline int attach_white_player(Player *p){return attach_player(WHITE, p);}

    int detach_player(CHESS_SIDE);
    inline int detach_black_player(){return detach_player(BLACK);}
    inline int detach_white_player(){return detach_player(WHITE);}

    void board_attach_observer(BoardObserver *ob){
      board.attach_observer(ob);
    }
    void board_detach_observer(BoardObserver *ob){
      board.detach_observer(ob);
    }

  public:
    /* 
     * wait players if not has_enough_players
     * async mode : return 0 if game is stop or not restarted, else return 1
     * sync mode : return 1 if game is ready to prepare(has enough players)
     *             , else return 0
     */
    inline int has_enough_players(){
      return black != NULL && white != NULL;
    }
    int async_wait_players();
    int wait_players();

    /*
     * wait all players ready to game
     * async mode : return 0 if game is not ready to prepare(do not have 
     *              , enough players), else return 1
     * sync mode : return 1 if game is ready to roll, else return 0;
     */
    inline int is_all_players_ready(){
      return black->is_ready() && white->is_ready();
    }
    int async_prepare(CHESS_GRID *g = NULL, CHESS_SIDE nextside = BLACK, int stepnow = 0);
    int prepare(CHESS_GRID *g = NULL, CHESS_SIDE nextside = BLACK, int stepnow = 0);

    /*
     * roll the game until the game is not rollable(stop or restarted)
     * async mode : return 0 if not all players are ready to game, else return 1
     * sync mode : return 1 until next rollable round, return 0 if game is not
     *             rollable next round or this round
     */
    int is_rollable();
    int async_roll();
    int roll();

    /*
     * exhange player when game is over
     * return 1 on success, else return 0
     */
    int is_gameover();
    int exchange();

    void restart();

    void stop();
    void wait_stop();
    int is_stopped();

    void lock();

    void unlock();

    void get_board_snapshot(CHESS_GRID *, CHESS_SIDE &whose_turn, int &step);

  public:
    inline void loop(){
      while(!is_stopped()){
        if(wait_players() && prepare()){
          while(roll()){};
          if(is_stopped())break;
          exchange();
          restart();
        }
      }
    }

    inline void loop_noexchange(){
      while(!is_stopped()){
        if(wait_players() && prepare()){
          while(roll()){};
          if(is_stopped())break;
          restart();
        }
      }
    }
  };

  class GameCreator{
  protected:
    Game *game;
  public:
    GameCreator(){game = new Game();}
    virtual ~GameCreator(){delete game;}
  public:
    virtual int is_ok() = 0;
    virtual std::string get_last_error() = 0;
  public:
    Game* get_game(){return game;}
  };

  class SingleMachineGame : public GameCreator{
  public:
    SingleMachineGame();
  public:
    virtual int is_ok();
    virtual std::string get_last_error();
  };

}

#endif

