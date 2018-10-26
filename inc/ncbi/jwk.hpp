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

#ifndef _hpp_ncbi_oauth_jwk_
#define _hpp_ncbi_oauth_jwk_

#include <string>
#include <vector>
#include <map>

namespace ncbi
{
    // forwards
    class JWKSet;
    class JSONObject;

    // JSON Web Key - RFC 7517

    class JWK
    {
    public:

        static JWK * parse ( const std :: string & json_text );

        // "kty"
        //  MANDATORY in a JWK (section 4.1)
        //  legal values "oct", "RSA", "EC"
        std :: string getType () const;

        // "kid"
        //  optional (section 4.5), but
        //  our library currently makes it MANDATORY
        std :: string getID () const;

        // "alg"
        //  identifies the algorithm (section 4.4)
        std :: string getAlg () const;
        void setAlg ( const std :: string & alg );

        // "use"
        //  only for public keys (section 4.2)
        //  legal values are "sig" (signature) and "enc" (encryption)
        std :: string getUse () const;
        void setUse ( const std :: string & use );

        std :: string toJSON () const;

        // primitive memory management
        JWK * duplicate ();
        const JWK * duplicate () const;
        void release () const;

    protected:

        virtual ~ JWK ();

        JWK ( const std :: string & kid, const std :: string & kty );
        JWK ( JSONObject * props );

        JSONObject * props;
        mutable std :: atomic < unsigned int > refcount;

        friend class JWKSet;
    };

    class JWKSet
    {
    public:

        bool isEmpty () const;
        unsigned long int count () const;

        bool contains ( const std :: string & kid ) const;

        std :: vector < std :: string > getKeyIDs () const;

        void addKey ( JWK * jwk );

        JWK & getKey ( const std :: string & kid );
        const JWK & getKey ( const std :: string & kid ) const;

        void removeKey ( const std :: string & kid );

        JWKSet ( const JWKSet & ks );
        JWKSet & operator = ( const JWKSet & ks );

        JWKSet ();
        ~ JWKSet ();

    private:

        JSONObject * kset;
        std :: map < std :: string, JWK * > map;
    };

    class HMAC_JWKey : public JWK
    {
    public:

        static HMAC_JWKey * make ( const std :: string & kid );

        // get/set symmetric key "k"
        std :: string getValue () const;
        void setValue ( const std :: string & k );

    private:

        static HMAC_JWKey * make ( JSONObject * props );

        // "kty" = "oct"
        HMAC_JWKey ( const std :: string & kid );
        HMAC_JWKey ( JSONObject * props );

        friend class JWK;
    };

    class RSAPublic_JWKey : public JWK
    {
    public:

        static RSAPublic_JWKey * make ( const std :: string & kid, const std :: string & use );

        // modulus "n"
        std :: string getModulus () const;
        void setModulus ( const std :: string & n );

        // exponent "e"
        std :: string getExponent () const;
        void setExponent ( const std :: string & e );

    private:

        static RSAPublic_JWKey * make ( JSONObject * props );

        // "kty" = "RSA"
        RSAPublic_JWKey ( const std :: string & kid );
        RSAPublic_JWKey ( JSONObject * props );

        friend class JWK;
    };

    class RSAPrivate_JWKey : public JWK
    {
    public:

        static RSAPrivate_JWKey * make ( const std :: string & kid );

        // modulus "n"
        std :: string getModulus () const;
        void setModulus ( const std :: string & n );

        // exponent "e"
        std :: string getExponent () const;
        void setExponent ( const std :: string & e );

        // private exponent "d"
        std :: string getPrivateExponent () const;
        void setPrivateExponent ( const std :: string & d );

        // first prime factor "p"
        std :: string getFirstPrimeFactor () const;
        void setFirstPrimeFactor ( const std :: string & p );

        // second prime factor "q"
        std :: string getSecondPrimeFactor () const;
        void setSecondPrimeFactor ( const std :: string & q );

        // first factor CRT exponent "dp"
        std :: string getFirstFactorCRTExponent () const;
        void setFirstFactorCRTExponent ( const std :: string & dp );

        // second factor CRT exponent "dq"
        std :: string getSecondFactorCRTExponent () const;
        void setSecondFactorCRTExponent ( const std :: string & dq );

        // first CRT coefficient "qi"
        std :: string getFirstCRTCoefficient () const;
        void setFirstCRTCoefficient ( const std :: string & qi );

        // other primes "oth"
        //  prime factor "r"
        //  factor CRT exponent "d"
        //  factor CRT coefficient "t"
        JSONObject & getOtherPrime ( unsigned int idx ) const;
        void addOtherPrime ( JSONObject * prime );

    private:

        static RSAPrivate_JWKey * make ( JSONObject * props );

        // "kty" = "RSA"
        RSAPrivate_JWKey ( const std :: string & kid );
        RSAPrivate_JWKey ( JSONObject * props );

        friend class JWK;
    };


    class EllipticCurvePublic_JWKey : public JWK
    {
    public:

        static EllipticCurvePublic_JWKey * make ( const std :: string & kid, const std :: string & use );

        // curve "crv"
        std :: string getCurve () const;
        void setCurve ( const std :: string & crv );

        // X coordinate "x"
        std :: string getXCoordinate () const;
        void setXCoordinate ( const std :: string & x );

        // Y coordinate "y"
        std :: string getYCoordinate () const;
        void setYCoordinate ( const std :: string & y );

    private:

        static EllipticCurvePublic_JWKey * make ( JSONObject * props );

        // "kty" = "EC"
        EllipticCurvePublic_JWKey ( const std :: string & kid );
        EllipticCurvePublic_JWKey ( JSONObject * props );

        friend class JWK;
    };


    class EllipticCurvePrivate_JWKey : public JWK
    {
    public:

        static EllipticCurvePrivate_JWKey * make ( const std :: string & kid );

        // curve "crv"
        std :: string getCurve () const;
        void setCurve ( const std :: string & crv );

        // X coordinate "x"
        std :: string getXCoordinate () const;
        void setXCoordinate ( const std :: string & x );

        // Y coordinate "y"
        std :: string getYCoordinate () const;
        void setYCoordinate ( const std :: string & y );

        // ECC private key "d"
        std :: string getECCPrivateKey () const;
        void setECCPrivateKey ( const std :: string & d );

    private:

        static EllipticCurvePrivate_JWKey * make ( JSONObject * props );

        // "kty" = "EC"
        EllipticCurvePrivate_JWKey ( const std :: string & kid );
        EllipticCurvePrivate_JWKey ( JSONObject * props );

        friend class JWK;
    };

}

#endif /* _hpp_ncbi_oauth_jwk_ */
