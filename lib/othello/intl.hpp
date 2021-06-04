/**
 * @file     lib/othello/intl.hpp
 * @summary  othello international support
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: intl.hpp,v 1.1 2007/06/16 03:41:59 hellwolfmisty Exp $
 */


#ifndef LIB_OTHELLO_INTL_HPP
#define LIB_OTHELLO_INTL_HPP

#include "intl_config.hpp"

#  define _(String) (othello::gettext(String))
#  define N_(String) (String)

#endif

