/**
 * @file     lib/othello/player_plugin.hpp
 * @summary  othello player plugin manager
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player_plugin_manager.hpp,v 1.5 2009/07/05 19:30:56 hellwolfmisty Exp $
 */


#ifndef LIB_OTHELLO_PLAYER_PLUGIN_MANAGER_HPP
#define LIB_OTHELLO_PLAYER_PLUGIN_MANAGER_HPP

#ifdef __WIN32__
# include <windows.h>
#else
# include <dlfcn.h>
#endif
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "player_plugin.hpp"


namespace othello{
#ifdef __WIN32__
# define plugin_postfix ".dll"
  typedef HMODULE plugin_handler;
# define PMdlopen(handle, flag) LoadLibrary(handle)
# define PMdlsym(handle, sym) GetProcAddress(handle, sym)
# define PMdlclose(handle) FreeLibrary(handle)
    char *PMdlerror(HMODULE handle);
#else
# define plugin_postfix ".so"
  typedef void* plugin_handler;
# define PMdlopen dlopen
# define PMdlsym dlsym
# define PMdlclose dlclose
# define PMdlerror(handle) dlerror()
#endif

  class PlayerPluginManager{
  public:
    class Plugin : public othello_player_plugin{
    public:
      int initok;
      plugin_handler hdl;
      othello_player_plugin_fini_funptr pfini;
    protected:
      std::string pluginfile;
    public:
      Plugin(plugin_handler, othello_player_plugin_fini_funptr, const std::string &_pluginfile);
      ~Plugin();
    public:
      void set_initok(){initok = 1;}
      inline std::string get_pluginfilename(){return pluginfile;}
    };

    class PluginNode{
      boost::shared_ptr<Plugin> plugin;
      othello_player_plugin_node *node;
    public:
      PluginNode(boost::shared_ptr<Plugin> _plugin, othello_player_plugin_node* _node) : 
        plugin(_plugin), node(_node){
      }
    public:
      inline othello_player_plugin_node* get_node(){return node;}
      inline boost::shared_ptr<Plugin> get_plugin(){return plugin;}
    };

  private:
    typedef std::map<std::string, boost::shared_ptr<PluginNode> > PluginMap;
    PluginMap plugins;
    std::vector<std::string> plugindirs_vec;
    const char* *plugindirs;

  private:
    void add_plugindir(const std::string &plugindir, std::string &errors);
    int add_plugin(const std::string &name,
                   const std::string &pluginfile,
                   std::string &errorstr);
  public:
    PlayerPluginManager(std::vector<std::string> &_plugindirs_vec);
    ~PlayerPluginManager();

  public:
    void init(std::string &errors);

    typedef PluginMap::iterator plugin_iterator;
    plugin_iterator plugin_begin();
    plugin_iterator plugin_end();

    othello_player_plugin_node* get_plugin(const std::string &name);

    std::string get_pluginfilename(const std::string &name);
  };
};

#endif

