/*
 Copyright (C) 1998, 1999, 2001, 2002, 2003 J?r?me Lecomte
 Copyright (C) 2007, 2008 Eric Ehlers

 This file is part of xlw, a free-software/open-source C++ wrapper of the
 Excel C API - http://xlw.sourceforge.net/

 xlw is free software: you can redistribute it and/or modify it under the
 terms of the xlw license.  You should have received a copy of the
 license along with this program; if not, please email xlw-users@lists.sf.net

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*!
 * \file Win32StreamBuf.h
 * \brief Class Win32StreamBuf - Forward stream to Win32 debugger
 * \ingroup utils
 */

// $Id: Win32StreamBuf.h 474 2008-03-05 15:40:40Z ericehlers $

#ifndef INC_Win32StreamBuf_H
#define INC_Win32StreamBuf_H

#include <streambuf>

namespace xlw {

    //! Forwards stream to Win32 debugger.
    /*!
    Use iostream::rdbuf method to set Win32StreamBuf.
    */
    class Win32StreamBuf: public std::streambuf
    {
    public:
        //! \name Structors
        //@{
        //! Default constructor
        Win32StreamBuf() {}
        //! Empty destructor
        ~Win32StreamBuf() {}
        //@}
    protected:
        //! \name Implementation
        //@{
        //! Write to the buffer.
        int_type overflow(int_type ch);
        //! Synchronize the buffer.
        int sync();
        //@}
    private:
        //! Redirect output to compiler debug window
        void SendToDebugWindow();
        std::string buf_;

        //! Copy ctor not implemented to prevent use
        Win32StreamBuf(const Win32StreamBuf&);
        //! Assignment otor not implemented to prevent use
        Win32StreamBuf& operator=(const Win32StreamBuf&);
    };

}

#ifdef NDEBUG
#include "Win32StreamBuf.inl"
#endif

#endif // INC_Win32StreamBuf_H

