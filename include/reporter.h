/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2013 KiCad Developers, see change_log.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef _REPORTER_H_
#define _REPORTER_H_

#include <wx/string.h>
#include <widgets/ui_common.h>

/**
 * @file reporter.h
 * @author Wayne Stambaugh
 * @note A special thanks to Dick Hollenbeck who came up with the idea that inspired
 *       me to write this.
 */

class wxTextCtrl;
class wxHtmlListbox;
class WX_HTML_REPORT_PANEL;


/**
 * REPORTER
 * is a pure virtual class used to derive REPORTER objects from.
 *
 * The purpose of the REPORTER object is to offer a way for a procedural function
 * to report multiple errors without having to:
 * <ul>
 * <li> know too much about the caller's UI, i.e. wx. </li>
 * <li> stop after the first error </li>
 * </ul>
 * the reporter has 4 severity levels (flags) tagging the messages:
 * - information
 * - warning
 * - error
 * - action (i.e. indication of changes - add component, change footprint, etc. )
 * They are indicators for the message formatting and displaying code,
 * filtering is not made here.
 */

class REPORTER {

public:
    /**
     * Location where the message is to be reported.
     * LOC_HEAD messages are printed before all others (typically intro messages)
     * LOC_BODY messages are printed in the middle
     * LOC_TAIL messages are printed after all others (typically status messages)
     */
    enum LOCATION {
        LOC_HEAD = 0,
        LOC_BODY,
        LOC_TAIL
    };

    /**
     * Function Report
     * is a pure virtual function to override in the derived object.
     *
     * @param aText is the string to report.
     * @param aSeverity is an indicator ( RPT_UNDEFINED, RPT_INFO, RPT_WARNING,
     * RPT_ERROR, RPT_ACTION ) used to filter and format messages
     */

    virtual REPORTER& Report( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) = 0;

    /**
     * Function ReportTail
     * Places the report at the end of the list, for objects that support report ordering
     */
    virtual REPORTER& ReportTail( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED )
    {
        return Report( aText, aSeverity );
    }

    /**
     * Function ReportHead
     * Places the report at the beginning of the list for objects that support ordering
     */
    virtual REPORTER& ReportHead( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED )
    {
        return Report( aText, aSeverity );
    }

    REPORTER& Report( const char* aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED );

    REPORTER& operator <<( const wxString& aText ) { return Report( aText ); }
    REPORTER& operator <<( const wxChar* aText ) { return Report( wxString( aText ) ); }
    REPORTER& operator <<( wxChar aChar ) { return Report( wxString( aChar ) ); }
    REPORTER& operator <<( const char* aText ) { return Report( aText ); }

    /**
     * Function HasMessage
     * Returns true if the reporter client is non-empty.
     */
    virtual bool HasMessage() const = 0;

    virtual ~REPORTER()
    {
    }
};


/**
 * WX_TEXT_CTRL_REPORTER
 * is wrapper for reporting to a wxTextCtrl object.
 */
class WX_TEXT_CTRL_REPORTER : public REPORTER
{
    wxTextCtrl* m_textCtrl;

public:
    WX_TEXT_CTRL_REPORTER( wxTextCtrl* aTextCtrl ) :
        REPORTER(),
        m_textCtrl( aTextCtrl )
    {
    }

    virtual ~WX_TEXT_CTRL_REPORTER()
    {
    }

    REPORTER& Report( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    bool HasMessage() const override;
};


/**
 * WX_STRING_REPORTER
 * is a wrapper for reporting to a wxString object.
 */
class WX_STRING_REPORTER : public REPORTER
{
    wxString* m_string;

public:
    WX_STRING_REPORTER( wxString* aString ) :
        REPORTER(),
        m_string( aString )
    {
    }

    virtual ~WX_STRING_REPORTER()
    {
    }

    REPORTER& Report( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    bool HasMessage() const override;
};


/**
 * WX_HTML_PANEL_REPORTER
 * is a wrapper for reporting to a wx HTML window
 */
class WX_HTML_PANEL_REPORTER : public REPORTER
{
    WX_HTML_REPORT_PANEL* m_panel;

public:
    WX_HTML_PANEL_REPORTER( WX_HTML_REPORT_PANEL* aPanel ) :
        REPORTER(),
        m_panel( aPanel )
    {
    }

    virtual ~WX_HTML_PANEL_REPORTER()
    {
    }

    REPORTER& Report( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    REPORTER& ReportTail( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    REPORTER& ReportHead( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    bool HasMessage() const override;
};

/**
 * NULL_REPORTER
 *
 * A singleton reporter that reports to nowhere. Used as to simplify code by
 * avoiding the reportee to check for a non-NULL reporter object.
 */
class NULL_REPORTER : public REPORTER
{
public:
    NULL_REPORTER()
    {
    }

    virtual ~NULL_REPORTER()
    {
    }

    static REPORTER& GetInstance();

    REPORTER& Report( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    bool HasMessage() const override { return false; }
};

/**
 * STDOUT_REPORTER
 *
 * Debug type reporter, forwarding messages to std::cout.
 */
class STDOUT_REPORTER : public REPORTER
{
public:
    STDOUT_REPORTER()
    {
    }

    virtual ~STDOUT_REPORTER()
    {
    }

    static REPORTER& GetInstance();

    REPORTER& Report( const wxString& aText, SEVERITY aSeverity = RPT_SEVERITY_UNDEFINED ) override;

    bool HasMessage() const override { return false; }
};

#endif     // _REPORTER_H_
