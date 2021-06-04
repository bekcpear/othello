/**
 * @file     cli/othello-cli.cpp
 * @summary  othello core cli test program
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: othello-cli.cpp,v 1.17 2009/07/05 19:32:17 hellwolfmisty Exp $
 */

#include <exception>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "othello/configs.hpp"
#include "othello/game.hpp"
#include "othello/player.hpp"
#include "othello/networkgame.hpp"
#include "othello/player_plugin_manager.hpp"

using namespace std;
using namespace othello;


static int suppress = 0;

static const char* grid2str[] = {
  "NONE",
  "BLACK",
  "WHITE"
};

static void print_board(const Board* board){
  printf("  0 1 2 3 4 5 6 7\n");
  for(int y = 0; y < 8; ++y){
    printf("%d", y);
    for(int x = 0; x < 8; ++x){
      static char gridsym[] = {'-', 'x', 'o'};
      printf(" %c", gridsym[board->getgrid(x, y)]);
    }
    printf("\n");
  }
  printf("B:%2d   W:%2d\n", board->getnum(BLACK), board->getnum(WHITE));
}

class CLIPlayer : public Player{
public:
  CLIPlayer(){}

  virtual void do_prepare(){
    while(1){
      printf("%s READY?(y/n)", grid2str[get_myside()]);
      string line;
      if(cin.fail())cin.clear();
      getline(cin, line);
      if(line == "y"){
        ready();
        break;
      }else if(line == "n"){
        quit_game();
        break;
      }
    }
  }

  virtual void do_get_turn(Board &board){
    while(1){
      printf("%s %02d > ", grid2str[get_myside()], board.getstep());
      string line;
      if(cin.fail())cin.clear();
      getline(cin, line);
      if(line[0] == 'g'){
        int x, y;
        if(sscanf(line.c_str(), "g %1u %1u", &x, &y) == 2 &&
           !(x > 7 || y > 7)){
          switch(board.try_go(x, y, get_myside())){
          case Board::GO_OK:
            printf("GO_OK\n");
            go(x, y);
            return;
            break;
          case Board::GO_GRID_OCCUPIED:
            printf("GO_GRID_OCCUPIED\n");
            break;
          case Board::GO_NO_GRID_TO_TURN:
            printf("GO_NO_GRID_TO_TURN\n");
            break;
          case Board::GO_INVAL:
            printf("GO_INVAL\n");
            break;
          }
        }else{
          cout << "Bad format for command g" << endl;
        }
      }else if(line[0] == 'p'){
        print_board(&board);
      }else if(line[0] == 'n'){
        pass();
        break;
      }else if(line[0] == 'q'){
        quit_game();
        return;
      }else{
        cout << "Unknown command " << line[0] << endl;
      }
    }
  }
};

