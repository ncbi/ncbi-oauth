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
    
    // Decoding follows RFC 7519: Second 7.2
    JWTClaims JWTFactory :: decode ( const JWSFactory & jws_fact, const JWT & jwt ) const
    {
        // 1. verify that the JWT contains at least one '.'
        size_t pos = 0;
        size_t p = jwt . find_first_of ( "." );
        if ( p == std :: string :: npos )
            throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: '.'" );
        
        // 2. split off the JOSE header from the start of "jwt" to the period.
        // this must be a base64url-encoded string representing a JSONObject
        std :: string header = jwt . substr ( pos, p - pos );
        pos = ++ p;
        
        // 3. run decodeBase64URL() on the JOSE string
        // this will produce raw JSON text
        header = decodeBase64URL ( header );
        
        // 4. trust your JSON parser enough to parse the raw JSON text of the JOSE header
        // use restricted limits
        JSONValue :: Limits lim;
        lim . recursion_depth = 1;
        JSONObject *jose = JSONObject :: make ( lim, header );
        
        // 5. let the JWSFactory validate the JOSE header
        // "5. Verify that the resulting JOSE Header includes only parameters and values whose syntax and
        // semantics are both understood and supported or that are specified as being ignored when not understood."
    
        
        // 6. determine that this is in fact a JWS; we dont support JWE
        int p_count = 1; // already have the header
        size_t pay_pos = 0;
        
        // JWS - only has two '.'
        while ( p != std :: string :: npos )
        {
            if ( p_count < 2 )
                pay_pos = jwt . find_first_of ( ".", p );
            
            ++ p_count;
        }

        switch ( p_count )
        {
            case 2:
                break;
            case 4:
                throw JWTException ( __func__, __LINE__, "Not currently supporting JWE" );
            default:
                throw JWTException ( __func__, __LINE__, "Invalid JWT - does not conform to JWS or JWE" );
        }
        
        // 7. Validate JWS - RFC 7515: Section 5.2
        if ( ! jws_fact . validate () ) // this is missing
            throw JWTException ( __func__, __LINE__, "Failed to validate JWS" );
    
        // 8. check for header member "cty"
        // if exists, the payload is a nested JWT and need to repeat the previous steps
        if ( jose -> exists( "cty" ) )
            return decode ( jws_fact, jwt . substr ( pos, pay_pos - pos ) );
        
        // 9. decode payload
        std :: string pay = decodeBase64URL ( jwt . substr ( pos, pay_pos - pos ) );
        
        // 10. trust your JSON parser enough to parse the raw JSON text of the payload
        JSONObject *payload = JSONObject :: make ( lim, pay );
        
        // build claim set
        JWTClaims claims ( payload );
        claims . setIssuer ( payload -> getValue ( "iss" ) . toString () );
        claims . setSubject ( payload -> getValue ( "sub" ) . toString () );
        JSONArray aud = payload -> getValue ( "aud" ) . toArray ();
        for ( size_t i = 0; i < aud . count (); ++ i )
        {
            claims . addAudience ( aud . getValue ( i ) . toString () );
        }

        return claims;
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

