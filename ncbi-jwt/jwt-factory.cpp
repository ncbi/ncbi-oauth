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

namespace ncbi
{
    JWTClaims JWTFactory :: make () const
    {
        JSONObject *obj = JSONObject :: make ();
        
        JWTClaims claims ( obj );

        if ( ! iss . empty() )
            claims . setIssuer ( iss );
        
        if ( ! sub . empty() )
            claims . setSubject ( sub );
        
        if ( ! aud . empty() )
        {
            size_t count = aud . size ();
            for ( size_t i = 0; i < count; ++ i )
            {
                claims . addAudience ( aud [ i ] );
            }
            obj -> getValue ( "aud" ) . toArray () . lock ();
        }
        
        if ( duration != -1 )
            claims . setDuration ( duration );
            
        if ( not_before != -1 )
            claims . setNotBefore ( not_before );
        
        return claims;
    }
    
    JWTClaims JWTFactory :: decode ( const JWSFactory & jws_fact, const JWT & jwt ) const
    {
        // "1. verify that the JWT contains at least one period ('.') character."
        
        // split off the JOSE header from the start of "jwt" to the period.
        // this must be a base64url-encoded string representing a JSONObject
        // "2. Let the Encoded JOSE Header be the portion of the JWT before the first period ('.') character"
        
        // run decodeBase64URL() on the JOSE string
        // this will produce raw JSON text
        // "3. Base64url decode the Encoded JOSE Header following the restriction that no line breaks,
        // whitespace, or other additional characters have been used."
        
        // trust your JSON parser enough to parse the raw JSON text of the JOSE header
        // use restricted limits
        // "4.   Verify that the resulting octet sequence is a UTF-8-encoded representation of a completely
        // valid JSON object conforming to RFC 7159 [RFC7159]; let the JOSE Header be this JSON object."
        
        // let the JWSFactory validate the JOSE header
        // "5. Verify that the resulting JOSE Header includes only parameters and values whose syntax and
        // semantics are both understood and supported or that are specified as being ignored when not understood."
        
        // look at the JOSE header to determine that this is in fact a JWS
        // "6. Determine whether the JWT is a JWS or a JWE using any of the methods described in Section 9 of [JWE]."
        
        // at this point, we should know that there must be a payload section
    }
    
    JWT JWTFactory :: signCompact ( const JWSFactory & jws_fact, const JWTClaims & claims ) const
    {
        return "";
    }
    
    void JWTFactory :: setIssuer ( const StringOrURI & iss )
    {
        this -> iss = iss;
    }
    
    void JWTFactory :: setSubject ( const StringOrURI & sub )
    {
        this -> sub = sub;
    }
    
    void JWTFactory :: addAudience ( const StringOrURI & aud )
    {
        this -> aud . push_back ( aud );
    }
    
    void JWTFactory :: setDuration ( long long int dur_seconds )
    {
        if ( dur_seconds > 0 )
            duration = dur_seconds;
    }
    
    void JWTFactory :: setNotBefore ( long long int nbf_seconds )
    {
        if ( nbf_seconds > 0 )
            duration = nbf_seconds;
    }
    
    JWTFactory & JWTFactory :: operator = ( const JWTFactory & jwt_fact )
    {
        iss = jwt_fact . iss;
        sub = jwt_fact . sub;
        aud = jwt_fact . aud;
        duration = jwt_fact . duration;
        not_before = jwt_fact . not_before;
        
        return *this;
    }
    
    JWTFactory :: JWTFactory ( const JWTFactory & jwt_fact )
    : iss ( jwt_fact . iss )
    , sub ( jwt_fact . sub )
    , aud ( jwt_fact . aud )
    , duration ( jwt_fact . duration )
    , not_before ( jwt_fact . not_before )
    {
    }
    
    JWTFactory :: JWTFactory ()
    : duration ( -1 )
    , not_before ( -1 )
    {
    }
    
    JWTFactory :: ~ JWTFactory ()
    {
        duration = -1;
        not_before = -1;
    }
    
    std :: string JWTFactory :: newJTI () const
    {
        return "";
    }
    
    long long int JWTFactory :: now ()
    {
        return ( long long int ) time ( nullptr );
    }
}

