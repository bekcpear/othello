/**
 * @file     wx/configmgr.cpp
 * @summary  wx othello config manager
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: configmgr.cpp,v 1.8 2009/07/05 19:30:56 hellwolfmisty Exp $
 */


#include <vector>
#include <string>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/xrc/xmlres.h>
#include <wx/config.h>
#include <wx/spinctrl.h>

#include "othello/configs.hpp"
#include "othello/networkgame.hpp"
#include "othello/intl_config.hpp"

#include "configmgr.hpp"
#include "thememgr.hpp"

using namespace othello;

class ConfigManagerImpl : public ConfigManager{
  friend class ConfigDialog;
private:
  static std::vector<std::string> sv;

  static const char* wxgettext(const char* msgid){
    wxString ws = wxGetTranslation(wxString(msgid, wxConvUTF8).c_str());
    std::string s(ws.mb_str(wxConvUTF8));
    sv.push_back(s);
    return s.c_str();
  }

#if wxCHECK_VERSION(2, 8, 0)
  static const char* wxdgettext(const char* domainname, const char* msgid){
    wxString ws = wxGetTranslation(wxString(msgid, wxConvUTF8).c_str(),
                                   wxString(domainname, wxConvUTF8).c_str());
    std::string s(ws.mb_str(wxConvUTF8));
    sv.push_back(s);
    return s.c_str();
  }
#endif

private:
  wxConfig config;
  wxString *supported_languages_wxstr;
  int *supported_languages;
  int supported_languages_size;
  wxLocale locale;

public:
  ConfigManagerImpl(){
    supported_languages_size = 4;

    supported_languages = new int[supported_languages_size];
    supported_languages[0] = -1;
    supported_languages[1] = wxLANGUAGE_DEFAULT;
    supported_languages[2] = wxLANGUAGE_CHINESE_SIMPLIFIED;
    supported_languages[3] = wxLANGUAGE_GALICIAN;

    int langcode = get_langcode();
    /* locale init */
    if(langcode >= 0){
      locale.Init(langcode);
      locale.AddCatalogLookupPathPrefix(wxString(locale_dir, wxConvUTF8));
      if(locale.AddCatalog(wxString(package_name, wxConvUTF8)) == true){
        othello::gettext = wxgettext;
#if wxCHECK_VERSION(2, 8, 0)
        othello::dgettext = wxdgettext;
#endif
      }
    }

    supported_languages_wxstr = new wxString[supported_languages_size];
    supported_languages_wxstr[0] = _("Application Default Language(English)");
    supported_languages_wxstr[1] = _("System Default Language");
    supported_languages_wxstr[2] = wxConvUTF8.cMB2WC("简体中文(Chinese Simplified)");
    supported_languages_wxstr[3] = wxConvUTF8.cMB2WC("Lingua galega(Galician)");
  }

  ~ConfigManagerImpl(){
    delete[] supported_languages_wxstr;
    delete[] supported_languages;
  }

public:
  virtual int get_langcode(){
    long langcode;
    config.Read(wxT("lang"), &langcode, wxLANGUAGE_DEFAULT);
    return langcode;
  }
  void set_langcode(int l){
    config.Write(wxT("lang"), l);
  }

  virtual std::string get_datadir(){
    return wx_datadir;
  }

  virtual std::string get_themesdir(){
    return get_datadir()+"/themes/";
  }

  virtual void reset_themename(){
    config.DeleteEntry(wxT("themename"));
  }
  virtual std::string get_themename(){
    wxString themename;
    config.Read(wxT("themename"), &themename, wxT("default"));
    return (const char*)themename.mb_str(wxConvUTF8);
  }
  void set_themename(wxString themename){
    config.Write(wxT("themename"), themename);
  }
  virtual std::string get_themedir(){
    return get_themesdir()+get_themename();
  }