class CLIObserver :
  private BoardObserver,
  private GameStateObserver,
  private GameAuxinfoObserver{
  int a_winnum, b_winnum;
  int ab_order;
  Game *game;
  const char *gametype;
  int round;

public:
  CLIObserver(Game *_game, const char *_gametype, int _round){
    ab_order = 0;
    a_winnum = b_winnum = 0;
    game = _game;
    gametype = _gametype;
    round = _round;
    game->Subject<GameStateObserver>::attach_observer(this);
    game->Subject<GameAuxinfoObserver>::attach_observer(this);
    game->board_attach_observer(this);
  };

  //BoardObserver
private:
  virtual void board_prepare_begin(){
    if (!suppress) printf("board_prepare_begin()\n");
  }
  virtual void board_prepare_end(){
    if (!suppress) printf("board_prepare_end()\n");
  }
  virtual void board_grid_set(int x, int y, CHESS_SIDE side){
    if (!suppress) printf("board_grid_set(%d, %d, %s)\n", x, y, grid2str[side]);
  }
  virtual void board_grid_turn(int x, int y, CHESS_SIDE newside){
    if (!suppress) printf("board_grid_turn(%d, %d, %s)\n", x, y, grid2str[newside]);
  }

  //GameStateObserver
private:
  virtual void game_state_wait_players(){
    if (!suppress) printf("game_state_wait_players()\n");
    if(strcmp(gametype, "single") == 0){
      if(!game->has_enough_players())game->stop();
    }
  }
  virtual void game_state_player_join(CHESS_SIDE side){
    if (!suppress) printf("game_state_player_join(%s)\n", grid2str[side]);
  }
  virtual void game_state_wait_players_ready(){
    if (!suppress) printf("game_state_wait_players_ready()\n");
  }
  virtual void game_state_player_ready(CHESS_SIDE side){
    if (!suppress) printf("game_state_player_ready(%s)\n", grid2str[side]);
  }
  virtual void game_state_roll(){
    if (!suppress) {
        printf("game_state_roll()\n");
        print_board(game->get_board());
    }
  }
  virtual void game_state_player_foul(CHESS_SIDE side, const std::string &reason){
    if (!suppress) printf("game_state_player_foul(%s, %s)\n", grid2str[side], reason.c_str());
  }
  virtual void game_state_players_exchange(){
    if (!suppress) printf("game_state_players_exchange()\n");
    ab_order = !ab_order;
  }
  virtual void game_state_player_go(CHESS_SIDE side, int x, int y){
    if (!suppress) printf("game_state_player_go(%s, %d, %d)\n", grid2str[side], x, y);
  }
  virtual void game_state_player_pass(CHESS_SIDE side){
    if (!suppress) printf("game_state_player_pass(%s)\n", grid2str[side]);
  }
  virtual void game_state_player_quit(CHESS_SIDE side){
    if (!suppress) printf("game_state_player_quit(%s)\n", grid2str[side]);
  }
  virtual void game_state_restart(){
    if (!suppress) printf("game_state_restart()\n");
  }
  virtual void game_state_stop(){
    if (!suppress) printf("game_state_stop()\n");
  }
  virtual void game_state_player_detached(CHESS_SIDE side){
    if (!suppress) printf("game_state_player_detached(%s)\n", grid2str[side]);
  }

  //GameAuxinfoObserver
private:
  virtual void game_auxinfo_start(){
    if (!suppress) printf("game_auxinfo_start()\n");
  }
  virtual void game_auxinfo_turn_of(CHESS_SIDE side){
    if (!suppress) {
        printf("game_auxinfo_turn_of(%s)\n", grid2str[side]);
        printf("waiting for %s to make decision\n", grid2str[side]);
    }
  }
  virtual void game_auxinfo_black_win(){
    if (!suppress) printf("game_auxinfo_black_win()\n");
    if(ab_order){
      b_winnum += 2;
    }else{
      a_winnum += 2;
    }
  }
  virtual void game_auxinfo_white_win(){
    if (!suppress) printf("game_auxinfo_white_win()\n");
    if(ab_order){
      a_winnum += 2;
    }else{
      b_winnum += 2;
    }
  }
  virtual void game_auxinfo_draw(){
    if (!suppress) printf("game_auxinfo_draw()\n");
    a_winnum += 1;
    b_winnum += 1;
  }
  virtual void game_auxinfo_ahead_over(){
    if (!suppress) printf("game_auxinfo_ahead_over()\n");
  }
  virtual void game_auxinfo_gameover(){
    if (!suppress) printf("game_auxinfo_gameover()\n");
    if(round > 0){
      --round;
      if(round == 0){
        game->stop();
      }
    }
    if ((round == 0) || !suppress) printf("A vs B ------ %d%s : %d%s\n",
           a_winnum/2, a_winnum%2?".5":"",
           b_winnum/2, b_winnum%2?".5":"");
  }
  virtual void game_auxinfo_force_stop(){
    printf("game_auxinfo_force_stop()\n");
  }
  virtual void game_auxinfo_no_vital_grids_for(CHESS_SIDE side){
    if (!suppress) printf("game_auxinfo_no_vital_grids_for(%s)\n", grid2str[side]);
  }
  virtual void game_auxinfo_player_force_pass(CHESS_SIDE side){
    if (!suppress) printf("game_auxinfo_player_force_pass(%s)\n", grid2str[side]);
  }
};

