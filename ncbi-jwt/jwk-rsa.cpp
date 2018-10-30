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
    RSAPublic_JWKey * RSAPublic_JWKey :: make ( const std :: string & kid, const std :: string & use )
    {
        RSAPublic_JWKey * key = new RSAPublic_JWKey ( kid );
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

    // modulus "n"
    std :: string RSAPublic_JWKey :: getModulus () const
    {
        return props -> getValue ( "n" ) . toString ();
    }

    void RSAPublic_JWKey :: setModulus ( const std :: string & n )
    {
        props -> setValueOrDelete ( "n", JSONValue :: makeString ( n ) );
    }

    // exponent "e"
    std :: string RSAPublic_JWKey :: getExponent () const
    {
        return props -> getValue ( "e" ) . toString ();
    }

    void RSAPublic_JWKey :: setExponent ( const std :: string & e )
    {
        props -> setValueOrDelete ( "e", JSONValue :: makeString ( e ) );
    }

    RSAPublic_JWKey * RSAPublic_JWKey :: make ( JSONObject * props )
    {
        return new RSAPublic_JWKey ( props );
    }

    // "kty" = "RSA"
    RSAPublic_JWKey :: RSAPublic_JWKey ( const std :: string & kid )
        : JWK ( kid, "RSA" )
    {
    }

    RSAPublic_JWKey :: RSAPublic_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }

    RSAPrivate_JWKey * RSAPrivate_JWKey :: make ( const std :: string & kid )
    {
    }

    // modulus "n"
    std :: string RSAPrivate_JWKey :: getModulus () const
    {
        return props -> getValue ( "n" ) . toString ();
    }

    void RSAPrivate_JWKey :: setModulus ( const std :: string & n )
    {
        props -> setValueOrDelete ( "n", JSONValue :: makeString ( n ) );
    }

    // exponent "e"
    std :: string RSAPrivate_JWKey :: getExponent () const
    {
        return props -> getValue ( "e" ) . toString ();
    }

    void RSAPrivate_JWKey :: setExponent ( const std :: string & e )
    {
        props -> setValueOrDelete ( "e", JSONValue :: makeString ( e ) );
    }

    // private exponent "d"
    std :: string RSAPrivate_JWKey :: getPrivateExponent () const
    {
        return props -> getValue ( "d" ) . toString ();
    }

    void RSAPrivate_JWKey :: setPrivateExponent ( const std :: string & d )
    {
        props -> setValueOrDelete ( "d", JSONValue :: makeString ( d ) );
    }

    // first prime factor "p"
    std :: string RSAPrivate_JWKey :: getFirstPrimeFactor () const
    {
        return props -> getValue ( "p" ) . toString ();
    }

    void RSAPrivate_JWKey :: setFirstPrimeFactor ( const std :: string & p )
    {
        props -> setValueOrDelete ( "p", JSONValue :: makeString ( p ) );
    }

    // second prime factor "q"
    std :: string RSAPrivate_JWKey :: getSecondPrimeFactor () const
    {
        return props -> getValue ( "q" ) . toString ();
    }

    void RSAPrivate_JWKey :: setSecondPrimeFactor ( const std :: string & q )
    {
        props -> setValueOrDelete ( "q", JSONValue :: makeString ( q ) );
    }

    // first factor CRT exponent "dp"
    std :: string RSAPrivate_JWKey :: getFirstFactorCRTExponent () const
    {
        return props -> getValue ( "dp" ) . toString ();
    }

    void RSAPrivate_JWKey :: setFirstFactorCRTExponent ( const std :: string & dp )
    {
        props -> setValueOrDelete ( "dp", JSONValue :: makeString ( dp ) );
    }

    // second factor CRT exponent "dq"
    std :: string RSAPrivate_JWKey :: getSecondFactorCRTExponent () const
    {
        return props -> getValue ( "dq" ) . toString ();
    }

    void RSAPrivate_JWKey :: setSecondFactorCRTExponent ( const std :: string & dq )
    {
        props -> setValueOrDelete ( "dq", JSONValue :: makeString ( dq ) );
    }

    // first CRT coefficient "qi"
    std :: string RSAPrivate_JWKey :: getFirstCRTCoefficient () const
    {
        return props -> getValue ( "qi" ) . toString ();
    }

    void RSAPrivate_JWKey :: setFirstCRTCoefficient ( const std :: string & qi )
    {
        props -> setValueOrDelete ( "qi", JSONValue :: makeString ( qi ) );
    }

    // other primes "oth"
    //  prime factor "r"
    //  factor CRT exponent "d"
    //  factor CRT coefficient "t"
    JSONObject & RSAPrivate_JWKey :: getOtherPrime ( unsigned int idx ) const
    {
        throw "aaah";
    }

    void RSAPrivate_JWKey :: addOtherPrime ( JSONObject * prime )
    {
    }

    RSAPrivate_JWKey * RSAPrivate_JWKey :: make ( JSONObject * props )
    {
        return new RSAPrivate_JWKey ( props );
    }

    // "kty" = "RSA"
    RSAPrivate_JWKey :: RSAPrivate_JWKey ( const std :: string & kid )
        : JWK ( kid, "RSA" )
    {
    }

    RSAPrivate_JWKey :: RSAPrivate_JWKey ( JSONObject * props )
        : JWK ( props )
    {
    }

}
