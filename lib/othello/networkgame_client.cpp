/**
 * @file     lib/othello/networkgame_client.cpp
 * @summary  othello network game server side code
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: networkgame_client.cpp,v 1.4 2009/07/05 05:17:41 hellwolfmisty Exp $
 */


#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <utility>
#include <stdexcept>

#include "networkgame.hpp"

namespace othello{
  using namespace boost;
  using asio::ip::tcp;

  static std::string itoa(int i){
    char b[20];
    snprintf(b, sizeof(b), "%d", i);
    return b;
  }

  class NetworkGameClientImpl{
    class NetworkPlayerAgent :
      public Player,
      public PlayerObserver{
      friend class NetworkGameClientImpl;
      system::error_code last_error;
      Player *player;
      NetworkGameClientImpl *impl;
    public:
      NetworkPlayerAgent(Player *_player, NetworkGameClientImpl *_impl) 
        : player(_player), impl(_impl){
        if(player)player->attach_observer(this);
      }
      ~NetworkPlayerAgent(){
        if(player)player->detach_observer(this);
      }
      int is_local_player(){return player != NULL;}
    public:
      virtual void do_set_myside(CHESS_SIDE s){
        if(player)player->set_myside(s);
      }
      virtual void do_prepare(){
        if(player)player->prepare();
      }
      virtual void do_get_turn(Board &board){
        if(player)player->get_turn(board);
      }
      virtual void end_game(){
        if(player)player->end_game();
      }
    public:
      virtual void player_ready(Player *){
        impl->io_service_write(get_myside() == BLACK?"black_ready\n":"white_ready\n");
      }
      virtual void player_go(Player *, int x, int y){
        std::string s = get_myside() == BLACK?"black_go ":"white_go ";
        s += itoa(x);
        s += itoa(y);
        s += '\n';
        impl->io_service_write(s);
      }
      virtual void player_pass(Player *){
        impl->io_service_write(get_myside() == BLACK?"black_pass\n":"white_pass\n");
      }
      virtual void player_quit(Player *){
        impl->io_service_write(get_myside() == BLACK?"black_quit\n":"white_quit\n");
      }
    };

    friend class NetworkGameClient;
    NetworkGameClient *client;
    asio::io_service io_service;
    tcp::socket socket;
    NetworkPlayerAgent a_agent, b_agent;
    NetworkPlayerAgent *black_agent, *white_agent;
    boost::thread *client_thread;
    system::error_code last_error;
    std::string last_error2;
    asio::streambuf recvbuf;
    int need_bootstrap_roll;

