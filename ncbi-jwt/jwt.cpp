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
        JWT jwt;

        jwt . header . addMember ( "alg", JSONValue :: makeString ( "RS512" ), true );
        jwt . header . addMember ( "typ", JSONValue :: makeString ( "JWT" ), true );
        
        jwt . payload . addMember ( "iss", JSONValue :: makeString ( "Batman" ), true );
        jwt . payload . addMember ( "sub", JSONValue :: makeString ( "Joker" ), true );
        jwt . payload . addMember ( "aud", JSONValue :: makeString ( "Gotham" ), true );
        jwt . payload . addMember ( "iat", JSONValue :: makeInteger ( 0 ), true );
        jwt . payload . addMember ( "exp", JSONValue :: makeInteger ( 10 ), true );
        
        return jwt;
    }
    
    JWT JWT :: make ( const std :: string & encoding )
    {
        // make sure there is at least one '.'
        size_t dot = encoding . find_first_of ('.', 0 );
        if ( dot == std :: string :: npos )
            throw std :: logic_error ( "Invalid encoded string" );
        
        std :: string header = encoding . substr ( 0, dot - 0 );
            
        JWT jwt;
        return jwt;
    }
    
    std :: string JWT :: encode () const
    {
        std :: string encoding = encodeBase64URL ( header . toJSON () ) + ".";
        encoding += encodeBase64URL( payload.toJSON() ) + ".";
        
        return encoding;
    }
    
    void JWT :: decode ( const std :: string &encoding )
    {
        
    }
    
    JWT & JWT :: operator = ( const JWT & jwt )
    {
        header = jwt . header;
        payload = jwt . payload;
        
        return *this;
    }
    
    JWT :: JWT ( const JWT & jwt )
    : header ( jwt . header )
    , payload ( jwt . payload )
    {
    }
    
    JWT :: ~JWT ()
    {
    }
}

