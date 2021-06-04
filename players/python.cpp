/**
 * @file     player/python.cpp
 * @summary  othello python player plugin
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: python.cpp,v 1.4 2009/07/05 19:30:55 hellwolfmisty Exp $
 */

#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <boost/python.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <map>
#include <algorithm>

#include "othello/player_plugin.hpp"
#include "othello/configs.hpp"
#include "othello/intl.hpp"
#include "pyothello/init.hpp"
#include "pyothello/utility.hpp"
#include "pyothello/player.hpp"

using namespace othello;
using namespace boost;
using boost::shared_ptr;

class PlayerCreator;

struct CreatorNode{
  std::string summary;
  std::string description;
  othello_player_plugin_node plugin_node;
  python::object pycreator;
  PlayerCreator* creator;
  std::map<othello::Player*, python::object> pyplayers;
};

class PlayerCreator{
public:
  virtual ~PlayerCreator(){}
  virtual int create(CreatorNode *node,
                     const python::dict &argsdict,
                     othello::Player** player,
                     std::string& errstring) = 0;
};

class heldPlayerCreator : public PlayerCreator{
  PyObject *self;

public:
  heldPlayerCreator(PyObject* _self):self(_self){
  }

  virtual int create(CreatorNode *node,
                     const python::dict &argsdict,
                     othello::Player** player,
                     std::string& errstring){
    python::object rt = python::call_method<python::object>(self, "create", argsdict);
    //return Player directly
    python::extract<pyPlayer*> playerext(rt);
    if(playerext.check()){
      *player = playerext()->get_player().get();
      node->pyplayers[*player] = rt;
      return 0;
    }
    //return (Player, warning_string)
    python::extract<python::tuple> playerext2(rt);
    if(playerext2.check()){
      python::object playerobj2(playerext2());
      if(PyTuple_Size(playerobj2.ptr()) == 2){
        python::object playerobj(playerobj2[0]);
        python::str warnstrobj(playerobj2[1]);
        errstring += PyString_AsString(warnstrobj.ptr());
        *player = (python::extract<pyPlayer*>(playerobj))()->get_player().get();
        node->pyplayers[*player] = playerobj;
        return 0;
      }else{
        errstring += "creator return unknown tuple\n";
        return 1;
      }
    }
    //return error_string
    errstring += PyString_AsString(rt.ptr());
    return 1;
  }
};

class PlayerCreatorsAdmin{
  typedef std::map<std::string, shared_ptr<CreatorNode> > creators_type;

private:
  creators_type creators;

public:
  std::vector<othello_player_plugin_node*> *plugin_nodes;
  std::string *errstring;

  PlayerCreatorsAdmin() :
    plugin_nodes(NULL),
    errstring(NULL){
  }

  static int create_player(othello_player_plugin_node *plugin_node,
                           arg_node *args,
                           othello::Player** player,
                           char **errstr){
    int rt = 0;
    CreatorNode *node = (CreatorNode*)plugin_node->data;
    std::string errstring;
    try{
      PlayerCreator *creator = node->creator;
      python::dict pyargs;
      for(arg_node *anode = args; anode->name != NULL; ++anode){
        pyargs[anode->name] = anode->value;
      }
      rt = creator->create(node, pyargs, player, errstring);
    }catch(...){
      python::handle_exception();
      errstring += python_get_traceback_string();
      rt = 1;
    }
    if(!errstring.empty()){
      *errstr = strdup(errstring.c_str());
    }
    return rt;
  }

  static void release_player(othello_player_plugin_node *plugin_node,
                             othello::Player *player){
    CreatorNode *node = (CreatorNode*)plugin_node->data;
    node->pyplayers.erase(node->pyplayers.find(player));
  }

  int register_player(const char *name,
                       const char *summary,
                       const char *description,
                       python::object pycreator){
    if(creators.find(name) != creators.end()){
      if(errstring){
        *errstring += std::string() + "Python player " + name + " have been registered\n";
      }
      return 1;
    }
    python::extract<PlayerCreator*> pycreatorext(pycreator);
    if(!pycreatorext.check()){
      *errstring += std::string() + "Register_player called with a non PlayerCreator type\n";
      return 1;
    }

    shared_ptr<CreatorNode> node(new CreatorNode());
    creators[name] = node;
    node->summary = summary;
    node->pycreator = pycreator;
    node->description = description;
    node->creator = pycreatorext();

    node->plugin_node.subname = creators.find(name)->first.c_str();
    node->plugin_node.summary = node->summary.c_str();
    node->plugin_node.description = node->description.c_str();
    node->plugin_node.data = node.get();
    node->plugin_node.create_player = &PlayerCreatorsAdmin::create_player;
    node->plugin_node.release_player = &PlayerCreatorsAdmin::release_player;
    if(plugin_nodes){
      plugin_nodes->push_back(&node->plugin_node);
    }
    return 0;
  }
};

