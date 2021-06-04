/**
 * @file     lib/pyothello/utility.cpp
 * @summary  python wrapper utilities
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: utility.cpp,v 1.1 2007/06/20 14:46:46 hellwolfmisty Exp $
 */

#include <stdexcept>
#include <Python.h>

#include "utility.hpp"

namespace othello{
  std::string python_get_traceback_string() {
    std::string result;
    PyObject *exception, *v, *traceback;
    PyErr_Fetch(&exception, &v, &traceback);
    PyErr_NormalizeException(&exception, &v, &traceback);

    /*
      import traceback
      lst = traceback.format_exception(exception, v, traceback)
      for line in lst:
      result += line
    */
    PyObject* tbstr = PyString_FromString("traceback");
    PyObject* tbmod = PyImport_Import(tbstr);
    if(!tbmod){
      throw new std::runtime_error("Unable to import traceback"
                                   "module. Is your Python installed?");
    }
    PyObject* tbdict = PyModule_GetDict(tbmod);
    PyObject* formatFunc = PyDict_GetItemString(tbdict,
                                                "format_exception");
    if(!formatFunc)throw new std::runtime_error("Can't find traceback.format_exception");
    if(!traceback){
      traceback = Py_None;
      Py_INCREF(Py_None);
    }
    PyObject* args = Py_BuildValue("(OOO)", exception, v,
                                   traceback);
    PyObject* lst = PyObject_CallObject(formatFunc, args);

    for (int i= 0; i<PyList_GET_SIZE(lst); i++) {
      result += PyString_AsString(PyList_GetItem(lst, i));
    }

    Py_DECREF(args);
    Py_DECREF(lst);
    return result;
  }
}
