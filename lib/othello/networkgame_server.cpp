/**
 * @file     lib/othello/networkgame_server.cpp
 * @summary  othello network game server side code
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: networkgame_server.cpp,v 1.4 2009/07/05 05:17:59 hellwolfmisty Exp $
 */

#include <string>
#include <exception>
#include <streambuf>
#include <istream>
#include <iomanip>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "networkgame.hpp"

namespace othello{
  using namespace boost;
  using asio::ip::tcp;

  static char gridtbl[] = {'-', 'x', 'o'};
  static const char* grid2str[] = {
    "NONE",
    "BLACK",
    "WHITE"
  };

  static std::string itoa(int i){
    char b[20];
    snprintf(b, sizeof(b), "%d", i);
    return b;
  }

  class Session :
    public GameStateObserver,
    public boost::enable_shared_from_this<Session>{
    class NetworkPlayerDummy : public Player{
      friend class Session;
    protected:
      virtual void do_prepare(){}
    };

    system::error_code last_error;
    Game *game;
    NetworkPlayerDummy *black;
    NetworkPlayerDummy *white;
    std::string sendstr;
    asio::streambuf recvbuf;
    int session_end;

  private:
    std::string recvbuf2str(){
      std::istream is(&recvbuf);
      std::string line;
      std::getline(is, line);
      return line;
    }

  public:
    tcp::socket socket;
  private:
    Session(asio::io_service &io_service, Game *g):socket(io_service){
      session_end = 0;
      game = g;
      black = white = NULL;
    }
  public:
    ~Session(){
      game->Subject<GameStateObserver>::detach_observer(this);
      if(black){
        black->quit_game();
        delete black;
      }
      if(white){
        white->quit_game();
        delete white;
      }
    }
    static boost::shared_ptr<Session> create(asio::io_service &io_service, Game *g){
      return boost::shared_ptr<Session>(new Session(io_service, g));
    }

#define ASYNC_WRITE(str, fun)                                           \
    sendstr = str;                                                      \
        asio::async_write(socket, asio::buffer(sendstr),                \
                          boost::bind(&Session::fun, shared_from_this(), \
                                      asio::placeholders::error));

#define SYNC_WRITE(str)                                                 \
    sendstr = str;                                                      \
    asio::write(socket, asio::buffer(sendstr),                          \
                asio::transfer_all(),                                   \
                last_error);

#define ASYNC_READ(fun)                                                 \
    asio::async_read_until(socket, recvbuf, '\n',                       \
                           boost::bind(&Session::fun, shared_from_this(), \
                                       asio::placeholders::error));

    /* othello protocol */
    void start(){
      ASYNC_WRITE(std::string("othello ") + othello_protocol_version + "\n", send_hello_over);
    }
  private:
    void send_failed(const std::string &errstr, const std::string errpfx = "failed"){
      SYNC_WRITE(errpfx + ' ' + errstr);
      session_end = 1;
    }
    void send_hello_over(const system::error_code &error){
      if(!error){
        ASYNC_READ(wait_hello);
      }
    }
    void wait_hello(const system::error_code& error){
      if(!error){
        if(recvbuf2str() == std::string("othello ") + othello_protocol_version){
          game->lock();
          game->Subject<GameStateObserver>::attach_observer(this);
          std::string s = "init ";
          s += game->get_black_player()?(game->get_black_player()->is_ready()?'B':'b'):'?';
          s += game->get_white_player()?(game->get_white_player()->is_ready()?'W':'w'):'?';
          CHESS_GRID grids[64];
          CHESS_SIDE whose_turn;
          int step;
          game->get_board_snapshot(grids, whose_turn, step);
          s += whose_turn == BLACK?'B':'W';//get nextside
          for(int i = 0; i < 64; ++i){
            s += gridtbl[grids[i]];
          }
          s += itoa(step);
          game->unlock();
          SYNC_WRITE(s + "\n");
          ASYNC_READ(wait_command);
        }else{
          send_failed("wait hello error\n");
        }
      }
    }
    void wait_command(const system::error_code& error){
      if(!error){
        std::string l = recvbuf2str();
        std::string cmd = l.substr(0, l.find(' '));
        std::string arg;
        if(cmd != l){
          arg = l.substr(l.find(' ')+1);
        }
        if(cmd == "quit"){
          return;
        }else if(cmd == "be_black"){
          if(black){
            send_failed("black player is active\n");
          }else{
            black = new NetworkPlayerDummy();
            if(!game->attach_black_player(black)){
              send_failed("black\n", "be_failed");
              delete black;
              black = NULL;
            }
          }
        }else if(cmd == "be_white"){
          if(white){
            send_failed("white player is active\n");
          }else{
            white = new NetworkPlayerDummy();
            if(!game->attach_white_player(white)){
              send_failed("white\n", "be_failed");
              delete white;
              white = NULL;
            }
          }
        }else if(cmd == "black_ready"){
          if(black == NULL){
            send_failed("black user is not active\n");
          }else{
            if(game->has_enough_players())black->ready();
          }
        }else if(cmd == "white_ready"){
          if(white == NULL){
            send_failed("white user is not active\n");
          }else{
            if(game->has_enough_players())white->ready();
          }
        }else if(cmd == "black_go"){
          if(black == NULL){
            send_failed("black user is not active\n");
          }else{
            if(arg.length() != 2)send_failed("go invalid\n");
            black->go(arg[0]-'0', arg[1]-'0');
          }
        }else if(cmd == "white_go"){
          if(white == NULL){
            send_failed("white user is not active\n");
          }else{
            if(arg.length() != 2)send_failed("go invalid\n");
            white->go(arg[0]-'0', arg[1]-'0');
          }
        }else if(cmd == "black_pass"){
          if(black == NULL){
            send_failed("black user is not active\n");
          }else{
            black->pass();
          }
        }else if(cmd == "white_pass"){
          if(white == NULL){
            send_failed("white user is not active\n");
          }else{
            white->pass();
          }
        }else if(cmd == "black_quit"){
          if(black == NULL){
            send_failed("black user is not active\n");
          }else{
            black->quit_game();
          }
        }else if(cmd == "white_quit"){
          if(white == NULL){
            send_failed("white user is not active\n");
          }else{
            white->quit_game();
          }
        }else{
          send_failed("unknown command : " + cmd + "\n");
        }
        if(!session_end)ASYNC_READ(wait_command);
      }
    }

  private:
    virtual void game_state_wait_players(){
      SYNC_WRITE("game_state_wait_players\n");
    }
    virtual void game_state_player_join(CHESS_SIDE side){
      SYNC_WRITE(std::string()+"game_state_player_join "+grid2str[side]+"\n");
    }
    virtual void game_state_wait_players_ready(){
      SYNC_WRITE("game_state_wait_players_ready\n");
    }
    virtual void game_state_player_ready(CHESS_SIDE side){
      SYNC_WRITE(std::string()+"game_state_player_ready "+grid2str[side]+"\n");
    }
    virtual void game_state_roll(){
      SYNC_WRITE("game_state_roll\n");
    }
    virtual void game_state_player_foul(CHESS_SIDE side, const std::string &reason){
      SYNC_WRITE(std::string()+"game_state_player_foul "+grid2str[side]+" "+reason+"\n");
    }
    virtual void game_state_players_exchange(){
      SYNC_WRITE("game_state_players_exchange\n");
      std::swap(black, white);
    }
    virtual void game_state_player_go(CHESS_SIDE side, int x, int y){
      SYNC_WRITE(std::string()+"game_state_player_go "+grid2str[side]+" "+itoa(x)+itoa(y)+"\n");
    }
    virtual void game_state_player_pass(CHESS_SIDE side){
      SYNC_WRITE(std::string()+"game_state_player_pass "+grid2str[side]+"\n");
    }
    virtual void game_state_player_quit(CHESS_SIDE side){
      SYNC_WRITE(std::string()+"game_state_player_quit "+grid2str[side]+"\n");
    }
    virtual void game_state_stop(){
      SYNC_WRITE("game_state_stop\n");
      session_end = 1;
    }
    virtual void game_state_restart(){
      SYNC_WRITE("game_state_restart\n");
    }
    virtual void game_state_player_detached(CHESS_SIDE side){
      if(side == BLACK){
        if(black){
          delete black;
          black = NULL;
        }
      }else{
        if(white){
          delete white;
          white = NULL;
        }
      }
      SYNC_WRITE(std::string()+"game_state_player_detached "+grid2str[side]+"\n");
    }
  };
  typedef boost::shared_ptr<Session> SessionPtr;


