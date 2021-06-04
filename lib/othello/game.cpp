/**
 * @file     lib/othello/game.cpp
 * @summary  othello game
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: game.cpp,v 1.3 2008/07/26 16:28:48 hellwolfmisty Exp $
 */

#include <stdexcept>
#include <algorithm>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include "game.hpp"

namespace othello{
  enum STATE{
    START,
    WAIT_PLAYERS,
    GAME_READY,
    PLAYERS_PREPARE,
    PLAYERS_READY,
    IN_TURN,
    GAMEOVER,
    STOP,
  };

#define BEGIN_STATE_OBSERVERS_FOREACH(ob, self)                               \
  BEGIN_OBSERVERS_FOREACH(ob, (self)->Subject<GameStateObserver>::observers)

#define BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, self)                             \
  BEGIN_OBSERVERS_FOREACH(ob, (self)->Subject<GameAuxinfoObserver>::observers)

  class GameImpl :
    public PlayerObserver{
  public:
    CHESS_GRID  grids_saved[64];
    CHESS_SIDE  whose_turn_saved;
    int step_saved;

    Game *game;
    STATE state;
    boost::mutex state_mutex;
    boost::mutex::scoped_lock game_lock;
    boost::condition state_cond;

  public:
    GameImpl(Game *g):game_lock(state_mutex){
      game_lock.unlock();
      game = g;
      state = START;
    }
    ~GameImpl(){
      boost::mutex::scoped_lock lock(state_mutex);
      if(state != STOP){
        state = STOP;
        state_cond.notify_all();
      }
    }
    inline int state_is_not_wait_players(){return state != WAIT_PLAYERS;}
    inline int state_is_not_in_turn(){return state != IN_TURN;}
    inline int state_is_not_prepare(){return state != PLAYERS_PREPARE;}

  public:
    void save_board_snapshot(){
      for(int i = 0; i < 64; ++i)
        grids_saved[i] = game->get_board()->getgrid(Board::flat2x(i), Board::flat2y(i));
      whose_turn_saved = game->get_board()->whose_turn();
      step_saved = game->get_board()->getstep();
    }
    void stop_or_restart(STATE what_state){
      boost::mutex::scoped_lock lock(state_mutex);
      if(state == what_state){
        return;
      }
      if(state == PLAYERS_READY ||
         state == IN_TURN){
        lock.unlock();
        game->black->end_game();
        game->white->end_game();
        BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
          (*ob)->game_auxinfo_force_stop();
        END_OBSERVERS_FOREACH;
        lock.lock();
      }
      if(what_state == START){
        BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
          (*ob)->game_state_restart();
        END_OBSERVERS_FOREACH;
      }else{
        BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
          (*ob)->game_state_stop();
        END_OBSERVERS_FOREACH;
      }
      game->board.reset();
      state = what_state;
      state_cond.notify_all();
      return;
    }
    void player_make_foul(Player* player, const std::string &reason){
      BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
        (*ob)->game_state_player_foul(player->get_myside(), reason);
      END_OBSERVERS_FOREACH;
      game->restart();
      game->detach_player(player->get_myside());
    }
  public:
    virtual void player_ready(Player *player){
      boost::mutex::scoped_lock lock(state_mutex);
      if(state != PLAYERS_PREPARE){
        lock.unlock();
        player_make_foul(player, std::string("player call ready when state is not PLAYERS_PREPARE"));
        return;
      }
      BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
        (*ob)->game_state_player_ready(player->get_myside());
      END_OBSERVERS_FOREACH;
      if(game->is_all_players_ready()){
        state = PLAYERS_READY;
        BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
          (*ob)->game_auxinfo_start();
        END_OBSERVERS_FOREACH;
        state_cond.notify_all();
      }
    }

    virtual void player_go(Player *player, int x, int y){
      boost::mutex::scoped_lock lock(state_mutex);
      //somebody quit
      if(state == START || state == STOP)return;
      Board *board = &game->board;
      CHESS_SIDE side = player->get_myside();
      if(state != IN_TURN){
        lock.unlock();
        player_make_foul(player, std::string("player call go when state is not IN_TURN"));
        return;
      }
      if(board->whose_turn() != side){
        lock.unlock();
        player_make_foul(player, std::string("player call go, but it's not her turn"));
        return;
      }
      if(board->try_go(x, y, side) != Board::GO_OK){
        lock.unlock();
        player_make_foul(player, std::string("player go not ok"));
        return;
      }

      lock.unlock();
      board->go(x, y);
      lock.lock();

      save_board_snapshot();
      if(state != IN_TURN)return;

      lock.unlock();
      BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
        (*ob)->game_state_player_go(side, x, y);
      END_OBSERVERS_FOREACH;
      lock.lock();

      if(state != IN_TURN)return;
      int vb = board->vital_grids(BLACK);
      int vw = board->vital_grids(WHITE);
      int white_num = board->getnum(WHITE);
      int black_num = board->getnum(BLACK);
      if(white_num + black_num == 64 || (vb == 0 && vw == 0)){
        //game over
        state = GAMEOVER;
        lock.unlock();
        if(white_num + black_num < 64){
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_no_vital_grids_for(board->getotherside(side));
          END_OBSERVERS_FOREACH;
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_no_vital_grids_for(side);
          END_OBSERVERS_FOREACH;
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_ahead_over();
          END_OBSERVERS_FOREACH;
        }
        if(black_num > white_num){
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_black_win();
          END_OBSERVERS_FOREACH;
        }else if(black_num == white_num){
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_draw();
          END_OBSERVERS_FOREACH;
        }else{
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_white_win();
          END_OBSERVERS_FOREACH;
        }
        BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
          (*ob)->game_auxinfo_gameover();
        END_OBSERVERS_FOREACH;
        game->black->end_game();
        game->white->end_game();
        lock.lock();
      }else{
        if(!((side == BLACK && vw > 0) ||
             (side == WHITE && vb > 0))){
          lock.unlock();
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_no_vital_grids_for(board->getotherside(side));
          END_OBSERVERS_FOREACH;
          //other side has no grid to go, we help him to pass
          board->pass();
          BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, game)
            (*ob)->game_auxinfo_player_force_pass(board->getotherside(side));
          END_OBSERVERS_FOREACH;
          lock.lock();
        }
        if(state != IN_TURN)return;
        state = PLAYERS_READY;
      }
      state_cond.notify_all();
    }

