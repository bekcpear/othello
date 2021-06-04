/**
 * @file     lib/pyothello/utility.hpp
 * @summary  python wrapper utilities
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: utility.hpp,v 1.1 2007/06/20 14:46:46 hellwolfmisty Exp $
 */

#ifndef LIB_PYOTHELLO_UTILITY_HPP
#define LIB_PYOTHELLO_UTILITY_HPP

#include <string>

namespace othello{
  std::string python_get_traceback_string();
}

#endif

