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
    EllipticCurvePublic_JWKey * EllipticCurvePublic_JWKey :: make ( const std :: string & kid, const std :: string & use )
    {
        EllipticCurvePublic_JWKey * key = new EllipticCurvePublic_JWKey ( kid );
        try
        {
            key -> setUse ( use );
        }
        catch ( ... )
        {
            key -> release ();
            throw;
        }
        return key;
    }

    // curve "crv"
    std :: string EllipticCurvePublic_JWKey :: getCurve () const
    {
        return props -> getValue ( "crv" ) . toString ();
    }

    void EllipticCurvePublic_JWKey :: setCurve ( const std :: string & crv )
    {
        props -> setValueOrDelete ( "crv", JSONValue :: makeString ( crv ) );
    }

    // X coordinate "x"
    std :: string EllipticCurvePublic_JWKey :: getXCoordinate () const
    {
        return props -> getValue ( "x" ) . toString ();
    }

    void EllipticCurvePublic_JWKey :: setXCoordinate ( const std :: string & x )
    {
        props -> setValueOrDelete ( "x", JSONValue :: makeString ( x ) );
    }

    // Y coordinate "y"
    std :: string EllipticCurvePublic_JWKey :: getYCoordinate () const
    {
        return props -> getValue ( "y" ) . toString ();
    }

    void EllipticCurvePublic_JWKey :: setYCoordinate ( const std :: string & y )
    {
        props -> setValueOrDelete ( "y", JSONValue :: makeString ( y ) );
    }

    EllipticCurvePublic_JWKey * EllipticCurvePublic_JWKey :: make ( JSONObject * props )
    {
        return new EllipticCurvePublic_JWKey ( props );
    }

    // "kty" = "EC"
    EllipticCurvePublic_JWKey :: EllipticCurvePublic_JWKey ( const std :: string & kid )
        : JWK ( kid, "EC" )
    {
    }

    EllipticCurvePublic_JWKey :: EllipticCurvePublic_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }

    EllipticCurvePrivate_JWKey * EllipticCurvePrivate_JWKey :: make ( const std :: string & kid )
    {
        return nullptr;
    }

    // curve "crv"
    std :: string EllipticCurvePrivate_JWKey :: getCurve () const
    {
        return props -> getValue ( "crv" ) . toString ();
    }

    void EllipticCurvePrivate_JWKey :: setCurve ( const std :: string & crv )
    {
        props -> setValueOrDelete ( "crv", JSONValue :: makeString ( crv ) );
    }

    // X coordinate "x"
    std :: string EllipticCurvePrivate_JWKey :: getXCoordinate () const
    {
        return props -> getValue ( "x" ) . toString ();
    }

    void EllipticCurvePrivate_JWKey :: setXCoordinate ( const std :: string & x )
    {
        props -> setValueOrDelete ( "x", JSONValue :: makeString ( x ) );
    }

    // Y coordinate "y"
    std :: string EllipticCurvePrivate_JWKey :: getYCoordinate () const
    {
        return props -> getValue ( "y" ) . toString ();
    }

    void EllipticCurvePrivate_JWKey :: setYCoordinate ( const std :: string & y )
    {
        props -> setValueOrDelete ( "y", JSONValue :: makeString ( y ) );
    }

    // ECC private key "d"
    std :: string EllipticCurvePrivate_JWKey :: getECCPrivateKey () const
    {
        return props -> getValue ( "d" ) . toString ();
    }

    void EllipticCurvePrivate_JWKey :: setECCPrivateKey ( const std :: string & d )
    {
        props -> setValueOrDelete ( "d", JSONValue :: makeString ( d ) );
    }

    EllipticCurvePrivate_JWKey * EllipticCurvePrivate_JWKey :: make ( JSONObject * props )
    {
        return new EllipticCurvePrivate_JWKey ( props );
    }

    // "kty" = "EC"
    EllipticCurvePrivate_JWKey :: EllipticCurvePrivate_JWKey ( const std :: string & kid )
        : JWK ( kid, "EC" )
    {
    }

    EllipticCurvePrivate_JWKey :: EllipticCurvePrivate_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }

}
