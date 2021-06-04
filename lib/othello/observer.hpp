/**
 * @file     lib/othello/observer.hpp
 * @summary  generic observer pattern class
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: observer.hpp,v 1.2 2007/06/18 08:31:28 hellwolfmisty Exp $
 */

#ifndef LIB_OTHELLO_OBSERVER_HPP
#define LIB_OTHELLO_OBSERVER_HPP

#include <list>
#include <boost/utility.hpp>

namespace othello{
  class SubjectBase : public boost::noncopyable{
  public:
    virtual unsigned long get_subject_typeid()const = 0;
    virtual ~SubjectBase(){}
  };

  template<typename T>
  class Subject : public SubjectBase{
    static int subject_typeid;
  public:
    typedef T ObserverType;

  protected:
    std::list<ObserverType*> observers;

  public:
    virtual unsigned long get_subject_typeid()const{
      return (unsigned long)&subject_typeid;
    }
    int count_observers()const{
      return observers.size();
    }
    int has_observer(const ObserverType *o)const{
      for(typename std::list<ObserverType*>::iterator ob = observers.begin();
          ob != observers.end();
          ++ob){
        if((*ob)->get_observer_id() == o->get_observer_id())return 1;
      }
      return 0;
    }
    void attach_observer(ObserverType *o){
      observers.push_back(o);
      o->observer_subject_attached(this);
    }
    void detach_observer(ObserverType *o){
      o->observer_subject_detached(this);
      for(typename std::list<ObserverType*>::iterator ob = observers.begin();
          ob != observers.end();
          ++ob){
        if((*ob)->get_observer_id() == o->get_observer_id()){
          observers.erase(ob);
          return;
        }
      }
    }
  };
  template<typename T> int Subject<T>::subject_typeid = 0;

  class Observer{
    int observer_id;
  public:
    virtual ~Observer(){}
    unsigned long get_observer_id()const{return (unsigned long)&observer_id;}
    virtual void observer_subject_attached(SubjectBase *){}
    virtual void observer_subject_detached(SubjectBase *){}
  };

#define BEGIN_OBSERVERS_FOREACH(ob, obs)                           \
  {                                                                \
    typeof(obs) obs_bak = obs;                                     \
    for(typeof((obs).begin()) ob = obs_bak.begin();                \
        ob != obs_bak.end();                                       \
        ++ob){

#define END_OBSERVERS_FOREACH }}

}

#endif

