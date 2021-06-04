/**
 * @file     lib/othello/intl_config.cpp
 * @summary  othello international configurations
 *
 * (C) 2004-2007 ZC Miao (hellwolf.misty@gmail.com)
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU 
 * General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * $Id: intl_config.cpp,v 1.2 2009/07/05 05:16:23 hellwolfmisty Exp $
 */


#include "intl_config.hpp"

namespace othello{
  const char* gettext_null(const char* msgid){
    return msgid;
  }

  const char* dgettext_null(const char* domainname, const char* msgid){
    (void)domainname;
    return msgid;
  }

  const char* (*gettext)(const char*) = gettext_null;
  const char* (*dgettext)(const char*, const char*) = dgettext_null;
}

