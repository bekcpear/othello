/**
 * @file     wx/wxothelloboard.cpp
 * @summary  wx othello game frame
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: wxothelloframe.cpp,v 1.11 2008/07/26 16:29:18 hellwolfmisty Exp $
 */

#include <wx/xrc/xmlres.h>
#if wxCHECK_VERSION(2, 8, 0)
#include <wx/aboutdlg.h>
#endif

#include "othello/configs.hpp"
#include "othello/game.hpp"
#include "othello/networkgame.hpp"
#include "othello/player_plugin_manager.hpp"

#include "configmgr.hpp"
#include "thememgr.hpp"
#include "playerchoosedialog.hpp"
#include "windowboardpainter.hpp"
#include "wxothelloplayer.hpp"
#include "wxothelloframe.hpp"

using namespace othello;

static const wxChar* grid2wxstr[3] = {
  wxT("NONE"),
  wxT("BLACK"),
  wxT("WHITE")
};

enum {SINGLE_GAME, SERVER_GAME, CLIENT_GAME};

DEFINE_EVENT_TYPE(EVT_UPDATE_GAME_INFO)
DEFINE_EVENT_TYPE(EVT_GAME_STOPPED)
DEFINE_EVENT_TYPE(EVT_LOG_MESSAGE)
DEFINE_EVENT_TYPE(EVT_SET_GAME_STATE)
DEFINE_EVENT_TYPE(EVT_MESSAGE_DIALOG)

class postMessageBox_t{
public:
  wxMutex mutex;
  wxCondition cond;
  wxString msg;
  int style;
  int result;
  postMessageBox_t():cond(mutex){}
};

class GameThread : public wxThread{
  wxWindow *w;
  Game *game;
  Player *a;
  Player *b;
  int gametype;
  int &flag_stop;
  int &flag_quit;
public:
  GameThread(wxWindow *_w,
             Game *_game,
             Player *_a, Player *_b,
             int _gametype,
             int &_flag_stop,
             int &_flag_quit):
    wxThread(wxTHREAD_DETACHED),
    w(_w),
    game(_game),
    a(_a), b(_b),
    gametype(_gametype),
    flag_stop(_flag_stop),
    flag_quit(_flag_quit){
  }

  virtual ExitCode Entry(){
    try{
      if(gametype == CLIENT_GAME){
        game->wait_stop();
      }else{
        if(a)game->attach_black_player(a);
        if(b)game->attach_white_player(b);
        game->loop();
      }
    }catch(std::exception &e){
      postMessageBox_t data;
      data.msg = wxString::Format(_("Game Exception Catched : \n%s\n"),
                                  wxString(e.what(), wxConvUTF8).c_str());
      data.style = wxOK | wxICON_ERROR;
      data.mutex.Lock();
      wxCommandEvent event(EVT_MESSAGE_DIALOG, wxID_ANY);
      event.SetClientData(&data);
      wxPostEvent(w, event);
      data.cond.Wait();
      game->stop();
    }
    flag_stop = 0;
    wxCommandEvent event(EVT_GAME_STOPPED, wxID_ANY);
    wxPostEvent(w, event);
    /* prevent everthing have been destoryed here*/
    if(!flag_quit){
      wxMutexGuiEnter();
      w->UpdateWindowUI();
      wxMutexGuiLeave();
    }
    return 0;
  }
};

class GameStopThread : public wxThread{
  Game *game;
public:
  GameStopThread(Game *_game):
    wxThread(wxTHREAD_DETACHED),
    game(_game){
  }

  virtual ExitCode Entry(){
    game->stop();
    return 0;
  }
};


