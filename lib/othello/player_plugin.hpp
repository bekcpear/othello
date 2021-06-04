/**
 * @file     lib/othello/player_plugin.hpp
 * @summary  othello player plugin stubs
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: player_plugin.hpp,v 1.4 2009/07/05 05:19:14 hellwolfmisty Exp $
 */


#ifndef LIB_OTHELLO_PLAYER_PLUGIN_HPP
#define LIB_OTHELLO_PLAYER_PLUGIN_HPP

#include "player.hpp"

#ifdef __WIN32__
# define OTHELLO_PLAYER_PLUGIN_FUNCTION __declspec(dllexport)
#else
# define OTHELLO_PLAYER_PLUGIN_FUNCTION
#endif

extern "C"{

  struct arg_node{
    const char *name;
    const char *value;
  };

  typedef struct _othello_player_plugin_node{
    const char *subname;
    const char *summary;
    const char *description;
    void *data;
    /* 
     * create a new player
     *
     * args is a NULL terminated array
     * errstr is (if) allocated by plugin, and will be freed by caller
     * return 0 on success, return other value on failed
     */
    int (*create_player)(struct _othello_player_plugin_node *plugin_node,
                         arg_node *args,
                         othello::Player**,
                         char **errstr);
    void (*release_player)(struct _othello_player_plugin_node *plugin_node,
                           othello::Player*);
  }othello_player_plugin_node;

  typedef struct _othello_player_plugin{
    othello_player_plugin_node **nodes;
    int num;
    void *data;
  }othello_player_plugin;

  /* 
   * plugin init function
   * 
   * plugindirs is NULL terminated array
   * errstr is (if) allocated by plugin, and will be freed by called
   * return 0 on success, return other value when failed
   */
  typedef int (*othello_player_plugin_init_funptr)(othello_player_plugin *phdl,
                                                   const char* *plugindirs,
                                                   char **errstr);
  typedef void (*othello_player_plugin_fini_funptr)(othello_player_plugin *phdl);

  int othello_player_plugin_init(othello_player_plugin *phdl,
                                  const char* *plugindir,
                                  char **errstr);
  void othello_player_plugin_fini(othello_player_plugin *phdl);
};

#endif

