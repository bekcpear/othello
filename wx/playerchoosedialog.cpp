/**
 * @file     wx/playerchoosedialog.cpp
 * @summary  choose player dialog
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: playerchoosedialog.cpp,v 1.4 2007/06/28 04:48:45 hellwolfmisty Exp $
 */

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>
#include <wx/notebook.h>

#include "playerchoosedialog.hpp"

class PlayerChooseDialog : public wxDialog{
  PlayerPluginManager *ppm;
  wxRadioBox *radiobox;
  wxListBox *playerslist;
  wxListBox *argslistbox;
  int a_selection;
  int b_selection;
  std::string &player_a;
  std::string &player_b;
  argmap &a_args;
  argmap &b_args;

public:
  PlayerChooseDialog(PlayerPluginManager *_ppm, 
                     std::string &_a, std::string &_b,
                     argmap &_a_args, argmap &_b_args)
    : ppm(_ppm), player_a(_a), player_b(_b),
      a_args(_a_args), b_args(_b_args){
  }

  void Prepare(){
    playerslist = XRCCTRL(*this, "dialog_choose_players_listbox", wxListBox);
    radiobox = XRCCTRL(*this, "dialog_choose_players_radiobox", wxRadioBox);
    argslistbox = XRCCTRL(*this, "dialog_choose_players_args_listbox", wxListBox);

    /* Human Player */
    playerslist->Append(wxT("human"));
    playerslist->Append(wxT("null"));

    a_selection = 0;
    b_selection = 0;

    if(player_a == "null")a_selection = 1;
    if(player_b == "null")b_selection = 1;
    for(PlayerPluginManager::plugin_iterator iter = ppm->plugin_begin();
        iter != ppm->plugin_end();
        ++iter){
      wxString wxs = wxString(iter->first.c_str(), wxConvUTF8);
      int s = playerslist->Append(wxs);
      if(iter->first == player_a)a_selection = s;
      if(iter->first == player_b)b_selection = s;
    }

    playerslist->SetSelection(radiobox->GetSelection()?b_selection:a_selection);
    UpdateInfoPanel();
  }

  void OnOk(wxCommandEvent& WXUNUSED(event)){
    player_a = playerslist->GetString(a_selection).mb_str(wxConvUTF8);
    player_b = playerslist->GetString(b_selection).mb_str(wxConvUTF8);
    EndModal(1);
  }

  void OnCancel(wxCommandEvent& WXUNUSED(event)){
    EndModal(0);
  }

  void UpdateInfoPanel(){
    int s = playerslist->GetSelection();
    XRCCTRL(*this, "dialog_choose_players_a_name", wxStaticText)->SetLabel
      (playerslist->GetString(a_selection));
    XRCCTRL(*this, "dialog_choose_players_b_name", wxStaticText)->SetLabel
      (playerslist->GetString(b_selection));

    if(s == 0){
      XRCCTRL(*this, "dialog_choose_players_playerinfo_name", wxStaticText)->SetLabel
        (playerslist->GetString(0));
      XRCCTRL(*this, "dialog_choose_players_playerinfo_summary", wxStaticText)->SetLabel
        (_("Human Player"));
      XRCCTRL(*this, "dialog_choose_players_playerinfo_description", wxStaticText)->SetLabel
        (_("You control this player with your mouse. Use left click to go,"
           "and right click to pass"));
    }else if(s == 1){
      XRCCTRL(*this, "dialog_choose_players_playerinfo_name", wxStaticText)->SetLabel
        (playerslist->GetString(1));
      XRCCTRL(*this, "dialog_choose_players_playerinfo_summary", wxStaticText)->SetLabel
        (_("Null Player"));
      XRCCTRL(*this, "dialog_choose_players_playerinfo_description", wxStaticText)->SetLabel
        (_("This will not create any local player. It's used when playing network game"));
    }else if(s != wxNOT_FOUND){
      wxString name = playerslist->GetString(s);
      othello_player_plugin_node *pn = ppm->get_plugin
        (std::string(name.mb_str(wxConvUTF8)));
      XRCCTRL(*this, "dialog_choose_players_playerinfo_name", wxStaticText)->SetLabel
        (name);
      XRCCTRL(*this, "dialog_choose_players_playerinfo_summary", wxStaticText)->SetLabel
        (wxString(pn->summary, wxConvUTF8));
      XRCCTRL(*this, "dialog_choose_players_playerinfo_description", wxStaticText)->SetLabel
        (wxString(pn->description, wxConvUTF8));
    }

    argmap *am;
    if(radiobox->GetSelection() == 0){
      am = &a_args;
    }else{
      am = &b_args;
    }
    argslistbox->Clear();
    for(argmap::iterator iter = am->begin();
        iter != am->end();
        ++iter){
      argslistbox->Append(wxString((iter->first+'='+iter->second).c_str(),
                                   wxConvUTF8));
    }
  }

