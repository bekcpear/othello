/**
 * @file     lib/othello/player_plugin.cpp
 * @summary  othello player plugin manager implementation
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player_plugin_manager.cpp,v 1.6 2009/07/05 05:23:24 hellwolfmisty Exp $
 */

#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <boost/format.hpp>

#include "configs.hpp"
#include "intl.hpp"

#include "player_plugin_manager.hpp"
#include "stdio.h"
namespace othello{

#ifdef __WIN32__
    char *PMdlerror(HMODULE handle) {
        static char dlerror_str[1000];
        DWORD dw = GetLastError();

        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
            (LPTSTR)&dlerror_str,
            sizeof(dlerror_str), NULL );

        return dlerror_str;
    }
#endif

  PlayerPluginManager::Plugin::Plugin(plugin_handler h,
                                      othello_player_plugin_fini_funptr pf,
                                      const std::string &_pluginfile){
    initok = 0;
    hdl = h;
    pfini = pf;
    pluginfile = _pluginfile;
  }

  PlayerPluginManager::Plugin::~Plugin(){
    if(initok){
      pfini(this);
      PMdlclose(hdl);
    }
  }


  PlayerPluginManager::PlayerPluginManager(std::vector<std::string> &_plugindirs_vec) :
    plugindirs_vec(_plugindirs_vec){
    plugindirs_vec.push_back(playerpluginsdir);
    if(strcmp(playerpluginsdir, compatible_playerpluginsdir))
      plugindirs_vec.push_back(compatible_playerpluginsdir);

    plugindirs = new const char*[plugindirs_vec.size()+1];
    const char* *p = plugindirs;
    for(int i = 0; i < int(plugindirs_vec.size()); ++i){
      *p++ = plugindirs_vec[i].c_str();
    }
    *p = NULL;
  }

  PlayerPluginManager::~PlayerPluginManager(){
    delete[] plugindirs;
  }

  void PlayerPluginManager::init(std::string &errors){
    for(int i = 0; i < int(plugindirs_vec.size()); ++i){
      add_plugindir(plugindirs_vec[i], errors);
    }
  }

  void PlayerPluginManager::add_plugindir(const std::string &plugindir, std::string &errors){
    //find all plugins
    DIR *pd = opendir(plugindir.c_str());
    if(pd == NULL){
      errors += boost::str(boost::format(_("plugins dir %1% not exists\n")) % plugindir);
      return;
    }
    for(struct dirent *d = readdir(pd); d != NULL; d = readdir(pd)){
      char *f = d->d_name;
      while(*f)f++;
      if(f - strlen(plugin_postfix) > d->d_name){
        if(strcmp(f - strlen(plugin_postfix), plugin_postfix) == 0){
          add_plugin(std::string(d->d_name, f - d->d_name - strlen(plugin_postfix)),
                     plugindir + '/' + d->d_name,
                     errors);
        }
      }
    }
    closedir(pd);
  }

  int PlayerPluginManager::add_plugin(const std::string &name,
                                      const std::string &pluginfile,
                                      std::string &errorstr){
    plugin_handler hdl;
    hdl = PMdlopen(pluginfile.c_str(), RTLD_NOW);
    if(hdl == NULL){
      errorstr += boost::str(boost::format(_("PMdlopen %1% failed : %2%\n"))
                              % pluginfile.c_str() % PMdlerror(hdl));
      return -1;
    }
    othello_player_plugin_init_funptr pinit = 
      (othello_player_plugin_init_funptr) PMdlsym(hdl, "othello_player_plugin_init");
    if(pinit == NULL){
      errorstr += boost::str(boost::format(_("PMdlsym %1% init function failed : %2%\n"))
                              % pluginfile.c_str() % PMdlerror(hdl));
      PMdlclose(hdl);
      return -1;
    }
    othello_player_plugin_fini_funptr pfini = 
      (othello_player_plugin_fini_funptr) PMdlsym(hdl, "othello_player_plugin_fini");
    if(pinit == NULL){
      errorstr += boost::str(boost::format(_("PMdlsym %1% fini function failed : %2%\n"))
                              % pluginfile.c_str() % PMdlerror(hdl));
      PMdlclose(hdl);
      return -1;
    }
    boost::shared_ptr<Plugin> p(new Plugin(hdl, pfini, pluginfile));
    char *pinit_err = NULL;
    if(pinit(p.get(), plugindirs, &pinit_err)){
      errorstr += boost::str(boost::format(_("plugin %1% init function failed :\n%2%"))
                             % pluginfile.c_str() % (pinit_err == NULL?"(none)":pinit_err));
      PMdlclose(hdl);
      free(pinit_err);
      return -1;
    }
    if(pinit_err){
      errorstr += boost::str(boost::format(_("plugin %1% init function warnings :\n%2%"))
                             % pluginfile.c_str() % (pinit_err == NULL?"(none)":pinit_err));
      free(pinit_err);
    }
    p->set_initok();
    for(int i = 0; i < p->num; ++i){
      std::string pname = name;
      if(p->nodes[i]->subname){
        pname += '/';
        pname += p->nodes[i]->subname;
      }
      if(plugins.find(pname) != plugins.end()){
        errorstr += boost::str(boost::format(_("plugin %1% exists\n")) % pname);
        return -1;
      }
      plugins[pname] = boost::shared_ptr<PluginNode>(new PluginNode(p, p->nodes[i]));
    }

    return 0; 
  }

  PlayerPluginManager::plugin_iterator PlayerPluginManager::plugin_begin(){
    return plugins.begin();
  }

  PlayerPluginManager::plugin_iterator PlayerPluginManager::plugin_end(){
    return plugins.end();
  }

  othello_player_plugin_node* PlayerPluginManager::get_plugin(const std::string &name){
    if(plugins.find(name) == plugins.end())return NULL;
    return plugins[name]->get_node();
  }

  std::string PlayerPluginManager::get_pluginfilename(const std::string &name){
    if(plugins.find(name) == plugins.end())return std::string();
    return plugins[name]->get_plugin()->get_pluginfilename();
  }
}