extern "C" void initothello();
BOOST_PYTHON_MODULE(othello){
  init_pyothello();

  python::class_<PlayerCreator, heldPlayerCreator, boost::noncopyable>
    ("PlayerCreator")
    .def("create", pure_virtual(&PlayerCreator::create));

  python::object adminclass = python::class_<PlayerCreatorsAdmin, boost::noncopyable>
    ("PlayerCreatorsAdmin")
    .def("register_player", &PlayerCreatorsAdmin::register_player);

  python::scope().attr("creators") = adminclass();
}

static void do_init(othello_player_plugin *phdl,
                    const char* *plugindirs,
                    std::string &errstring){
  python::object main_module(python::handle<>(python::borrowed(PyImport_AddModule("__main__"))));
  python::object main_namespace((main_module.attr("__dict__")));
  python::object othello_module(python::handle<>(PyImport_ImportModule("othello")));

  /* run all python plugin */
  PlayerCreatorsAdmin &admin = python::extract<PlayerCreatorsAdmin&>
    (python::scope(othello_module).attr("creators"));
  std::vector<othello_player_plugin_node*> plugin_nodes;
  admin.errstring = &errstring;
  admin.plugin_nodes = &plugin_nodes;
  for(const char **pplugindir = plugindirs; *pplugindir != NULL; ++pplugindir){
    DIR *plugindir = opendir((std::string()+*pplugindir+"/python").c_str());
    if(plugindir == NULL)continue;
    for(struct dirent *d = readdir(plugindir); d != NULL; d = readdir(plugindir)){
      char *f = d->d_name;
      while(*f)f++;
      if(f - strlen(".py") > d->d_name){
        if(strcmp(f - strlen(".py"), ".py") == 0){
          std::string fullpath = std::string() + *pplugindir + "/python/" + d->d_name;
          std::ifstream file(fullpath.c_str());  
          if(!file){
            errstring += boost::str(boost::format(_("Open python plugin failed : %s"))
                                    % fullpath);
            continue;
          }
          std::ostringstream ostr;
          ostr << file.rdbuf();
          PyRun_String(ostr.str().c_str(), Py_file_input,
                       main_namespace.ptr(), main_namespace.ptr());
          if(PyErr_Occurred()){
            errstring += std::string() + "Running " + d->d_name + " got execption:\n" + python_get_traceback_string();
          }
          file.close();
        }
      }
    }
  }
  admin.plugin_nodes = NULL;
  admin.errstring = NULL;

  phdl->nodes = new othello_player_plugin_node*[plugin_nodes.size()];
  othello_player_plugin_node **pnode = phdl->nodes;
  for(std::vector<othello_player_plugin_node*>::iterator iter = plugin_nodes.begin();
      iter != plugin_nodes.end();
      ++iter){
    *pnode++ = *iter;
  }
  phdl->num = plugin_nodes.size();
}

extern "C" {
    OTHELLO_PLAYER_PLUGIN_FUNCTION
    int othello_player_plugin_init  (othello_player_plugin *phdl,
                                     const char* *plugindirs,
                                     char **errstr) {
        int rt = 0;

        std::string errstring;

        //init python module if needed
        if(!Py_IsInitialized()){
            if(PyImport_AppendInittab((char*)"othello", initothello) == -1){
                errstring = "Failed to add othello to the interpreter's builtin modules\n";
                python::throw_error_already_set();
                return 1;
            }
            Py_Initialize();
        }

        //load all py
        phdl->nodes = NULL;
        phdl->num = 0;
        if(python::handle_exception(boost::bind(&do_init,
                                                phdl,
                                                plugindirs,
                                                boost::ref(errstring)))){
            if(PyErr_Occurred()){
                errstring += python_get_traceback_string();
            }
            rt = 1;
            Py_Finalize();
        }
        if(!errstring.empty()){
            *errstr = strdup(errstring.c_str());
        }

        return rt;
    }

    OTHELLO_PLAYER_PLUGIN_FUNCTION
    void othello_player_plugin_fini(othello_player_plugin *phdl){
        delete[] phdl->nodes;
        Py_Finalize();
    }
}