    std::string recvbuf2str(){
      std::istream is(&recvbuf);
      std::string line;
      std::getline(is, line);
      return line;
    }
  public:
    NetworkGameClientImpl(NetworkGameClient *c,
                          Player *_black, Player *_white) :
      client(c), socket(io_service),
      a_agent(_black, this), b_agent(_white, this){
      need_bootstrap_roll = 0;
      black_agent = &a_agent;
      white_agent = &b_agent;
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(client->destaddr, itoa(client->tcpport));
      tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, last_error);
      if (!last_error) {
          while (endpoint_iterator != tcp::resolver::iterator()) {
              socket.connect(*endpoint_iterator, last_error);
              if (last_error) {
                  /* try next ip, reopen the socket */
                  socket.close();
                  ++endpoint_iterator;
              } else {
                  /* successfully connected */
                  break;
              }
          }
      }
      if(!last_error){
        asio::read_until(socket, recvbuf, '\n', last_error);
      }
      if(!last_error){
        std::string hello_got = recvbuf2str();
        if(hello_got != std::string("othello ") + othello_protocol_version){
          last_error2 = "mismatched othello protocol version, got : "+hello_got;
        }
      }
      if(!last_error || !last_error2.empty()){
        std::string hello = std::string("othello ") + othello_protocol_version + "\n";
        asio::write(socket, asio::buffer(hello),
                    asio::transfer_all(),
                    last_error);
      }
      if(last_error || !last_error2.empty()){
        client_thread = NULL;
      }else{
        client_thread = new boost::thread(boost::bind(&NetworkGameClientImpl::do_client_thread, this));
      }
    }
    ~NetworkGameClientImpl(){
      io_service.stop();
      client->game->stop();
      if(client_thread){
        client_thread->join();
        delete client_thread;
      }
    }
  public:
    void do_io_service_write(std::string msg){
      asio::write(socket, asio::buffer(msg),
                  asio::transfer_all(),
                  last_error);
    }
    void io_service_write(const std::string &msg){
      io_service.post(boost::bind(&NetworkGameClientImpl::do_io_service_write,
                                  this,
                                  msg));
    }
    void do_client_thread(){
      try{
        asio::async_read_until(socket, recvbuf, '\n',
                               boost::bind(&NetworkGameClientImpl::on_init_command, this,
                                           asio::placeholders::error));
        io_service.run();
      }catch(std::exception &e){
        last_error2 = std::string() + "client thread catched exception : " + e.what();
      }catch(...){
        last_error2 = "client thread catched unknown exception";
      }
      if(!client->game->is_stopped())client->game->stop();
    }
    void on_init_command(const system::error_code& error){
      if(!error){
        std::string l = recvbuf2str();
        std::string cmd = l.substr(0, l.find(' '));
        std::string arg;
        if(cmd != l){
          arg = l.substr(l.find(' ')+1);
        }
        if(cmd != "init" || arg.size() < 67){
          throw std::logic_error("protocol error, init command failed, got : "+l);
        }else{
          char bs = arg[0];//black status
          char ws = arg[1];//white status
          char ns = arg[2];//next side
          std::string sstep = arg.substr(67);
          int step;
          if(boost::all(sstep, boost::is_digit())){
            step = atoi(sstep.c_str());
          }else{
            throw std::logic_error("protocol error, init command failed, got step : "+sstep);
          }
          if(black_agent->is_local_player()){
            if(bs == '?'){
              io_service_write("be_black\n");
            }else{
              last_error2 = "black player is already attached";
              return;
            }
          }
          if(white_agent->is_local_player()){
            if(ws == '?'){
              io_service_write("be_white\n");
            }else{
              last_error2 = "white player is already attached";
              return;
            }
          }
          if(bs == '?' || ws == '?'){
            client->game->async_wait_players();
          }
          if(bs == 'B' || bs == 'b'){
            client->game->attach_black_player(black_agent);
          }
          if(ws == 'W' || ws == 'w'){
            client->game->attach_white_player(white_agent);
          }
          if(bs == 'B' && ws == 'W'){
            CHESS_GRID g[64];
            for(int i = 0; i < 64; ++i){
              char c = arg[3+i];
              if(c == '-'){
                g[i] = NONE;
              }else if(c == 'x'){
                g[i] = BLACK;
              }else if(c == 'o'){
                g[i] = WHITE;
              }else{
                throw std::logic_error("protocol error, init command failed, unknown grid : "+c);
              }
            }
            CHESS_SIDE nextside;
            if(ns == 'B'){
              nextside = BLACK;
            }else if(ns == 'W'){
              nextside = WHITE;
            }else{
              throw std::logic_error("protocol error, init command failed, unknown nextside : "+ns);
            }
            client->game->async_prepare(g, nextside, step);
            need_bootstrap_roll = 1;
          }else if(!(bs == '?' || ws == '?')){
            client->game->async_prepare();
          }
          if(!(bs == '?' || ws == '?')){
            if(bs == 'B')black_agent->ready();
            if(ws == 'W')white_agent->ready();
          }
        }
        wait_server_command();
      }
    }
    void wait_server_command(){
      asio::async_read_until(socket, recvbuf, '\n',
                             boost::bind(&NetworkGameClientImpl::on_server_command, this,
                                         asio::placeholders::error));
    }
    void on_server_command(const system::error_code& error){
      if(client->game->is_stopped())return;
      if(!error){
        std::string l = recvbuf2str();
        std::string cmd = l.substr(0, l.find(' '));
        std::string arg;
        if(cmd != l){
          arg = l.substr(l.find(' ')+1);
        }
        if(cmd == "failed"){
          throw std::logic_error("protocol error, got failed : "+arg);
        }else if(cmd == "be_failed"){
          last_error2 = "player is already attached : "+arg;
          return;
        }else if(cmd == "game_state_wait_players"){
          if(client->game->async_wait_players() == 0){
            throw std::logic_error("protocol error, game_state_wait_players");
          }
        }else if(cmd == "game_state_player_join"){
          if(arg == "BLACK"){
            if(client->game->attach_black_player(black_agent) == 0){
              throw std::logic_error("protocol error, game_state_player_join, "
                                     "BLACK join failed");
            }
          }else if(arg == "WHITE"){
            if(client->game->attach_white_player(white_agent) == 0){
              throw std::logic_error("protocol error, game_state_player_join, "
                                     "WHITE join failed");
            }
          }else{
            throw std::logic_error("protocol error, game_state_player_join, "
                                   "bad player : " + arg);
          }
        }else if(cmd == "game_state_wait_players_ready"){
          if(client->game->async_prepare() == 0){
            throw std::logic_error("protocol error, game_state_wait_players_ready");
          }
        }else if(cmd == "game_state_player_ready"){
          if(client->game->has_enough_players()){
            if(arg == "BLACK" && client->game->get_black_player()){
              black_agent->ready();
            }else if(arg == "WHITE" && client->game->get_white_player()){
              white_agent->ready();
            }else{
              throw std::logic_error("protocol error, game_state_player_ready, "
                                     "bad player : " + arg);
            }
          }
        }else if(cmd == "game_state_roll"){
          if(need_bootstrap_roll)need_bootstrap_roll = 0;
          if(client->game->async_roll() == 0){
            throw std::logic_error("protocol error, game_state_roll");
          }
        }else if(cmd == "game_state_player_foul"){
        }else if(cmd == "game_state_players_exchange"){
          if(client->game->exchange() == 0){
            throw std::logic_error("protocol error, game_state_players_exchange");
          }
          std::swap(black_agent, white_agent);
        }else if(cmd == "game_state_player_go"){
          if(need_bootstrap_roll){
            if(client->game->async_roll() == 0){
              throw std::logic_error("protocol error, game_state_player_go(bootstrap roll)");
            }
            need_bootstrap_roll = 0;
          }
          client->game->async_roll();
          int x,y;
          std::string side = arg.substr(0, arg.find_first_of(' '));
          if(arg.size() == 8 && (side == "BLACK" || side == "WHITE")){
            x = arg[6] - '0';
            y = arg[7] - '0';
          }else{
            throw std::logic_error("protocol error, game_state_player_go, invalid arg : "+arg);
          }
          if(side == "BLACK" && client->game->get_black_player()){
            black_agent->go(x, y);
          }else if(side == "WHITE" && client->game->get_white_player()){
            white_agent->go(x, y);
          }else{
            throw std::logic_error("protocol error, game_state_player_go, "
                                   "bad player : " + side);
          }
        }else if(cmd == "game_state_player_pass"){
          if(need_bootstrap_roll){
            if(client->game->async_roll() == 0){
              throw std::logic_error("protocol error, game_state_player_pass(bootstrap roll)");
            }
            need_bootstrap_roll = 0;
          }
          if(arg == "BLACK" && client->game->get_black_player()){
            black_agent->pass();
          }else if(arg == "WHITE" && client->game->get_white_player()){
            white_agent->pass();
          }else{
            throw std::logic_error("protocol error, game_state_player_pass, "
                                   "bad player : " + arg);
          }
        }else if(cmd == "game_state_player_quit"){
          if(arg == "BLACK" && client->game->get_black_player()){
            black_agent->quit_game();
          }else if(arg == "WHITE" && client->game->get_white_player()){
            white_agent->quit_game();
          }else{
            throw std::logic_error("protocol error, game_state_player_quit, "
                                   "bad player : " + arg);
          }
        }else if(cmd == "game_state_restart"){
          client->game->restart();
        }else if(cmd == "game_state_stop"){
          client->game->stop();
        }else if(cmd == "game_state_player_detached"){
          if(arg == "BLACK"){
            client->game->detach_black_player();
          }else if(arg == "WHITE"){
            client->game->detach_white_player();
          }else{
            throw std::logic_error("protocol error, game_state_player_detached, "
                                   "bad player : " + arg);
          }
        }else{
          throw std::logic_error("unknown command "+cmd);
        }
        wait_server_command();
      }
    }
  };

  NetworkGameClient::NetworkGameClient(const char *_destaddr, int _tcpport,
                                       Player *_black, Player *_white){
    tcpport = _tcpport;
    destaddr = strdup(_destaddr);
    impl = new NetworkGameClientImpl(this, _black, _white);
  }

  int NetworkGameClient::is_ok(){
    return !impl->last_error && impl->last_error2.empty();
  }

  std::string NetworkGameClient::get_last_error(){
    return impl->last_error?impl->last_error.message():impl->last_error2;
  }

  NetworkGameClient::~NetworkGameClient(){
    delete impl;
    delete destaddr;
  }

}