  void OnSelect(wxCommandEvent& event){
    if(event.GetInt() == wxNOT_FOUND)return;
    if(radiobox->GetSelection() == 0){
      a_selection = event.GetInt();
      a_args.clear();
    }else{
      b_selection = event.GetInt();
      b_args.clear();
    }
    UpdateInfoPanel();
  }

  void OnRadioChanged(wxCommandEvent& event){
    if(playerslist->GetSelection() == wxNOT_FOUND)return;
    if(event.GetInt() == 0){
      playerslist->SetSelection(a_selection);
    }else{
      playerslist->SetSelection(b_selection);
    }
    UpdateInfoPanel();
  }

  void OnArgAdd(wxCommandEvent &WXUNUSED(event)){
    wxString argname = wxGetTextFromUser(_("Argument Name"), _("Input text"), wxT(""), this);
    if(argname.IsNull())return;
    if(argname.Find(wxChar('=')) != wxNOT_FOUND){
      wxMessageBox(_("Argument name should not contain '='"),
                   _("Invalid Arguments"), wxOK|wxICON_ERROR, this);
      return;
    }
    wxString argval = wxGetTextFromUser(_("Argument Value"), _("Input text"), wxT(""), this);
    if(argval.IsNull())return;
    if(radiobox->GetSelection() == 0){
      a_args[std::string(argname.mb_str(wxConvUTF8))] = argval.mb_str(wxConvUTF8);
    }else{
      b_args[std::string(argname.mb_str(wxConvUTF8))] = argval.mb_str(wxConvUTF8);
    }
    UpdateInfoPanel();
  }

  void OnArgDelete(wxCommandEvent &event){
    wxString s = argslistbox->GetString(event.GetInt());
    std::string ss(s.Mid(0, s.Find(wxChar('='))).mb_str(wxConvUTF8));
    if(radiobox->GetSelection() == 0){
      a_args.erase(ss);
    }else{
      b_args.erase(ss);
    }
    UpdateInfoPanel();
  }

  void OnArgReset(wxCommandEvent &WXUNUSED(event)){
    UpdateInfoPanel();
    if(radiobox->GetSelection() == 0){
      a_args.clear();
    }else{
      b_args.clear();
    }
    UpdateInfoPanel();
  }

protected:
  DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(PlayerChooseDialog, wxDialog)
EVT_BUTTON(wxID_OK, PlayerChooseDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, PlayerChooseDialog::OnCancel)
EVT_LISTBOX(XRCID("dialog_choose_players_listbox"), PlayerChooseDialog::OnSelect)
EVT_RADIOBOX(XRCID("dialog_choose_players_radiobox"), PlayerChooseDialog::OnRadioChanged)
EVT_BUTTON(XRCID("dialog_choose_players_args_add"), PlayerChooseDialog::OnArgAdd)
EVT_BUTTON(XRCID("dialog_choose_players_args_delete"), PlayerChooseDialog::OnArgDelete)
EVT_BUTTON(XRCID("dialog_choose_players_args_reset"), PlayerChooseDialog::OnArgReset)
END_EVENT_TABLE();


void ChoosePlayers(PlayerPluginManager *ppm,
                   ThemeManager *theme, wxWindow* parent,
                   std::string &player_a, std::string &player_b,
                   argmap &a_args, argmap &b_args){
  PlayerChooseDialog *dialog = new PlayerChooseDialog(ppm, player_a, player_b,
                                                      a_args, b_args);
  theme->Load_ChoosePlayersDialog(dialog, parent);
  dialog->Prepare();
  dialog->ShowModal();
  delete dialog;
}
