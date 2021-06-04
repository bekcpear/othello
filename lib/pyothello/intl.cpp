/**
 * @file     lib/pyothello/intl.cpp
 * @summary  othello international support python wrapper
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: intl.cpp,v 1.2 2009/07/05 19:30:56 hellwolfmisty Exp $
 */

#include <boost/python.hpp>

#include "othello/intl.hpp"
#include "intl.hpp"

using namespace boost;

namespace othello{
  void wrapper_Intl(){
    python::def("gettext", gettext);
    python::def("dgettext", dgettext);
  }
}

