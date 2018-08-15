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
#include <ncbi/jws.hpp>
#include "base64-priv.hpp"

#include <iostream>
#include <chrono>

namespace ncbi
{
    /* JWTClaims
     *
     **********************************************************************************/
    void JWTClaims :: setIssuer ( const StringOrURI & iss )
    {
        validateStringOrURI ( iss );
        
        claims -> setFinalValue( "iss", JSONValue :: makeString ( iss ) );
    }
    
    void JWTClaims :: setSubject ( const StringOrURI & sub )
    {
        validateStringOrURI ( sub );
        
        claims -> setFinalValue( "sub", JSONValue :: makeString ( sub ) );
    }
    
    void JWTClaims :: addAudience ( const StringOrURI & aud )
    {
        validateStringOrURI ( aud );
        
        if ( ! claims -> exists ( "aud" ) )
            claims -> setFinalValue ( "aud", JSONArray :: make () );
        
        JSONArray array = claims -> getValue ( "aud " ) . toArray ();
        array . appendValue ( JSONValue :: makeString ( aud ) );
    }
    
    void JWTClaims :: setDuration ( long long int dur_seconds )
    {
    }
    
    void JWTClaims :: setNotBefore ( long long int nbf_seconds )
    {
    }
    
    void JWTClaims :: addClaim ( const std :: string & name, JSONValue * value, bool isFinal )
    {
        if ( isFinal )
            claims -> setFinalValue ( name, value );
        else
            claims -> setValue ( name, value );
    }
    
    const JSONValue & JWTClaims :: getClaim ( const std :: string & name ) const
    {
        return claims -> getValue ( name );
    }
    
    JWTClaims & JWTClaims :: operator = ( const JWTClaims & jwt )
    {
        claims = jwt . claims;
        return *this;
    }
    
    JWTClaims :: JWTClaims ( const JWTClaims & jwt )
    : claims ( jwt . claims )
    {
    }
    
    void JWTClaims :: validateStringOrURI ( const std::string &str )
    {
        throw JWTException ( __func__, __LINE__, "Invalid StringOrURI" );
    }
    
    JWTClaims :: JWTClaims ( JSONObject *claims )
    : claims ( claims )
    {
    }
}