class wxOthelloFrame :
  public wxFrame,
  private GameStateObserver,
  private GameAuxinfoObserver{
  ConfigManager *config;
  ThemeManager *theme;
  wxWindow *window_board;
  WindowBoardPainter *wbpainter;

  PlayerPluginManager *ppm;
  std::string a_type;
  std::string b_type;
  othello_player_plugin_node *plugin_a;
  othello_player_plugin_node *plugin_b;
  Player *player_a;
  Player *player_b;
  argmap a_args;
  argmap b_args;

  int a_winnum, b_winnum;
  int ab_order;
  int gametype;
  GameCreator *gamecreator;

  int flag_stop;
  int flag_restart;
  int flag_quit;

public:
  wxOthelloFrame(){
    // load config
    config = ConfigManager::Create();

    // load theme
    theme = ThemeManager::Create(config);
    if(theme == NULL){
      Destroy();
      return;
    }
    std::string themefailed;
    if(theme->ReloadTheme(themefailed)){
      wxMessageBox(wxString::Format(_("Load theme \"%s\" failed :\n%s\n"
                                      "Try to load default theme now"),
                                    wxString(config->get_themename().c_str(),
                                             wxConvUTF8).c_str(),
                                    wxString(themefailed.c_str(), wxConvUTF8).c_str()),
                   _("Theme System Warning"),
                   wxOK|wxICON_ERROR);
      config->reset_themename();
      if(theme->ReloadTheme(themefailed)){
        wxMessageBox(wxString::Format(_("Load default theme failed :\n%s\n"
                                        "Please check your installation"),
                                      wxString(themefailed.c_str(), wxConvUTF8).c_str()),
                     _("Theme System Warning"),
                     wxOK|wxICON_ERROR);
        Destroy();
        return;
      }
    }
    if(theme->Load_wxOthelloFrame(this, NULL)){
      window_board = FindWindow(XRCID("window_board"));
      if(window_board){
        window_board->SetBackgroundStyle(wxBG_STYLE_CUSTOM);
        if(window_board == NULL){
          wxLogFatalError(wxT("Load Window Board Failed"));
        }
        window_board->SetMinSize(wxSize(theme->get_background().GetWidth(),
                                        theme->get_background().GetHeight()));
        wbpainter = WindowBoardPainter::Create(window_board, theme);
        wbpainter->UpdateTheme(theme);
        wbpainter->SetSpeed(config->get_gamespeed());
        Fit();
      }else{
        wxMessageBox(wxT("OthelloFrame find window_board subwindow failed"),
                     _("Theme System Warning"),
                     wxOK|wxICON_ERROR);
        Destroy();
        return;
      }
    }else{
      wxMessageBox(wxT("Load OthelloFrame failed"),
                   _("Theme System Warning"),
                   wxOK|wxICON_ERROR);
      Destroy();
      return;
    }

    // plugin init
    plugin_a = plugin_b = NULL;
    player_a = player_b = NULL;
    std::vector<std::string> plugindirs;
    config->get_plugindirs(plugindirs);
    ppm = new PlayerPluginManager(plugindirs);
    std::string pluginserrs;
    ppm->init(pluginserrs);
    if(!pluginserrs.empty()){
      wxMessageBox(wxString::Format(_("Plugin system warning :\n%s\n"),
                                    wxString(pluginserrs.c_str(), wxConvUTF8).c_str()),
                   _("Plugin System Warnings"),
                   wxOK|wxICON_EXCLAMATION, this);
    }

    //misc init
    ab_order = a_winnum = b_winnum = 0;
    gamecreator = NULL;
    flag_quit = flag_restart = flag_stop = 0;

    //init log entry
    delete wxLog::SetActiveTarget
      (new wxLogTextCtrl(XRCCTRL(*this, "textctrl_log", wxTextCtrl)));

    //add status bar
    CreateStatusBar();

    Show();
  }

  ~wxOthelloFrame(){
    delete wbpainter;
    delete theme;
    delete config;
    delete ppm;
  }

public:
  void startgame(){
    if(gamecreator)return;

    flag_stop = 0;
    a_winnum = b_winnum = 0;
    ab_order = 0;

    /* create players */
    std::string *typex[] = {&a_type, &b_type};
    Player **playerx[] = {&player_a, &player_b};
    argmap *argsx[] = {&a_args, &b_args};
    othello_player_plugin_node **pluginx[] = {&plugin_a, &plugin_b};
    for(int i = 0; i < 2; ++i){
      if(*typex[i] == "human" || typex[i]->empty()){
        *playerx[i] = Create_wxOthelloPlayer(window_board, theme);
      }else if(*typex[i] == "null"){
        *playerx[i] = NULL;
      }else{
        arg_node arg[32];
        arg_node *args = arg;
        for(argmap::iterator iter = argsx[i]->begin();
            iter != argsx[i]->end();
            ++iter, ++args){
          args->name = iter->first.c_str();
          args->value = iter->second.c_str();
        }
        args->name = args->value = NULL;
        *pluginx[i] = ppm->get_plugin(*typex[i]);
        char *pluginerr = NULL;
        if((*pluginx[i])->create_player(*pluginx[i], arg, playerx[i], &pluginerr)){
          cleanup_game();
          wxMessageBox(wxString::Format(_("Player create failed :\n%s\n"),
                                        pluginerr),
                       _("Player Create Failed"),
                       wxOK|wxICON_ERROR,
                       this);
          free(pluginerr);
          return;
        }
        if(pluginerr){
          wxMessageBox(wxString::Format(_("Player create with warnings :\n%s\n"),
                                        pluginerr),
                       _("Player Create Warning"),
                       wxOK|wxICON_EXCLAMATION,
                       this);
          free(pluginerr);
        }
      }
    }

    /* create game */
    switch(gametype){
    case SINGLE_GAME:
      gamecreator = new SingleMachineGame();
      break;
    case SERVER_GAME:
      gamecreator = new NetworkGameServer(config->get_serverport());
      if(!gamecreator->is_ok()){
        wxMessageBox(wxString::Format(_("Game server create failed :\n%s\n"),
                                      wxString(gamecreator->get_last_error().c_str(), wxConvUTF8).c_str()),
                     _("Create Server Failed"),
                     wxOK|wxICON_ERROR,
                     this);
        cleanup_game();
        return;
      }
      break;
    case CLIENT_GAME:
      long remote_tcpport = othello_default_tcpport;
      wxString svraddr = wxGetTextFromUser(_("Input server address(localhost, foo.com:26417, etc...)"),
                                           _("Server Adress"),
                                           wxT("localhost"),
                                           this);
      if(svraddr.IsNull()){
        cleanup_game();
        return;
      }
      if(svraddr.Find(wxChar(':')) != wxNOT_FOUND){
        wxString portstr = svraddr.Mid(svraddr.Find(wxChar(':'))+1);
        if(!portstr.ToLong(&remote_tcpport)){
          wxMessageBox(wxString::Format(_("Invalid port number\n")),
                       _("Create Server Failed"),
                       wxOK|wxICON_ERROR,
                       this);
          cleanup_game();
          return;
        }
        svraddr = svraddr.Mid(0, svraddr.Find(wxChar(':')));
      }
      gamecreator = new NetworkGameClient(svraddr.mb_str(wxConvUTF8), remote_tcpport,
                                          player_a, player_b);
      if(!gamecreator->is_ok()){
        wxMessageBox(wxString::Format(_("Game client create failed :\n%s\n"),
                                      wxString(gamecreator->get_last_error().c_str(), wxConvUTF8).c_str()),
                     _("Create Client Failed"),
                     wxOK|wxICON_ERROR,
                     this);
        cleanup_game();
        return;
      }
      break;
    }

    wbpainter->SetGame(gamecreator->get_game());
    gamecreator->get_game()->Subject<GameStateObserver>::attach_observer(this);
    gamecreator->get_game()->Subject<GameAuxinfoObserver>::attach_observer(this);

    wxThread *gamethread = new GameThread(this,
                                          gamecreator->get_game(),
                                          player_a, player_b,
                                          gametype,
                                          flag_stop,
                                          flag_quit);
    gamethread->Create();
    gamethread->Run();
  }

  void stopgame(){
    if(gamecreator == NULL)return;

    flag_stop = 1;
    wxThread *gamestopthread = new GameStopThread(gamecreator->get_game());
    gamestopthread->Create();
    gamestopthread->Run();
    postSetGameState(_("Stopping Game..."));
  }

public:
  void OnStartSingle(wxCommandEvent&){
    gametype = SINGLE_GAME;
    startgame();
  }

  void OnStartServer(wxCommandEvent&){
    gametype = SERVER_GAME;
    startgame();
  }

  void OnStartClient(wxCommandEvent&){
    gametype = CLIENT_GAME;
    startgame();
  }

  void OnStop(wxCommandEvent&){
    stopgame();
  }

  void OnRestart(wxCommandEvent&){
    flag_restart = 1;
    stopgame();
  }

  void OnChoosePlayers(wxCommandEvent&){
    ChoosePlayers(ppm, theme, this, a_type, b_type, a_args, b_args);
  }

  void OnQuit(wxCommandEvent&){
    Close();
  }

  void OnClose(wxCloseEvent&){
    if(gamecreator == NULL){
      Destroy();
      return;
    }
    flag_quit = 1;
    stopgame();
  }

  void OnPreference(wxCommandEvent&){
    if(config->show_config_dialog(theme, this) == 0){
      //update gamespeed
      wbpainter->SetSpeed(config->get_gamespeed());
    }
  }

  void OnAbout(wxCommandEvent&){
    wxString desc = _("Othello is a classic strategy game, also known as Reversi. "
                      "Its objective is to finish the game with the greater amount "
                      "of pieces (circles) of the same color.");
#if wxCHECK_VERSION(2, 8, 0)
    wxAboutDialogInfo info;
    info.SetName(wxT("wxothello"));
    info.SetVersion(wxString(package_version, wxConvUTF8));
    info.SetDescription(desc);
    info.AddDeveloper(wxT("ZC Miao <hellwolf.misty@gmail.com>"));
    info.SetCopyright(wxT("(C) 2007 ZC Miao <hellwolf.misty@gmail.com>"));
    info.SetWebSite(wxT("http://sourceforge.net/projects/othello-game"));
    wxAboutBox(info);
#else
    wxMessageBox(wxString(wxT("wxothello")) + wxString(package_version, wxConvUTF8) + wxChar('\n') +
                 desc + wxChar('\n') +
                 wxT("ZC Miao <hellwolf.misty@gmail.com>") + wxChar('\n') +
                 wxT("(C) 2007 ZC Miao <hellwolf.misty@gmail.com>") + wxChar('\n') +
                 wxT("http://sourceforge.net/projects/othello-game"),
                 _("About wxOthello"),
                 wxOK,
                 this);
#endif
  }

  void OnUpdateUI_enable_when_game_started(wxUpdateUIEvent &event){
    if(gamecreator && !flag_stop){
      event.Enable(true);
    }else{
      event.Enable(false);
    }
  }

  void OnUpdateUI_disable_when_game_started(wxUpdateUIEvent &event){
    if(gamecreator){
      event.Enable(false);
    }else{
      event.Enable(true);
    }
  }

  void OnUpdateGameInfo(wxCommandEvent &WXUNUSED(event)){
    int bnum = 0;
    int wnum = 0;
    if(gamecreator){
      bnum = gamecreator->get_game()->get_board()->getnum(BLACK);
      wnum = gamecreator->get_game()->get_board()->getnum(WHITE);
    }
    XRCCTRL(*this, "label_blacknum", wxStaticText)->SetLabel
      (wxString::Format(wxT("%d"), bnum));
    XRCCTRL(*this, "label_a_winnum", wxStaticText)->SetLabel
      (wxString::Format(wxT("%d%s"), (int)a_winnum/2, a_winnum%2?wxT(".5"):wxT("")));
    XRCCTRL(*this, "label_whitenum", wxStaticText)->SetLabel
      (wxString::Format(wxT("%d"), wnum));
    XRCCTRL(*this, "label_b_winnum", wxStaticText)->SetLabel
      (wxString::Format(wxT("%d%s"), (int)b_winnum/2, b_winnum%2?wxT(".5"):wxT("")));
  }

  void cleanup_game(){
    delete gamecreator;
    gamecreator = NULL;
    if(plugin_a){
      plugin_a->release_player(plugin_a, player_a);
      plugin_a = NULL;
      player_a = NULL;
    }else{
      delete player_a;
      player_a = NULL;
    }
    if(player_b){
      if(plugin_b){
        plugin_b->release_player(plugin_b, player_b);
        plugin_b = NULL;
        player_b = NULL;
      }else{
        delete player_b;
        player_b = NULL;
      }
    }
  }

  void OnGameStopped(wxCommandEvent&){
    wbpainter->ClearGame();
    cleanup_game();
    postSetGameState(_("Game stopped"));
    if(flag_restart){
      flag_restart = 0;
      startgame();
    }
    if(flag_quit){
      Close();
    }
  }

  void OnLogMessage(wxCommandEvent &event){
    wxLogMessage(wxT("%s"), event.GetString().c_str());
  }

  void OnSetGameState(wxCommandEvent &event){
    SetStatusText(event.GetString());
  }

  void OnMessageBox(wxCommandEvent &event){
    postMessageBox_t *pdata = (postMessageBox_t*)event.GetClientData();
    wxMutexLocker dialog_lock(pdata->mutex);
    pdata->result = wxMessageBox(pdata->msg,
                                 _("Game Infomation"),
                                 pdata->style,
                                 this);
    pdata->cond.Broadcast();
  }

  // Multithread GUI Function wrapper
public:
  void postUpdateGameInfo(){
    wxCommandEvent event(EVT_UPDATE_GAME_INFO, wxID_ANY);
    wxPostEvent(this, event);
  }
 
  int postMessageBox(wxString msg, int style = wxOK){
    postMessageBox_t data;
    data.msg = msg;
    data.style = style;
    data.mutex.Lock();
    wxCommandEvent event(EVT_MESSAGE_DIALOG, wxID_ANY);
    event.SetClientData(&data);
    wxPostEvent(this, event);
    data.cond.Wait();
    return data.result;
  }

  void postLogMessage(const wxChar *fmt, ...){
    wxString logmsg;
    va_list ap;
    va_start(ap, fmt);
    logmsg = wxString::FormatV(fmt, ap);
    va_end(ap);
    wxCommandEvent event(EVT_LOG_MESSAGE, wxID_ANY);
    event.SetString(logmsg);
    wxPostEvent(this, event);
  }

  void postSetGameState(wxString state){
    wxCommandEvent event(EVT_SET_GAME_STATE, wxID_ANY);
    event.SetString(state);
    wxPostEvent(this, event);
  }

  //GameStateObserver
public:
  virtual void game_state_wait_players(){
    postLogMessage(wxT("game_state_wait_players()"));
    postSetGameState(_("Waiting Players..."));
    postUpdateGameInfo();
  }
  virtual void game_state_player_join(CHESS_SIDE side){
    postLogMessage(wxT("game_state_player_join(%s)"), grid2wxstr[side]);
    postUpdateGameInfo();
  }
  virtual void game_state_wait_players_ready(){
    postLogMessage(wxT("game_state_wait_players_ready()"));
    postSetGameState(_("Wait All Players Ready..."));
    postUpdateGameInfo();
  }
  virtual void game_state_player_ready(CHESS_SIDE side){
    postLogMessage(wxT("game_state_player_ready(%s)"), grid2wxstr[side]);
    postUpdateGameInfo();
  }
  virtual void game_state_roll(){
    postLogMessage(wxT("game_state_roll()"));
    postUpdateGameInfo();
  }
  virtual void game_state_player_foul(CHESS_SIDE side, const std::string &reason){
    postLogMessage(wxT("game_state_player_foul(%s, %s)"),
                 grid2wxstr[side],
                 wxString(reason.c_str(), wxConvUTF8).c_str());
    postUpdateGameInfo();
  }
  virtual void game_state_players_exchange(){
    postLogMessage(wxT("game_state_players_exchange()"));
    ab_order = !ab_order;
    postUpdateGameInfo();
  }
  virtual void game_state_player_go(CHESS_SIDE side, int x, int y){
    postLogMessage(wxT("game_state_player_go(%s, %d, %d)"), grid2wxstr[side], x, y);
    postUpdateGameInfo();
  }
  virtual void game_state_player_pass(CHESS_SIDE side){
    postLogMessage(wxT("game_state_player_pass(%s)"), grid2wxstr[side]);
    postUpdateGameInfo();
  }
  virtual void game_state_player_quit(CHESS_SIDE side){
    postLogMessage(wxT("game_state_player_quit(%s)"), grid2wxstr[side]);
    postUpdateGameInfo();
  }
  virtual void game_state_restart(){
    postLogMessage(wxT("game_state_restart()"));
    postUpdateGameInfo();
  }
  virtual void game_state_stop(){
    postLogMessage(wxT("game_state_stop()"));
    if(gamecreator){
      if(!gamecreator->is_ok()){
        postMessageBox(wxString(gamecreator->get_last_error().c_str(), wxConvUTF8));
      }
    }
    postUpdateGameInfo();
  }
  virtual void game_state_player_detached(CHESS_SIDE side){
    postLogMessage(wxT("game_state_player_detached(%s)"), grid2wxstr[side]);
    postUpdateGameInfo();
  }

  //GameAuxinfoObserver
private:
  virtual void game_auxinfo_start(){
    postLogMessage(wxT("game_auxinfo_start()"));
  }
  virtual void game_auxinfo_turn_of(CHESS_SIDE side){
    postLogMessage(wxT("game_auxinfo_turn_of(%s)"), grid2wxstr[side]);
    postLogMessage(wxT("waiting for %s to make decision"), grid2wxstr[side]);
    if(side == BLACK){
      postSetGameState(wxString::Format(_("Turn of Black, step %d"),
                                        gamecreator->get_game()->get_board()->getstep()));
    }else{
      postSetGameState(wxString::Format(_("Turn of White, step %d"),
                                        gamecreator->get_game()->get_board()->getstep()));
    }
  }
  virtual void game_auxinfo_black_win(){
    postLogMessage(wxT("game_auxinfo_black_win()"));
    postMessageBox(_("Black Win!"));
    if(ab_order){
      b_winnum += 2;
    }else{
      a_winnum += 2;
    }
  }
  virtual void game_auxinfo_white_win(){
    postLogMessage(wxT("game_auxinfo_white_win()"));
    postMessageBox(_("White Win!"));
    if(ab_order){
      a_winnum += 2;
    }else{
      b_winnum += 2;
    }
  }
  virtual void game_auxinfo_draw(){
    postLogMessage(wxT("game_auxinfo_draw()"));
    postMessageBox(_("Draw Game!"));
    a_winnum += 1;
    b_winnum += 1;
  }
  virtual void game_auxinfo_ahead_over(){
    postLogMessage(wxT("game_auxinfo_ahead_over()"));
  }
  virtual void game_auxinfo_gameover(){
    postLogMessage(wxT("game_auxinfo_gameover()"));
    postSetGameState(_("Game Over"));
    if(postMessageBox(_("Continue?"), wxYES_NO) == wxNO){
      gamecreator->get_game()->stop();
    }
  }
  virtual void game_auxinfo_force_stop(){
    postLogMessage(wxT("game_auxinfo_force_stop()"));
  }
  virtual void game_auxinfo_no_vital_grids_for(CHESS_SIDE side){
    postLogMessage(wxT("game_auxinfo_no_vital_grids_for(%s)"), grid2wxstr[side]);
  }
  virtual void game_auxinfo_player_force_pass(CHESS_SIDE side){
    postLogMessage(wxT("game_auxinfo_player_force_pass(%s)"), grid2wxstr[side]);
  }

protected:
  DECLARE_EVENT_TABLE();
};


