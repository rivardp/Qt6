
/*
 Copyright (C) 1998, 1999, 2001, 2002, 2003, 2004 J?r?me Lecomte

 This file is part of XLW, a free-software/open-source C++ wrapper of the
 Excel C API - http://xlw.sourceforge.net/

 XLW is free software: you can redistribute it and/or modify it under the
 terms of the XLW license.  You should have received a copy of the
 license along with this program; if not, please email xlw-users@lists.sf.net

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*!
\file XlfException.inl
\brief Implements inline methods of XlfException.
*/

// $Id: XlfException.inl 474 2008-03-05 15:40:40Z ericehlers $

#ifdef NDEBUG
#define INLINE inline
#else
#define INLINE
#endif

namespace xlw {

    INLINE XlfException::XlfException(const std::string& what) : what_(what) {}

    INLINE const char* XlfException::what() const throw () { return what_.c_str(); }

}