  class NetworkGameServerImpl{
    friend class NetworkGameServer;
    system::error_code last_error;
    NetworkGameServer *server;
    asio::io_service io_service;
    tcp::acceptor acceptor;
    boost::thread *acceptor_thread;
    std::string last_error_str;

  public:
    NetworkGameServerImpl(NetworkGameServer *s):
      server(s),
      acceptor(io_service, tcp::v4()){
      acceptor.set_option(asio::socket_base::reuse_address(true));
      acceptor.bind(tcp::endpoint(tcp::v4(), server->tcpport), last_error);
      if(last_error){
        acceptor_thread = NULL;
      }else{
        acceptor_thread = new boost::thread(boost::bind(&NetworkGameServerImpl::do_acceptor_thread, this));
      }
    }
    ~NetworkGameServerImpl(){
      io_service.stop();
      if(acceptor_thread){
        acceptor_thread->join();
        delete acceptor_thread;
      }
    }

  private:
    void do_acceptor_thread(){
      acceptor.listen();
      wait_new_async_accept();
      try{
        io_service.run();
      }catch(std::exception &e){
        fprintf(stderr, "acceptor thread, catched exception : %s\n", e.what());
      }catch(...){
        fprintf(stderr, "acceptor thread, unknown exception\n");
      }
    }
    void wait_new_async_accept(){
      SessionPtr newsp = Session::create(io_service, server->game);
      acceptor.async_accept(newsp->socket, boost::bind(&NetworkGameServerImpl::on_async_accept, this,
                                                       newsp, asio::placeholders::error));
    }
    void on_async_accept(SessionPtr sp, const system::error_code& error){
      if(!error){
        sp->start();
        wait_new_async_accept();
      }
    }
  };

  NetworkGameServer::NetworkGameServer(int _tcpport){
    tcpport = _tcpport;
    impl = new NetworkGameServerImpl(this);
  }

  int NetworkGameServer::is_ok(){
    return !impl->last_error;
  }

  std::string NetworkGameServer::get_last_error(){
    return impl->last_error.message();
  }

  NetworkGameServer::~NetworkGameServer(){
    delete impl;
  }
}