    virtual void player_pass(Player *player){
      boost::mutex::scoped_lock lock(state_mutex);
      if(state == STOP)return;
      if(state != IN_TURN){
        lock.unlock();
        player_make_foul(player, std::string("GameImpl::player_pass called when state is not IN_TURN"));
        return;
      }
      Board *board = &game->board;
      CHESS_SIDE side = player->get_myside();
      if(board->whose_turn() != side){
        lock.unlock();
        player_make_foul(player, std::string("player call pass, but it's not her turn"));
        return;
      }

      lock.unlock();
      board->pass();
      lock.lock();

      save_board_snapshot();
      if(state != IN_TURN)return;

      lock.unlock();
      BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
        (*ob)->game_state_player_pass(side);
      END_OBSERVERS_FOREACH;
      lock.lock();

      if(state != IN_TURN)return;
      state = PLAYERS_READY;
      state_cond.notify_all();
    }

    virtual void player_quit(Player *player){
      BEGIN_STATE_OBSERVERS_FOREACH(ob, game)
        (*ob)->game_state_player_quit(player->get_myside());
      END_OBSERVERS_FOREACH;
      game->restart();
      game->detach_player(player->get_myside());
    }
  };

  Game::Game(){
    impl = new GameImpl(this);
    black = white = NULL;
    board.reset();
    impl->save_board_snapshot();
  }

  Game::~Game(){
    delete impl;
  }

  int Game::attach_player(CHESS_SIDE side, Player *p){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    if((side == BLACK?black:white) != NULL ||
       (impl->state != WAIT_PLAYERS && impl->state != START)){
      return 0;
    }
    if(side == BLACK){
      black = p;
    }else{
      white = p;
    }
    p->set_myside(side);
    p->attach_observer(impl);
    lock.unlock();
    BEGIN_STATE_OBSERVERS_FOREACH(ob, this)
      (*ob)->game_state_player_join(side);
    END_OBSERVERS_FOREACH;
    lock.lock();
    if(has_enough_players()){
      impl->state = GAME_READY;
      impl->state_cond.notify_all();
    }
    return 1;
  }

  int Game::detach_player(CHESS_SIDE side){
    Player **pp = &(side == BLACK?black:white);
    boost::mutex::scoped_lock lock(impl->state_mutex);
    if(*pp != NULL){
      (*pp)->detach_observer(impl);
      (*pp) = NULL;
      lock.unlock();
      BEGIN_STATE_OBSERVERS_FOREACH(ob, this)
        (*ob)->game_state_player_detached(side);
      END_OBSERVERS_FOREACH;
      return 1;
    }
    return 0;
  }

  int Game::async_wait_players(){
    BEGIN_STATE_OBSERVERS_FOREACH(ob, this)
      (*ob)->game_state_wait_players();
    END_OBSERVERS_FOREACH;
    boost::mutex::scoped_lock lock(impl->state_mutex);
    if(impl->state != START &&
       impl->state != GAME_READY &&
       impl->state != PLAYERS_READY ){
      return 0;
    }
    if(impl->state == PLAYERS_READY){
      return 1;
    }
    if(has_enough_players()){
      impl->state = GAME_READY;
      return 1;
    }
    impl->state = WAIT_PLAYERS;
    return 1;
  }

  int Game::wait_players(){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    while(1){
      lock.unlock();
      if(async_wait_players() == 0)return 0;
      lock.lock();
      impl->state_cond.wait(lock, boost::bind(&GameImpl::state_is_not_wait_players, impl));
      if(impl->state == STOP){
        return 0;
      }else if(impl->state == GAME_READY ||
               impl->state == PLAYERS_READY){
        return 1;
      }
    }
  }

  int Game::async_prepare(CHESS_GRID *g, CHESS_SIDE nextside, int stepnow){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    if(impl->state != GAME_READY && impl->state != PLAYERS_READY){
      return 0;
    }
    if(impl->state == PLAYERS_READY){
      return 1;
    }
    if(g){
      board.set(g, nextside, stepnow);
    }else{
      board.prepare();
    }
    impl->state = PLAYERS_PREPARE;
    impl->save_board_snapshot();
    lock.unlock();
    BEGIN_STATE_OBSERVERS_FOREACH(ob, this)
      (*ob)->game_state_wait_players_ready();
    END_OBSERVERS_FOREACH;
    black->prepare();
    lock.lock();
    /* black quit */
    if(impl->state != PLAYERS_PREPARE)return 0;
    lock.unlock();
    white->prepare();
    return 1;
  }

  int Game::prepare(CHESS_GRID *g, CHESS_SIDE nextside, int stepnow){
    if(async_prepare(g, nextside, stepnow) == 0)return 0;
    boost::mutex::scoped_lock lock(impl->state_mutex);
    impl->state_cond.wait(lock, boost::bind(&GameImpl::state_is_not_prepare, impl));
    return impl->state == PLAYERS_READY;
  }

  int Game::is_rollable(){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    return impl->state == PLAYERS_READY;
  }

  int Game::async_roll(){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    if(impl->state != PLAYERS_READY){
      return 0;
    }
    impl->state = IN_TURN;
    impl->save_board_snapshot();
    lock.unlock();
    BEGIN_STATE_OBSERVERS_FOREACH(ob, this)
      (*ob)->game_state_roll();
    END_OBSERVERS_FOREACH;
    if(is_stopped())return 0;
    BEGIN_AUXINFO_OBSERVERS_FOREACH(ob, this)
      (*ob)->game_auxinfo_turn_of(board.whose_turn());
    END_OBSERVERS_FOREACH;
    if(is_stopped())return 0;
    if(board.whose_turn() == BLACK)
      black->get_turn(board);
    else
      white->get_turn(board);
    return 1;
  }

  int Game::roll(){
    if(async_roll() == 0)return 0;
    boost::mutex::scoped_lock lock(impl->state_mutex);
    impl->state_cond.wait(lock, boost::bind(&GameImpl::state_is_not_in_turn, impl));
    return impl->state == PLAYERS_READY;
  }

  int Game::is_gameover(){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    return impl->state == GAMEOVER;
  }

  int Game::exchange(){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    if(impl->state != GAMEOVER){
      return 0;
    }
    if(!has_enough_players()){
      return 0;
    }
    lock.unlock();
    black->set_myside(Board::getotherside(black->get_myside()));
    white->set_myside(Board::getotherside(white->get_myside()));
    std::swap(black, white);
    BEGIN_STATE_OBSERVERS_FOREACH(ob, this)
      (*ob)->game_state_players_exchange();
    END_OBSERVERS_FOREACH;
    return 1;
  }

  void Game::stop(){
    return impl->stop_or_restart(STOP);
  }

  void Game::restart(){
    return impl->stop_or_restart(START);
  }

  int Game::is_stopped(){
    return impl->state == STOP;
  }

  void Game::wait_stop(){
    boost::mutex::scoped_lock lock(impl->state_mutex);
    impl->state_cond.wait(lock, boost::bind(&Game::is_stopped, this));
  }

  void Game::lock(){
    impl->game_lock.lock();
  }

  void Game::unlock(){
    impl->game_lock.unlock();
  }


  void Game::get_board_snapshot(CHESS_GRID *grids, CHESS_SIDE &whose_turn, int &step){
    for(int i = 0; i < 64; ++i)grids[i] = impl->grids_saved[i];
    whose_turn = impl->whose_turn_saved;
    step = impl->step_saved;
  }


  /* 
   * Single Machine Game Creator
   */
  SingleMachineGame::SingleMachineGame(){
  }

  int SingleMachineGame::is_ok(){
    return 1;
  }

  std::string SingleMachineGame::get_last_error(){
    return "";
  }
}

