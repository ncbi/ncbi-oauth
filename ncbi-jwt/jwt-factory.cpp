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
        
        if ( ! iss . empty() )
            obj -> setFinalValue( "iss", JSONValue :: makeString ( iss ) );
        
        if ( ! sub . empty() )
            obj -> setFinalValue ( "sub", JSONValue :: makeString ( sub ) );
        
        if ( ! aud . empty() )
        {
            size_t count = aud . size ();
            for ( size_t i = 0; i < count; ++ i )
            {
                obj -> setFinalValue ( "array", JSONArray :: make () );
                obj -> getValue ( "array" ) . toArray ()
                . appendValue ( JSONValue :: makeString ( aud [ i ] ) );
            }
        }
        
        if ( duration != -1 )
            claims -> setDuration ( duration );
            
        if ( not_before != -1 )
            claims -> setNotBefore ( not_before );
        
        JWTClaims *claims = new JWTClaims ( obj );
        return *claims;
    }
    
    JWTClaims JWTFactory :: decode ( const JWSFactory & jws_fact, const JWT & jwt ) const
    {
        // determine Base64 or Base64URL
        std :: string decoded;
        size_t pos = jwt . find_first_of ( "+/-_" );
        if ( pos != std :: string :: npos )
        {
            switch ( jwt [ pos ])
            {
                case '+':
                case '/':
                    decoded = decodeBase64 ( jwt );
                    break;
                case '-':
                case '_':
                    decoded = decodeBase64URL ( jwt );
                    break;
            }
        }
        
        JWTClaims *claims = new JWTClaims ();
        
        
        if ( ! iss . empty() )
            claims -> setIssuer ( iss );
        
        if ( ! sub . empty() )
            claims -> setSubject ( sub );
        
        if ( ! aud . empty() )
        {
            size_t count = aud . size ();
            for ( size_t i = 0; i < count; ++ i )
                claims -> addAudience ( aud [ i ] );
        }
        
        if ( duration != -1 )
            claims -> setDuration ( duration );
        
        if ( not_before != -1 )
            claims -> setNotBefore ( not_before );
        
        return *claims;
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

