/**
 * @file     lib/othello/intl_config.hpp
 * @summary  othello international configurations
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: intl_config.hpp,v 1.1 2007/06/16 03:41:59 hellwolfmisty Exp $
 */

#ifndef LIB_OTHELLO_INTL_CONFIG_HPP
#define LIB_OTHELLO_INTL_CONFIG_HPP

namespace othello{
  const char* gettext_null(const char* msgid);
  const char* dgettext_null(const char* domainname, const char* msgid);

  extern const char* (*gettext)(const char*);
  extern const char* (*dgettext)(const char*, const char*);
}

#endif
