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

namespace ncbi
{

    void JWTClaimSet :: setIssuer ( const StringOrURI & iss )
    {
        JWTMgr :: validateStringOrURI ( iss );

        JWTLocker locker ( obj_lock );
        claims -> setValue ( "iss", JSON :: makeString ( iss ) );
    }

    void JWTClaimSet :: setSubject ( const StringOrURI & sub )
    {
        JWTMgr :: validateStringOrURI ( sub );

        JWTLocker locker ( obj_lock );
        claims -> setValue ( "sub", JSON :: makeString ( sub ) );
    }

    void JWTClaimSet :: addAudience ( const StringOrURI & aud )
    {
        JWTMgr :: validateStringOrURI ( aud );

        JWTLocker locker ( obj_lock );

        // first time audience gets added as simple string
        if ( ! claims -> exists ( "aud" ) )
            claims -> setValue ( "aud", JSON :: makeString ( aud ) );
        else
        {
            // get previous entry
            JSONValue * p = & claims -> getValue ( "aud" );

            // if it's not an array, convert to an array
            if ( ! p -> isArray () )
            {
                // remove the string
                JSONValueRef r = claims -> removeValue ( "aud" );

                // add in an array
                claims -> addValue ( "aud", JSON :: makeArray () . release () );

                // put the string back into the array as first element
                p = & claims -> getValue ( "aud" );
                JSONArray & a = p -> toArray ();
                a . appendValue ( r );
            }

            JSONArray & auda = p -> toArray ();
            auda . appendValue ( JSON :: makeString ( aud ) );
        }
    }

    void JWTClaimSet :: setDuration ( long long int dur_seconds )
    {
        if ( dur_seconds < 0 )
            throw JWTException ( __func__, __LINE__, "invalid JWT duration" );

        duration = dur_seconds;
    }

    void JWTClaimSet :: setNotBefore ( long long int nbf_seconds )
    {
        if ( nbf_seconds <= 0 )
            throw JWTException ( __func__, __LINE__, "invalid JWT timestamp" );

        not_before = nbf_seconds;
    }

    void JWTClaimSet :: setID ( const std :: string & unique_jti )
    {
        if ( unique_jti . empty () )
            throw JWTException ( __func__, __LINE__, "invalid JWT identifier" );

        JWTLocker locker ( obj_lock );
        claims -> setValue ( "jti", JSON :: makeString ( unique_jti ) );
    }