PlayerPluginManager *ppm;
GameCreator *gamecreator = NULL;
othello_player_plugin_node* plugin_a = NULL;
othello_player_plugin_node* plugin_b = NULL;
Player *player_a = NULL;
Player *player_b = NULL;

static void usage(PlayerPluginManager *ppm){
  printf("othello-cli %s \n"
         "  Command line interface of othello game\n"
         "\n"
         "usage : \n"
         "othello-cli [options] single player_a[:args] player_b[:args]\n"
         "othello-cli [options] server player_a[:args] player_b[:args]\n"
         "othello-cli [options] client player_a[:args] player_b[:args] serveraddr\n"
         "\n"
         "options:\n"
         "  -a, --addpluginsdir=dir   add new plugins directory where to search plugins\n"
         "  -r, --round=NUM     play NUM rounds at most\n"
         "  -p, --port=PORT     specify the server port\n"
         "  -n, --no-exchange   don't exchange player when game end\n"
         "  -s, --suppress      suppress most of the messages\n"
         "  -h, --help          display this message again\n"
         "\n"
         "available players :\n"
         "  null        null user(used in network game)\n"
         "  human       command line human user\n"
         "plugin players:\n",
         package_version);
  for(PlayerPluginManager::plugin_iterator iter = ppm->plugin_begin();
      iter != ppm->plugin_end();
      ++iter){
    printf("  %-10s  %s\n", iter->first.c_str(), iter->second->get_node()->summary);
  }
}

static void exit_cleanup(){
  if(gamecreator){
    delete gamecreator;
  }
  if(player_a){
    if(plugin_a){
      plugin_a->release_player(plugin_a, player_a);
    }else{
      delete player_a;
    }
  }
  if(player_b){
    if(plugin_b){
      plugin_b->release_player(plugin_b, player_b);
    }else{
      delete player_b;
    }
  }
  delete ppm;
}