  virtual void get_plugindirs(std::vector<std::string> &plugindirs){
    wxString plugindirs_str;
    config.Read(wxT("plugindirs"), &plugindirs_str);
    int pos;
    while((pos = plugindirs_str.Find(wxChar('@'))) != wxNOT_FOUND){
      plugindirs.push_back((const char*)plugindirs_str.Mid(0, pos).mb_str(wxConvUTF8));
      plugindirs_str = plugindirs_str.Mid(pos+1);
      if(plugindirs_str.IsEmpty())break;
    } 
  }
  void set_plugindirs(std::vector<wxString> &plugindirs){
    wxString plugindirs_str;
    for(size_t i = 0; i < plugindirs.size(); ++i){
      plugindirs_str += plugindirs[i] + wxChar('@');
    }
    config.Write(wxT("plugindirs"), plugindirs_str);
  }

  virtual unsigned short get_serverport(){
    long serverport;
    config.Read(wxT("serverport"), &serverport, othello_default_tcpport);
    return serverport;
  }
  void set_serverport(int i){
    config.Write(wxT("serverport"), i);
  }

  virtual int get_gamespeed(){
    long gamespeed;
    config.Read(wxT("gamespeed"), &gamespeed, 200);
    return gamespeed;
  }
  void set_gamespeed(int i){
    config.Write(wxT("gamespeed"), i);
  }

public:
  virtual int show_config_dialog(ThemeManager *theme, wxWindow *parent);
};

std::vector<std::string> ConfigManagerImpl::sv;


class ConfigDialog : public wxDialog{
  int need_update_config;
  ConfigManagerImpl *configmgr;
  wxChoice *choice_lang;
  wxChoice *choice_themename;
  wxListBox *list_plugindirs;
  wxSpinCtrl *spin_game_server_port;
  wxChoice *choice_gamespeed;
public:
  ConfigDialog(ConfigManagerImpl *_configmgr, ThemeManager *theme, wxWindow *parent){
    need_update_config = 0;

    configmgr = _configmgr;
    theme->Load_PreferenceDialog(this, parent);

    // language choice
    choice_lang = XRCCTRL(*this, "choice_lang", wxChoice);
    int langcode = configmgr->get_langcode();
    for(int i = 0;
        i < configmgr->supported_languages_size;
        ++i){
      choice_lang->Append(configmgr->supported_languages_wxstr[i]);
      if(configmgr->supported_languages[i] == langcode){
        choice_lang->SetSelection(i);
        XRCCTRL(*this, "choice_lang_current", wxStaticText)
          ->SetLabel(configmgr->supported_languages_wxstr[i]);
      }
    }

    // theme choice
    wxDir themesdir(wxString(configmgr->get_themesdir().c_str(), wxConvUTF8));
    choice_themename = XRCCTRL(*this, "choice_themename", wxChoice);
    wxString themename = wxString(configmgr->get_themename().c_str(), wxConvUTF8);
    wxString themedirname;
    bool cont = themesdir.GetFirst(&themedirname, wxString(), wxDIR_DIRS);
    while(cont){
      choice_themename->Append(themedirname);
      cont = themesdir.GetNext(&themedirname);
    }
    choice_themename->SetStringSelection(themename);
    XRCCTRL(*this, "choice_themename_current", wxStaticText)
      ->SetLabel(themename);

    // plugin dir
    list_plugindirs = XRCCTRL(*this, "list_plugindirs", wxListBox);
    std::vector<std::string> plugindirs;
    configmgr->get_plugindirs(plugindirs);
    for(size_t i = 0; i < plugindirs.size(); ++i){
      list_plugindirs->Append(wxString(plugindirs[i].c_str(), wxConvUTF8));
    }

    // server port
    unsigned short serverport = configmgr->get_serverport();
    spin_game_server_port = XRCCTRL(*this, "game_server_port", wxSpinCtrl);
    spin_game_server_port->SetValue(serverport);

    // game speed;
    int gamespeed = configmgr->get_gamespeed();
    choice_gamespeed = XRCCTRL(*this, "choice_gamespeed", wxChoice);
    choice_gamespeed->SetStringSelection(wxString::Format(wxT("%d"), gamespeed));
  }

  void CommandUpdateConfig(wxCommandEvent &){
    need_update_config = 1;
  }

