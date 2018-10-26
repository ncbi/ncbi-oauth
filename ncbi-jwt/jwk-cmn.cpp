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

#include <ncbi/json.hpp>
#include <ncbi/jwt.hpp>
#include <ncbi/jws.hpp>
#include <ncbi/jwk.hpp>
#include "base64-priv.hpp"

#include <iostream>

namespace ncbi
{
    JWK * JWK :: parse ( const std :: string & json_text )
    {
        JWK * jwk = nullptr;

        // TBD - add limits : keys have known depths
        JSONObject * props = JSONObject :: parse ( json_text );
        try
        {
            // examine the type
            std :: string kty = props -> getValue ( "kty" ) . toString ();
            if ( kty . compare ( "oct" ) == 0 )
                jwk = HMAC_JWKey :: make ( props );
            else if ( kty . compare ( "RSA" ) == 0 )
            {
                if ( props -> exists ( "use" ) )
                    jwk = RSAPublic_JWKey :: make ( props );
                else
                    jwk = RSAPrivate_JWKey :: make ( props );
            }
            else if ( kty . compare ( "ES" ) == 0 )
            {
                if ( props -> exists ( "use" ) )
                    jwk = EllipticCurvePublic_JWKey :: make ( props );
                else
                    jwk = EllipticCurvePrivate_JWKey :: make ( props );
            }
            else
            {
                std :: string what ( "bad kty value for JWK: '" );
                what += kty;
                what += "'";
                throw JWTException ( __func__, __LINE__, what . c_str () );
            }
        }
        catch ( ... )
        {
            delete props;
            throw;
        }

        return jwk;
    }

    // "kty"
    //  MANDATORY in a JWK (section 4.1)
    //  legal values "oct", "RSA", "EC"
    std :: string JWK :: getType () const
    {
        return props -> getValue ( "kty" ) . toString ();
    }

    // "kid"
    //  optional (section 4.5), but
    //  our library currently makes it MANDATORY
    std :: string JWK :: getID () const
    {
        return props -> getValue ( "kid" ) . toString ();
    }

    // "alg"
    //  identifies the algorithm (section 4.4)
    std :: string JWK :: getAlg () const
    {
        return props -> getValue ( "alg" ) . toString ();
    }

    void JWK :: setAlg ( const std :: string & alg )
    {
        // TBD - check "alg" against known algorithms
        props -> setValueOrDelete ( "alg", JSONValue :: makeString ( alg ) );
    }

    // "use"
    //  only for public keys (section 4.2)
    //  legal values are "sig" (signature) and "enc" (encryption)
    std :: string JWK :: getUse () const
    {
        return props -> getValue ( "use" ) . toString ();
    }

    void JWK :: setUse ( const std :: string & use )
    {
        if ( use . compare ( "sig" ) != 0 &&
             use . compare ( "enc" ) != 0 )
        {
            std :: string what ( "illegal usa value for JWK: '" );
            what += use;
            what += "'";
            throw JWTException ( __func__, __LINE__, what . c_str () );
        }

        props -> setValueOrDelete ( "use", JSONValue :: makeString ( use ) );
    }

    std :: string JWK :: toJSON () const
    {
        return props -> toJSON ();
    }

    // primitive memory management
    JWK * JWK :: duplicate ()
    {
        ++ refcount;
        return this;
    }

    const JWK * JWK :: duplicate () const
    {
        ++ refcount;
        return this;
    }

    void JWK :: release () const
    {
        if ( -- refcount == 0 )
            delete this;
    }

    JWK :: ~ JWK ()
    {
        props -> invalidate ();
        delete props;
    }

    JWK :: JWK ( const std :: string & kid, const std :: string & kty )
        : props ( nullptr )
        , refcount ( 1 )
    {
        props = JSONObject :: make ();
        props -> setFinalValueOrDelete ( "kty", JSONValue :: makeString ( kty ) );
        props -> setFinalValueOrDelete ( "kid", JSONValue :: makeString ( kid ) );
    }

