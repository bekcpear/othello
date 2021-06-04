/**
 * @file     player/lowai_plugin.cpp
 * @summary  lowaip player plugin stub code
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: lowai_plugin.cpp,v 1.7 2009/07/05 05:22:04 hellwolfmisty Exp $
 */

#include "othello/player_plugin.hpp"
#include "othello/intl.hpp"

#include "lowai.hpp"

static int create_player(othello_player_plugin_node *phdl,
                         arg_node *args,
                         othello::Player** p,
                         char **errstr){
  (void)phdl;
  (void)args;
  (void)errstr;
  *p = new othello::PlayerLowai();
  return 0;
}

static void release_player(othello_player_plugin_node *phdl,
                           othello::Player *p){
  (void)phdl;
  delete p;
}

extern "C" {
    OTHELLO_PLAYER_PLUGIN_FUNCTION
    int othello_player_plugin_init(othello_player_plugin *phdl,
                                   const char * *plugindirs,
                                   char **errstr){
        (void)plugindirs;
        (void)errstr;
        phdl->nodes = new othello_player_plugin_node*[1];
        phdl->nodes[0] = new othello_player_plugin_node;
        phdl->nodes[0]->subname = NULL;
        phdl->nodes[0]->summary = _("Lowai Player");
        phdl->nodes[0]->description = _("Othello Lowai Player. Use simple score measure technology.");
        phdl->nodes[0]->data = NULL;
        phdl->nodes[0]->create_player = &create_player;
        phdl->nodes[0]->release_player = &release_player;
        phdl->num = 1;
        phdl->data = NULL;
        return 0;
    }

    OTHELLO_PLAYER_PLUGIN_FUNCTION
    void othello_player_plugin_fini(othello_player_plugin *phdl){
        (void)phdl;
        delete[] phdl->nodes;
    }
}
