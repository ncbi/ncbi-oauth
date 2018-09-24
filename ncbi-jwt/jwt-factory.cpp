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
    
    // Decoding follows RFC 7519: section 7.2
    JWTClaims JWTFactory :: decode ( const JWSFactory & jws_fact, const JWT & jwt, long long cur_time, long long skew ) const
    {
        // RFC 7519: section 1
        // JWTs are always represented using the JWS Compact Serialization or the JWE Compact Serialization.
        
        // 1. verify that the JWT contains at least one '.'
        size_t pos = 0;
        size_t p = jwt . find_first_of ( '.' );
        if ( p == std :: string :: npos )
            throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: '.'" );
        
        // 2. split off the JOSE header from the start of "jwt" to the period.
        // this must be a base64url-encoded string representing a JSONObject
        std :: string header = jwt . substr ( pos, p - pos );
        pos = ++ p;
        
        // 3. run decodeBase64URL() on the JOSE string
        // this will produce raw JSON text
        try
        {
            size_t bytes = 0;
            char * data = ( char * ) decodeBase64URL ( header, & bytes );
            header = std :: string ( data, bytes );
            delete [] data;
        }
        catch ( ... )
        {
            // any error in base64URL encoding is an invalid JWT/JOSE object
            // TBD - capture more information about what is wrong and include it in the new exception
            throw JWTException ( __func__, __LINE__, "Invalid JWT - illegal JOSE base64URL encoding" );
        }
        
        // 4. trust JSON parser enough to parse the raw JSON text of the JOSE header
        // use restricted limits
        JSONValue :: Limits lim;
        JSONObject *jose = nullptr;
        try
        {
            lim . recursion_depth = 1;
            jose = JSONObject :: make ( lim, header );
        }
        catch ( ... )
        {
            // any error in JSON format is an invalid JWT/JOSE object
            // TBD - capture more information about what is wrong and include it in the new exception
            throw JWTException ( __func__, __LINE__, "Invalid JWT - malformed JOSE JSON" );
        }
        
        try
        {
            // 5. count the number of compact segments
            unsigned int period_count = 1; // already have the header
            
            // "p" points to just after the first period
            p = jwt . find_first_of ( '.', p );
            
            // retain the end of payload for JWS
            size_t pay_pos = p;
            
            while ( p != std :: string :: npos )
            {
                if ( ++ period_count > 4 )
                {
                    throw JWTException ( __func__, __LINE__, "Invalid JWT - excessive number of sections." );
                }
                p = jwt . find_first_of ( '.', p + 1 );
            }
            
            // 6. detect JWS or JWE
            // RFC 7516 section 9
            // "The JOSE Header for a JWS can also be distinguished from the JOSE
            // Header for a JWE by determining whether an "enc" (encryption
            // algorithm) member exists.  If the "enc" member exists, it is a
            // JWE; otherwise, it is a JWS."
            if ( jose -> exists ( "enc" ) )
            {
                // compact JWE has 5 sections
                if ( period_count != 4 )
                    throw JWTException ( __func__, __LINE__, "Invalid JWT - malformed JWE." );
                
                // TBD
                throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - JWE is not supported at this time." );
            }
            
            // compact JWS has 3 sections
            if ( period_count != 2 )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - malformed JWS." );
            
            // 7. let the JWSFactory validate the JOSE header and signature - RFC 7515: Section 5.2
            // "Verify that the resulting JOSE Header includes only parameters and values whose syntax and
            // semantics are both understood and supported or that are specified as being ignored when not understood."
            jws_fact . validate ( * jose, jwt );
            
            // 8. check for header member "cty"
            // if exists, the payload is a nested JWT and need to repeat the previous steps
            if ( jose -> exists( "cty" ) )
            {
                delete jose;
                jose = nullptr;
                return decode ( jws_fact, jwt . substr ( pos, pay_pos - pos ), cur_time, skew );
            }
            
            // done with "jose"
            delete jose;
            jose = nullptr;
            
            // 9. decode payload
            size_t bytes = 0;
            char * pay_data = nullptr;
            try
            {
                pay_data = ( char * ) decodeBase64URL ( jwt . substr ( pos, pay_pos - pos ), & bytes );
            }
            catch ( ... )
            {
                // any error in base64URL encoding is an invalid JWT object
                // TBD - capture more information about what is wrong and include it in the new exception
                throw JWTException ( __func__, __LINE__, "Invalid JWT - illegal base64URL encoding" );
            }
            std :: string pay ( pay_data, bytes );
            delete [] pay_data;
            
            // 10. trust JSON parser enough to parse the raw JSON text of the payload
            lim . recursion_depth = 100; // TBD - determine valid limit
            JSONObject *payload = JSONObject :: make ( lim, pay );
            try
            {

                // create claims from JSON payload
                JWTClaims claims ( payload );

                // claim set is already built, but not validated
                // TBD - validate claims, mark protected claims as final
                claims . validate ( cur_time, skew );
            
#if 0
                // this stuff is likely to be part of JWTClaims :: validate ()
                claims . setIssuer ( payload -> getValue ( "iss" ) . toString () );
                claims . setSubject ( payload -> getValue ( "sub" ) . toString () );
                JSONArray aud = payload -> getValue ( "aud" ) . toArray ();
                for ( size_t i = 0; i < aud . count (); ++ i )
                {
                    claims . addAudience ( aud . getValue ( i ) . toString () );
                }
                
                // test validity of JWT based upon time
                // TBD - must handle cases where claim might be optional
                // right now if any are missing, they'll cause an exception to be thrown
                if ( claims . getClaim ( "iat" ) . toInteger () > cur_time )
                    throw "claims issued in the future";
                if ( claims . getClaim ( "nbf" ) . toInteger () > cur_time )
                    throw "claims are not yet valid";
                if ( claims . getClaim ( "exp" ) . toInteger () <= cur_time )
                    throw "claims have expired";
#endif
                
                return claims;
            }
            catch ( ... )
            {
                delete payload;
                throw;
            }
        }
        catch ( ... )
        {
            delete jose;
            throw;
        }
    }
    
    JWT JWTFactory :: sign ( const JWSFactory & jws_fact, const JWTClaims & claims ) const
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