    JWK :: JWK ( JSONObject * _props )
        : props ( _props )
        , refcount ( 1 )
    {
    }


    bool JWKSet :: isEmpty () const
    {
        return map . empty ();
    }

    unsigned long int JWKSet :: count () const
    {
        return map . size ();
    }

    bool JWKSet :: contains ( const std :: string & kid ) const
    {
        auto it = map . find ( kid );

        if ( it == map . end () )
                return false;
        
        return true;
    }

    std :: vector < std :: string > JWKSet :: getKeyIDs () const
    {
        std :: vector < std :: string > ids;
        
        for ( auto it = map . cbegin(); it != map . cend (); ++ it )
            ids . push_back ( it -> first );
        
        return ids;
    }

    void JWKSet :: addKey ( JWK * jwk )
    {
        if ( jwk != nullptr )
        {
            std :: string kid = jwk -> getID ();
            auto it = map . find ( kid );
            if ( it != map . end () )
            {
                std :: string what ( "duplicate key id: '" );
                what += kid;
                what += "'";
                throw JWTException ( __func__, __LINE__, what . c_str () );
            }

            JSONObject * props = ( JSONObject * ) jwk -> props -> clone ();
            try
            {
                map . emplace ( kid, jwk );
                kset -> getValue ( "keys" ) . toArray () . appendValue ( props );
            }
            catch ( ... )
            {
                delete props;
                throw;
            }
        }
    }

    JWK & JWKSet :: getKey ( const std :: string & kid )
    {
        auto it = map . find ( kid );
        if ( it == map . end () )
        {
            std :: string what ( "key not found: id = '" );
            what += kid;
            what += "'";
            throw JWTException ( __func__, __LINE__, what . c_str () );
        }

        return * it -> second;
    }

    const JWK & JWKSet :: getKey ( const std :: string & kid ) const
    {
        auto it = map . find ( kid );
        if ( it == map . end () )
        {
            std :: string what ( "key not found: id = '" );
            what += kid;
            what += "'";
            throw JWTException ( __func__, __LINE__, what . c_str () );
        }

        return * it -> second;
    }

    void JWKSet :: removeKey ( const std :: string & kid )
    {
        throw JWTException ( __func__, __LINE__, "UNIMPLEENTED" );
    }

    JWKSet :: JWKSet ( const JWKSet & ks )
        : kset ( nullptr )
    {
        kset = ( JSONObject * ) ks . kset -> clone ();
        if ( ! ks . map . empty () )
        {
            for ( auto it = ks . map . begin (); it != ks . map . end (); )
            {
                JWK * jwk = it -> second -> duplicate ();
                try
                {
                    map . emplace ( it -> first, jwk );
                }
                catch ( ... )
                {
                    jwk -> release ();
                    throw;
                }
            }
        }
    }

    JWKSet & JWKSet :: operator = ( const JWKSet & ks )
    {
        delete kset;
        kset = ( JSONObject * ) ks . kset -> clone ();

        if ( ! map . empty () )
        {
            for ( auto it = map . begin(); it != map . end (); )
            {
                it -> second -> release ();
                it = map . erase ( it );
            }
        }

        if ( ! ks . map . empty () )
        {
            for ( auto it = ks . map . begin (); it != ks . map . end (); )
            {
                JWK * jwk = it -> second -> duplicate ();
                try
                {
                    map . emplace ( it -> first, jwk );
                }
                catch ( ... )
                {
                    jwk -> release ();
                    throw;
                }
            }
        }
    }

    JWKSet :: JWKSet ()
        : kset ( nullptr )
    {
        kset = JSONObject :: make ();
        kset -> setValueOrDelete ( "keys", JSONArray :: make () );
    }

    JWKSet :: ~ JWKSet ()
    {
        if ( ! map . empty () )
        {
            for ( auto it = map . begin(); it != map . end (); )
            {
                it -> second -> release ();
                it = map . erase ( it );
            }
        }

        kset -> invalidate ();
        delete kset;
    }

}