  void SpinContrlUpdateConfig(wxSpinEvent &){
    need_update_config = 1;
  }

  void OnButtonAdd(wxCommandEvent&){
    const wxString& dir = wxDirSelector(_("Choose a plugin directory"),
                                        wxT(""),
                                        0,
                                        wxDefaultPosition,
                                        this);
    if(!dir.empty()){
      list_plugindirs->Append(dir);
      need_update_config = 1;
    }
  }

  void OnButtonRemove(wxCommandEvent&){
    int i = list_plugindirs->GetSelection();
    if(i != wxNOT_FOUND){
      list_plugindirs->Delete(i);
      need_update_config = 1;
    }
  }

  void OnButtonMoveup(wxCommandEvent&){
    int i = list_plugindirs->GetSelection();
    if(i != wxNOT_FOUND && i > 0){
      wxString s = list_plugindirs->GetString(i - 1);
      list_plugindirs->Delete(i - 1);
      list_plugindirs->Insert(s, i);
      need_update_config = 1;
    }
  }

  void OnButtonMovedown(wxCommandEvent&){
    int i = list_plugindirs->GetSelection();
    if((i != (int)wxNOT_FOUND) &&
       (i < (int)list_plugindirs->GetCount() - 1)){
      wxString s = list_plugindirs->GetString(i + 1);
      list_plugindirs->Delete(i + 1);
      list_plugindirs->Insert(s, i);
      need_update_config = 1;
    }
  }

  void OnOk(wxCommandEvent &){
    if(need_update_config){
      // language code
      wxString choice_lang_str = choice_lang->GetStringSelection();
      for(int i = 0;
          i < configmgr->supported_languages_size;
          ++i){
        if(choice_lang_str == configmgr->supported_languages_wxstr[i]){
          configmgr->set_langcode(configmgr->supported_languages[i]);
        }
      }

      // themename
      configmgr->set_themename(choice_themename->GetStringSelection());

      // plugin dirs
      std::vector<wxString> plugindirs;
      for(size_t i = 0; i < list_plugindirs->GetCount(); ++i){
        plugindirs.push_back(list_plugindirs->GetString(i));
      }
      configmgr->set_plugindirs(plugindirs);

      // server port
      configmgr->set_serverport(spin_game_server_port->GetValue());

      // gamespeed
      wxString gamespeed_str = choice_gamespeed->GetStringSelection();
      unsigned long gamespeed;
      if(gamespeed_str.ToULong(&gamespeed)){
        configmgr->set_gamespeed(gamespeed);
      }

      EndModal(0);
    }else{
      EndModal(1);
    }
  }
protected:
  DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(ConfigDialog, wxDialog)
EVT_CHOICE(XRCID("choice_lang"), ConfigDialog::CommandUpdateConfig)
EVT_CHOICE(XRCID("choice_themename"), ConfigDialog::CommandUpdateConfig)
EVT_SPINCTRL(XRCID("game_server_port"), ConfigDialog::SpinContrlUpdateConfig)
EVT_CHOICE(XRCID("choice_gamespeed"), ConfigDialog::CommandUpdateConfig)

EVT_BUTTON(XRCID("plugindir_add"), ConfigDialog::OnButtonAdd)
EVT_BUTTON(XRCID("plugindir_remove"), ConfigDialog::OnButtonRemove)
EVT_BUTTON(XRCID("plugindir_moveup"), ConfigDialog::OnButtonMoveup)
EVT_BUTTON(XRCID("plugindir_movedown"), ConfigDialog::OnButtonMovedown)
EVT_BUTTON(wxID_OK, ConfigDialog::OnOk)
END_EVENT_TABLE()

int ConfigManagerImpl::show_config_dialog(ThemeManager *theme, wxWindow *parent){
  ConfigDialog *dialog = new ConfigDialog(this, theme, parent);
  int rt = dialog->ShowModal();
  delete dialog;
  return rt;
}


ConfigManager* ConfigManager::Create(){
  return new ConfigManagerImpl();
}

