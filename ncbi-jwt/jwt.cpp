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

#include <ncbi/jwt.hpp>
#include "base64-priv.hpp"

#include <iostream>

namespace ncbi
{
    JWT JWT :: make ()
    {
        // make the header
        JSONObject *header = JSONObject :: make ();
        JSONObject *payload = JSONObject :: make ();
        
        JWT jwt ( header, payload );
        
        return jwt;
    }
    
    JWT JWT :: decode ( const std :: string & encoding, const std :: string & pub_key )
    {
        // make sure there is at least one '.'
        size_t dot = encoding . find_first_of ('.', 0 );
        if ( dot == std :: string :: npos )
            throw std :: logic_error ( "Invalid encoded string" );
        
        std :: string hdr = encoding . substr ( 0, dot - 0 );
            
        JSONObject *header = JSONObject :: make ();
        JSONObject *payload = JSONObject :: make ();
        
        JWT jwt ( header, payload );
        
        return jwt;
    }
    
    std :: string JWT :: encode () const
    {
        std :: string encoding = encodeBase64URL ( hdr -> toJSON () ) + ".";
        encoding += encodeBase64URL( pay -> toJSON() ) + ".";
        
        return encoding;
    }
    
    JWT & JWT :: operator = ( const JWT & jwt )
    {
        hdr = jwt . hdr;
        pay = jwt . pay;
        
        return *this;
    }
    
    JWT :: JWT ( const JWT & jwt )
    : hdr ( jwt . hdr )
    , pay ( jwt . pay )
    {
    }
    
    JWT :: JWT ( JSONObject * hdr, JSONObject * pay )
    : hdr ( hdr )
    , pay ( pay )
    {
    }
    
    JWT :: ~JWT ()
    {
    }
}