static int do_main(int argc, char **argv){
  srand((unsigned)time(NULL));

  std::vector<std::string> plugindirs;
  int no_exchange = 0;
  //parse options
  int tcpport = othello_default_tcpport;
  int round = -1;
  static struct option long_options[] = {
    {"addpluginsdir",1,0,'a'},
    {"round",1,0,'r'},
    {"port",1,0,'p'},
    {"no-exchange",0,0,'n'},
    {"suppress",0,0,'s'},
    {"help",0,0,'h'},
    {0,0,0,0}
  };
  while(1){
    int c = getopt_long(argc, argv, "a:r:p:nsh",
                        long_options, NULL);
    if(c == -1)break;
    switch(c){
    case 'a':
      plugindirs.push_back(optarg);
      break;
    case 'r':
      round = atoi(optarg);
      break;
    case 'p':
      tcpport = atoi(optarg);
      break;
    case 'h':
      usage(ppm);
      exit(0);
      break;
    case 'n':
      no_exchange = 1;
      break;
    case 's':
      suppress = 1;
      break;
    case '?':
      exit(EINVAL);
      break;
    default:
      exit(EINVAL);
      break;
    }
  }

  ppm = new PlayerPluginManager(plugindirs);
  std::string pluginserrs;
  ppm->init(pluginserrs);
  if(!pluginserrs.empty()){
    printf("collection of plugin manager warnings : \n%s\n", pluginserrs.c_str());
  }

  //players
  if(argc - optind < 3){
    usage(ppm);
    printf("Not enough arguments\n");
    exit_cleanup();
    exit(1);
  }
  const char *pa[] = {argv[optind+1], argv[optind+2]};
  Player** pla[] = {&player_a, &player_b};
  othello_player_plugin_node** plaplugin[] = {&plugin_a, &plugin_b};
  for(int i = 0; i < 2; ++i){
    std::string spa(pa[i]);
    std::string p = spa.substr(0, spa.find_first_of(':'));
    std::string a;
    if(spa.find_first_of(':') != std::string::npos){
      a = spa.substr(spa.find_first_of(':')+1);
    }
    Player **pl = pla[i];
    if(p == "null"){
      *pl = NULL;
    }else if(p == "human"){
      *pl = new CLIPlayer();
    }else{
      othello_player_plugin_node** pp = plaplugin[i];
      if((*pp = ppm->get_plugin(std::string(p))) != NULL){
        char *aap = strdup(a.c_str());
        char *aa = aap;
        arg_node arg[32];
        arg_node *args = args;
        args = arg;
        while(*aa){
          if(!a.empty()){
            args->name = aa;
            while(*aa && *aa != ',' && *aa != '=')++aa;
            if(*aa){
              if(*aa == ','){
                *aa++ = 0;
                args->value = NULL;
              }else{
                *aa++ = 0;
                args->value = aa;
                while(*aa && *aa != ',')++aa;
                if(*aa)*aa++ = 0;
              }
            }else{
              args->value = NULL;
            }
            ++args;
          }
        }
        args->name = NULL;
        printf("Creating plugin player %s\n", p.c_str());
        for(arg_node *ia = arg; ia != args; ++ia){
          printf("  %s", ia->name);
          if(ia->value){
            printf("=%s\n", ia->value);
          }else{
            printf("\n");
          }
        }
        char *pluginerr = NULL;
        if((*pp)->create_player(*pp, arg, pl, &pluginerr)){
          printf("Create plugin player %s failed : %s\n", p.c_str(), pluginerr);
          free(pluginerr);
          return 1;
        }
        if(pluginerr){
          printf("Create plugin player %s with warnings : %s\n", p.c_str(), pluginerr);
          free(pluginerr);
        }
        free(aap);
      }else{
        fprintf(stderr, "unknown player type %s\n", p.c_str());
        return 1;
      }
    }
  }

  //game type
  if(strcmp(argv[optind], "single") == 0){
    if(player_a == NULL || player_b == NULL){
      fprintf(stderr, "single machine game can not have null user\n");
      return 1;
    }
    gamecreator = new SingleMachineGame();
  }else if(strcmp(argv[optind], "server") == 0){
    gamecreator = new NetworkGameServer(tcpport);
  }else if(strcmp(argv[optind], "client") == 0){
    if(argc - optind < 4){
      usage(ppm);
      fprintf(stderr, "Not enough arguments\n");
      exit_cleanup();
      exit(1);
    }
    gamecreator = new NetworkGameClient(argv[optind+3], tcpport, player_a, player_b);
  }else{
    fprintf(stderr, "unknown game type %s\n", argv[optind]);
    exit_cleanup();
    exit(1);
  }

  if(gamecreator->is_ok()){
    CLIObserver cliobserver(gamecreator->get_game(), argv[optind], round);
    if(strcmp(argv[optind], "client") == 0){
        gamecreator->get_game()->wait_stop();
    }else{
      if(player_a)gamecreator->get_game()->attach_black_player(player_a);
      if(player_b)gamecreator->get_game()->attach_white_player(player_b);
      if(no_exchange){
        gamecreator->get_game()->loop_noexchange();
      }else{
        gamecreator->get_game()->loop();
      }
    }
    if(!gamecreator->is_ok()){
      fprintf(stderr, "Game ended with failue : %s\n", gamecreator->get_last_error().c_str());
    }
  }else{
    printf("Game create failed : %s\n", gamecreator->get_last_error().c_str());
  }

  exit_cleanup();

  return 0;
}

int main(int argc, char **argv){
  try{
    return do_main(argc, argv);
  }catch(std::exception &e){
    printf("std::exception catched : \n%s", e.what());
    exit_cleanup();
    exit(1);
  }
}
