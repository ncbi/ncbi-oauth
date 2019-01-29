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

    StringOrURI UnverifiedJWTClaimSet :: getIssuer () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "iss" ) . toString ();
    }

    StringOrURI UnverifiedJWTClaimSet :: getSubject () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "sub" ) . toString ();
    }

    std :: vector < StringOrURI > UnverifiedJWTClaimSet :: getAudience () const
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
                rtn . push_back ( auda [ i ] . toString () );
            }
        }
        else
        {
            rtn . push_back ( audv . toString () );
        }

        return rtn;
    }

    long long int UnverifiedJWTClaimSet :: getExpiration () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "exp" ) . toInteger ();
    }

    long long int UnverifiedJWTClaimSet :: getNotBefore () const
    {
        const JSONObject & cs = * claims;
        if ( cs . exists ( "nbf" ) )
            return cs . getValue ( "nbf" ) . toInteger ();
        return cs . getValue ( "iat" ) . toInteger ();
    }

    long long int UnverifiedJWTClaimSet :: getIssuedAt () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "iat" ) . toInteger ();
    }

    std :: string UnverifiedJWTClaimSet :: getID () const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( "jti" ) . toString ();
    }

    std :: vector < std :: string > UnverifiedJWTClaimSet :: getNames () const
    {
        return claims -> getNames ();
    }

    const JSONValue & UnverifiedJWTClaimSet :: getClaim ( const std :: string & name ) const
    {
        const JSONObject & cs = * claims;
        return cs . getValue ( name );
    }

    std :: vector < std :: string > UnverifiedJWTClaimSet :: getHdrNames () const
    {
        return jose -> getNames ();
    }

    const JSONValue & UnverifiedJWTClaimSet :: getHeader ( const std :: string & name ) const
    {
        const JSONObject & cs = * jose;
        return cs . getValue ( name );
    }

    UnverifiedJWTClaimSet :: ~ UnverifiedJWTClaimSet ()
    {
    }

    UnverifiedJWTClaimSet :: UnverifiedJWTClaimSet ( const JSONObjectRef & _jose, const JSONObjectRef & _claims )
        : jose ( _jose )
        , claims ( _claims )
    {
    }

    UnverifiedJWTClaimSet :: UnverifiedJWTClaimSet ( const JWKRef & key, const JSONObjectRef & _jose, const JSONObjectRef & _claims )
        : jose ( _jose )
        , claims ( _claims )
        , verification_key ( key )
    {
    }
}