    StringOrURI JWTClaimSet :: getIssuer () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "iss" ) . toString ();
    }

    StringOrURI JWTClaimSet :: getSubject () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "sub" ) . toString ();
    }

    std :: vector < StringOrURI > JWTClaimSet :: getAudience () const
    {
        std :: vector < StringOrURI > rtn;

        const JSONObject & cs = * claims;
        const JSONValue & audv = cs . getValue ( "aud" );

        if ( audv . isArray () )
        {
            const JSONArray & auda = audv . toArray ();
            unsigned long int i, count = auda . count ();
            for ( i = 0; i < count; ++ i )
            {
                assert ( auda [ i ] . isString () );
                rtn . push_back ( auda [ i ] . toString () );
            }
        }
        else
        {
            assert ( audv . isString () );
            rtn . push_back ( audv . toString () );
        }

        return rtn;
    }

    long long int JWTClaimSet :: getExpiration () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "exp" ) . toInteger ();
    }

    long long int JWTClaimSet :: getDuration () const
    {
        const JSONObject & cs = * claims;
        if ( cs . exists ( "exp" ) )
        {
            long long int exp = cs . getValue ( "exp" ) . toInteger ();
            if ( cs . exists ( "nbf" ) )
                return exp - cs . getValue ( "nbf" ) . toInteger ();
            if ( cs . exists ( "iat" ) )
                return exp - cs . getValue ( "iat" ) . toInteger ();
            return exp;
        }

        return duration;
    }

    long long int JWTClaimSet :: getNotBefore () const
    {
        const JSONObject & cs = * claims;
        if ( cs . exists ( "nbf" ) )
            return cs . getValue ( "nbf" ) . toInteger ();
        if ( cs . exists ( "iat" ) )
            return cs . getValue ( "iat" ) . toInteger ();

        return not_before;
    }

    long long int JWTClaimSet :: getIssuedAt () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "iat" ) . toInteger ();
    }

    std :: string JWTClaimSet :: getID () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "jti" ) . toString ();
    }

    void JWTClaimSet :: addClaim ( const std :: string & name, const JSONValueRef & value )
    {
        if ( reserved . claims . find ( name ) != reserved . claims . end () )
            throw JWTException ( __func__, __LINE__, "'%s' is a reserved claim", name . c_str () );

        JWTLocker locker ( obj_lock );
        claims -> setValue ( name, value );
    }

    std :: vector < std :: string > JWTClaimSet :: getNames () const
    {
        return claims -> getNames ();
    }

    const JSONValue & JWTClaimSet :: getClaim ( const std :: string & name ) const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( name );
    }

    void JWTClaimSet :: addHeader ( const std :: string & name, const JSONValueRef & value )
    {
        if ( reserved . hdrs . find ( name ) != reserved . hdrs . end () )
            throw JWTException ( __func__, __LINE__, "'%s' is a reserved header", name . c_str () );

        JWTLocker locker ( obj_lock );
        jose -> setValue ( name, value );
    }

    std :: vector < std :: string > JWTClaimSet :: getHdrNames () const
    {
        return jose -> getNames ();
    }

    const JSONValue & JWTClaimSet :: getHeader ( const std :: string & name ) const
    {
        const JSONObject & cs = * jose;
        return cs . getValue ( name );
    }

    std :: string JWTClaimSet :: toJSON () const
    {
        JSONObjectRef claims_cpy;
        long long duration_cpy;
        long long not_before_cpy;
        {
            JWTLocker locker ( obj_lock );
            claims_cpy = claims -> cloneObject ();
            duration_cpy = duration;
            not_before_cpy = not_before;
        }

        JWTMgr :: finalizeClaims ( * claims_cpy, duration_cpy, not_before_cpy );
        return claims_cpy -> toJSON ();
    }

    std :: string JWTClaimSet :: readableJSON ( unsigned int indent ) const
    {
        JSONObjectRef claims_cpy;
        long long duration_cpy;
        long long not_before_cpy;
        {
            JWTLocker locker ( obj_lock );
            claims_cpy = claims -> cloneObject ();
            duration_cpy = duration;
            not_before_cpy = not_before;
        }

        JWTMgr :: finalizeClaims ( * claims_cpy, duration_cpy, not_before_cpy );
        return claims_cpy -> readableJSON ( indent );
    }

    JWTClaimSet & JWTClaimSet :: operator = ( const JWTClaimSet & cs )
    {
        jose = cs . jose -> cloneObject ();
        claims = cs . claims -> cloneObject ();
        duration = cs . duration;
        not_before = cs . not_before;

        return * this;
    }

    JWTClaimSet :: JWTClaimSet ( const JWTClaimSet & cs )
        : jose ( cs . jose -> cloneObject () )
        , claims ( cs . claims -> cloneObject () )
        , duration ( cs . duration )
        , not_before ( cs . not_before )
    {
    }

    JWTClaimSet :: ~ JWTClaimSet ()
    {
    }

    JWTClaimSet :: JWTClaimSet ( const JSONObjectRef & _jose, const JSONObjectRef & _claims )
        : jose ( _jose )
        , claims ( _claims )
        , duration ( -1 )
        , not_before ( -1 )
    {
    }

    JWTClaimSet :: JWTClaimSet ( const JWKRef & _key,
            const JSONObjectRef & _jose, const JSONObjectRef & _claims )
        : jose ( _jose )
        , claims ( _claims )
        , verification_key ( _key )
        , duration ( -1 )
        , not_before ( -1 )
    {
    }

    JWTClaimSet :: Reserved JWTClaimSet :: reserved;

    JWTClaimSet :: Reserved :: Reserved ()
    {
        const char * clist [] =
        {
            "iss", "sub", "aud", "exp", "nbf", "iat", "jti"
        };

        for ( size_t i = 0; i < sizeof clist / sizeof clist [ 0 ]; ++ i )
        {
            claims . insert ( std :: string ( clist [ i ] ) );
        }

        const char * hlist [] =
        {
            "alg", "typ", "cty", "kid", "enc"
        };

        for ( size_t i = 0; i < sizeof hlist / sizeof hlist [ 0 ]; ++ i )
        {
            hdrs . insert ( std :: string ( hlist [ i ] ) );
        }
    }

    JWTClaimSet :: Reserved :: ~ Reserved ()
    {
    }
}
