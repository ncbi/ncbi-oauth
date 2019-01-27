/*==============================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#include <ncbi/jwx.hpp>
#include <cstring>
#include <string.h>

namespace ncbi
{
    const char * JWX :: what () const noexcept
    {
        return prepared_what_value;
    }

    const char * JWX :: message () const noexcept
    {
        return prepared_msg_value;
    }

    JWX :: JWX ( const char * func_name, unsigned int line, const char * fmt, ... )
        : prepared_what_value ( "" )
        , prepared_msg_value ( "" )
    {
        va_list args;
        va_start ( args, fmt );
        msgPrintf ( func_name, line, fmt, args );
        va_end ( args );
    }

    JWX :: JWX ( const char * func_name, unsigned int line, const std :: string & _msg )
        : msg ( _msg )
        , prepared_what_value ( "" )
        , prepared_msg_value ( "" )
    {
        createWhatValue ( func_name, line );
    }

    JWX :: ~ JWX ()
    {
        prepared_what_value = nullptr;
        prepared_msg_value = nullptr;
    }

    JWX :: JWX ()
        : prepared_what_value ( "" )
        , prepared_msg_value ( "" )
    {
    }

    void JWX :: msgPrintf ( const char * func_name, unsigned int line, const char * fmt, va_list args )
    {
        if ( fmt == nullptr )
            msg = std :: string ( "INTERNAL ERROR - null message format string" );
        else
        {
            char buffer [ 1 * 1024 ];
            int status = vsnprintf ( buffer, sizeof buffer, fmt, args );
            if ( status < 0 )
            {
                const char err_msg [] = "INTERNAL ERROR - bad format string";
                :: memmove ( buffer, err_msg, sizeof err_msg );
            }
            else if ( ( size_t ) status >= sizeof buffer )
            {
                const char cont_msg [] = "...";
                :: memmove ( & buffer [ sizeof buffer - sizeof cont_msg ], cont_msg, sizeof cont_msg );
            }

            msg = std :: string ( buffer );
        }

        createWhatValue ( func_name, line );
    }

    void JWX :: createWhatValue ( const char * func_name, unsigned int line )
    {
        if ( func_name == nullptr )
            func_name = "NULL-FUNCTION-NAME";

        char buffer [ 5 * 1024 ];
        int status = snprintf ( buffer, sizeof buffer, "%s:%u - %s", func_name, line, msg . c_str () );

        if ( status < 0 || ( size_t ) status >= sizeof buffer )
        {
            const char cont_msg [] = "...";
            :: memmove ( & buffer [ sizeof buffer - sizeof cont_msg ], cont_msg, sizeof cont_msg );
        }

        what_msg = std :: string ( buffer );

        prepared_what_value = what_msg . c_str ();
        prepared_msg_value = msg . c_str ();
    }

}