BEGIN_EVENT_TABLE(wxOthelloFrame, wxFrame)
//menu event
EVT_MENU(XRCID("menu_start_single"), wxOthelloFrame::OnStartSingle)
EVT_MENU(XRCID("menu_start_server"), wxOthelloFrame::OnStartServer)
EVT_MENU(XRCID("menu_start_client"), wxOthelloFrame::OnStartClient)
EVT_MENU(XRCID("menu_stop"), wxOthelloFrame::OnStop)
EVT_MENU(XRCID("menu_restart"), wxOthelloFrame::OnRestart)
EVT_MENU(XRCID("menu_players"), wxOthelloFrame::OnChoosePlayers)
EVT_MENU(wxID_PROPERTIES, wxOthelloFrame::OnPreference)
EVT_MENU(wxID_EXIT, wxOthelloFrame::OnQuit)
EVT_MENU(wxID_ABOUT, wxOthelloFrame::OnAbout)
//update ui event
EVT_UPDATE_UI(XRCID("menu_start"), wxOthelloFrame::OnUpdateUI_disable_when_game_started)
EVT_UPDATE_UI(XRCID("menu_stop"), wxOthelloFrame::OnUpdateUI_enable_when_game_started)
EVT_UPDATE_UI(XRCID("menu_restart"), wxOthelloFrame::OnUpdateUI_enable_when_game_started)
//game control customized events
EVT_COMMAND(wxID_ANY, EVT_UPDATE_GAME_INFO, wxOthelloFrame::OnUpdateGameInfo)
EVT_COMMAND(wxID_ANY, EVT_GAME_STOPPED, wxOthelloFrame::OnGameStopped)
EVT_COMMAND(wxID_ANY, EVT_LOG_MESSAGE, wxOthelloFrame::OnLogMessage)
EVT_COMMAND(wxID_ANY, EVT_SET_GAME_STATE, wxOthelloFrame::OnSetGameState)
EVT_COMMAND(wxID_ANY, EVT_MESSAGE_DIALOG, wxOthelloFrame::OnMessageBox)
//misc event
EVT_CLOSE(wxOthelloFrame::OnClose)
END_EVENT_TABLE();


wxFrame* Create_wxOthelloFrame(){
  return new wxOthelloFrame();
}

