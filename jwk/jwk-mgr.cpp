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

#include <ncbi/jwk.hpp>
#include <ncbi/jwa.hpp>
#include "base64-priv.hpp"

#include <mbedtls/pk.h>
#include <mbedtls/error.h>

#include <cassert>

namespace ncbi
{
    static
    JWKException MBEDTLSException ( const char * func, unsigned int line, int err, const char * msg )
    {
        char buffer [ 256 ];
        mbedtls_strerror ( err, buffer, sizeof buffer );
        return JWKException ( func, line, "%s: %s", msg, buffer );
    }

    JWKSetRef JWKMgr :: makeJWKSet ()
    {
        JSONObjectRef obj = JSON :: makeObject ();
        return JWKSetRef ( new JWKSet ( obj ) );
    }

    static void validateHMAC ( const JSONObject & props )
    {
        // key string
        if ( ! props . getValue ( "k" ) . isString () )
        {
            throw JWKException ( __func__, __LINE__,
                "expected k value as string for JWK" );
        }
    }

    static void validateRSA ( const JSONObject & props )
    {
        const char * pub_props [] = { "d", "e" };

        size_t i;
        for ( i = 0; i < sizeof pub_props / sizeof pub_props [ 0 ]; ++ i )
        {
            if ( ! props . getValue ( pub_props [ i ] ) . isString () )
            {
                throw JWKException ( __func__, __LINE__,
                    "expected %s value as string for JWK", pub_props [ i ] );
            }
        }

        const char * priv_props [] = { "d", "p", "q" };

        assert ( sizeof priv_props / sizeof priv_props [ 0 ] != 0 );
        if ( props . exists ( priv_props [ 0 ] ) )
        {
            for ( i = 0; i < sizeof priv_props / sizeof priv_props [ 0 ]; ++ i )
            {
                if ( ! props . getValue ( priv_props [ i ] ) . isString () )
                {
                    throw JWKException ( __func__, __LINE__,
                        "expected %s value as string for JWK", priv_props [ i ] );
                }
            }
        }
    }

    static void validateEC ( const JSONObject & props )
    {
        const char * pub_props [] = { "crv", "x", "y" };

        size_t i;
        for ( i = 0; i < sizeof pub_props / sizeof pub_props [ 0 ]; ++ i )
        {
            if ( ! props . getValue ( pub_props [ i ] ) . isString () )
            {
                throw JWKException ( __func__, __LINE__,
                    "expected %s value as string for JWK", pub_props [ i ] );
            }
        }

        const char * priv_props [] = { "d" };

        assert ( sizeof priv_props / sizeof priv_props [ 0 ] != 0 );
        if ( props . exists ( priv_props [ 0 ] ) )
        {
            for ( i = 0; i < sizeof priv_props / sizeof priv_props [ 0 ]; ++ i )
            {
                if ( ! props . getValue ( priv_props [ i ] ) . isString () )
                {
                    throw JWKException ( __func__, __LINE__,
                        "expected %s value as string for JWK", priv_props [ i ] );
                }
            }
        }
    }

    static void validateJWK ( const JSONObject & props )
    {
        // key type
        std :: string kty = props . getValue ( "kty" ) . toString ();

        // check the alg
        if ( props . exists ( "alg" ) )
        {
            std :: string alg = props . getValue ( "alg" ) . toString ();
            if ( ! JWAMgr :: acceptJWKAlgorithm ( kty, alg ) )
            {
                throw JWKException ( __func__, __LINE__,
                    "unsupported alg value for JWK: '%s'", alg . c_str () );
            }
        }

        // examine the usage
        if ( props . exists ( "key_ops" ) )
        {
            const JSONArray & ops = props . getValue ( "key_ops" ) . toArray ();
            unsigned int i, count = ops . count ();
            for ( i = 0; i < count; ++ i )
            {
                std :: string op = ops [ i ] . toString ();
                if ( op . compare ( "sign" ) != 0 &&
                     op . compare ( "verify" ) != 0 &&
                     op . compare ( "encrypt" ) != 0 &&
                     op . compare ( "decrypt" ) != 0 &&
                     op . compare ( "wrapKey" ) != 0 &&
                     op . compare ( "unwrapKey" ) != 0 &&
                     op . compare ( "deriveKey" ) != 0 &&
                     op . compare ( "deriveBits" ) != 0 )
                {
                    throw JWKException ( __func__, __LINE__,
                        "unrecognized key_ops[] value for JWK: '%s'", op . c_str () );
                }
            }
        }
        else if ( props . exists ( "use" ) )
        {
            std :: string use = props . getValue ( "use" ) . toString ();
            if ( use . compare ( "sig" ) != 0 &&
                 use . compare ( "enc" ) != 0 )
            {
                throw JWKException ( __func__, __LINE__,
                    "unrecognized use value for JWK: '%s'", use . c_str () );
            }
        }

        // examine the type
        if ( kty . compare ( "oct" ) == 0 )
            validateHMAC ( props );
        else if ( kty . compare ( "RSA" ) == 0 )
            validateRSA ( props );
        else if ( kty . compare ( "ES" ) == 0 )
            validateEC ( props );
        else
        {
            throw JWKException ( __func__, __LINE__,
                "unrecognized kty value for JWK: '%s'", kty . c_str () );
        }

        // check the kid
        if ( ! props . exists ( "kid" ) )
        {
            throw JWKException ( __func__, __LINE__,
                "expected kid value for JWK" );
        }
    }

    static void validateJWKSet ( const JSONObject & kset )
    {
        std :: set < std :: string > kid_set;

        const JSONArray & keys = kset . getValue ( "keys" ) . toArray ();
        unsigned long int i, count = keys . count ();
        for ( i = 0; i < count; ++ i )
        {
            const JSONObject & props = keys [ i ] . toObject ();
            validateJWK ( props );

            std :: string kid = props . getValue ( "kid" ) . toString ();
            auto it = kid_set . emplace ( kid );
            if ( ! it . second )
            {
                throw JWKException ( __func__, __LINE__,
                    "duplicate kid in JWKSet: %s", kid . c_str () );
            }
        }
    }

    JWKRef JWKMgr :: parseJWK ( const std :: string & json_text )
    {
        // keys have known depths
        JSON :: Limits lim;
        lim . recursion_depth = 20; // TBD - get real limit

        JSONObjectRef props = JSON :: parseObject ( lim, json_text );

        validateJWK ( * props );

        return JWKRef ( new JWK ( props ) );
    }

    JWKSetRef JWKMgr :: parseJWKSet ( const std :: string & json_text )
    {
        // key sets have known depths
        JSON :: Limits lim;
        lim . recursion_depth = 22; // TBD - get real limit

        JSONObjectRef props = JSON :: parseObject ( lim, json_text );

        validateJWKSet ( * props );

        return JWKSetRef ( new JWKSet ( props ) );
    }


    // code to perform some mbedtls magic
    static
    void writeKeyParameter ( JSONObject & props, const char * mbr, mbedtls_mpi & mpi )
    {
        // the MPI ( Multi-precision Integer ) represents some parameter
        // it is intended to be represented in base64url format
        // extract it first into a buffer
        unsigned char buff [ 4096 ], * bp = buff;
        size_t buff_size = sizeof buff;

        // test the actual size of mpi
        size_t mpi_size = mbedtls_mpi_size ( & mpi );

        // allocate a temporary buffer if necessary
        if ( mpi_size > buff_size )
        {
            bp = new unsigned char [ mpi_size ];
            buff_size = mpi_size;
        }
        try
        {
            // write mpi into our buffer
            int status = mbedtls_mpi_write_binary ( & mpi, bp, mpi_size );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to write key parameter" );

            // base64url encode the thing
            std :: string encoded = encodeBase64URL ( ( void * ) bp, mpi_size );

            // write it into the props
            props . setValue ( mbr, JSON :: makeString ( encoded ) );
        }
        catch ( ... )
        {
            if ( bp != buff )
                delete [] bp;
            throw;
        }

        if ( bp != buff )
            delete [] bp;
    }

    JWKRef JWKMgr :: parsePEM ( const std :: string & pem_text,
        const std :: string & use, const std :: string & alg, const std :: string & kid )
    {
        return parsePEM ( pem_text, "", use, alg, kid );
    }

    JWKRef JWKMgr :: parsePEM ( const std :: string & pem_text, const std :: string & pwd,
        const std :: string & use, const std :: string & alg, const std :: string & kid )
    {
        size_t i, start, end;

        // this is a NEVER TERMINATING LOOP
        // by which I mean that it terminates either upon error
        // or upon success, neither of which are at standard exit
        // points of the loop.
        for ( i = end = 0; ; ++ i )
        {
            // locate the start of opening delimiter line
            start = pem_text . find ( "-----BEGIN ", end );
            if ( start == std :: string :: npos )
                throw JWKException ( __func__, __LINE__, "invalid PEM text" );

            // locate the start of the label
            size_t label_start = start + sizeof "-----BEGIN " - 1;

            // locate the end of opening delimiter line
            // we are interested in cases when this ends in " KEY-----"
            // but PEM text can contain multiple entries.
            // regardless, the line MUST end in "-----"
            size_t key_start = pem_text . find ( "-----", label_start );
            if ( key_start == std :: string :: npos )
                throw JWKException ( __func__, __LINE__, "invalid PEM text" );

            // convert into potential start of base64-encoded key string
            // the pem-type keyword we want will be " KEY-----" - 1 bytes behind
            key_start += sizeof "-----" - 1;

            // locate the start of the next delimiter line
            // which should be a closing delimiter line
            // which would also be the end of the key
            size_t key_end = pem_text . find ( "-----", key_start );
            if ( key_end == std :: string :: npos )
                throw JWKException ( __func__, __LINE__, "invalid PEM text" );

            // this should be an "-----END " ... line
            size_t end_label = pem_text . find ( "END ", key_end + 5 );
            if ( end_label != key_end + 5 )
                throw JWKException ( __func__, __LINE__, "invalid PEM text" );

            // the start of the ending label
            end_label += sizeof "END " - 1;

            // locate the end of this delimiter
            size_t end = pem_text . find ( "-----", end_label );
            if ( end == std :: string :: npos )
                throw JWKException ( __func__, __LINE__, "invalid PEM text" );
            end += 5;

            // the delimiter lines should match other than "BEGIN" and "END"
            if ( pem_text . compare ( label_start, key_start - label_start, pem_text, end_label, end - end_label ) != 0 )
                throw JWKException ( __func__, __LINE__, "invalid PEM text" );

            // seems like a legitimate PEM entry - see if it's a KEY entry
            const char type_str [] = " KEY-----";
            const size_t type_str_len = sizeof type_str - 1;
            size_t type_start = key_start - type_str_len;
            if ( pem_text . compare ( type_start, type_str_len, type_str, type_str_len ) == 0 )
            {
                // THIS BLOCK WILL EITHER FAIL OR RETURN A KEY

                JWKRef jwk;
                
                // get the label
                std :: string label = pem_text . substr ( label_start, type_start - label_start );

                // get the full PEM text of this entry
                std :: string key_text = pem_text . substr ( start, end - start );

                // learn whether the key claims to be public or private
                bool key_is_public = false;

                int status = 0;
                mbedtls_pk_context pk;
                mbedtls_pk_init ( & pk );

                // catch exceptions to undo effect of mbedtls_pk_init()
                try
                {
                    // look for a label we support
                    if ( label . compare ( "RSA PRIVATE" ) == 0 ||
                         label . compare ( "EC PRIVATE" ) == 0 )
                    {
                        // NB - mbedtls states:
                        //  "Avoid calling mbedtls_pem_read_buffer() on non-null-terminated string"
                        //  so always pass "c_str()" rather than "data()"
                        // NB - incredibly, the key size they want passed in must include the NUL!!!
                        status = mbedtls_pk_parse_key ( & pk,
                            ( const unsigned char * ) key_text . c_str (), key_text . size () + 1,
                            ( const unsigned char * ) pwd . c_str (), pwd . size () );
                    }
                    else if ( label . compare ( "RSA PUBLIC" ) == 0 ||
                              label . compare ( "PUBLIC" ) == 0 )
                    {
                        // key claims to be public
                        key_is_public = true;
                        
                        // NB - mbedtls states
                        //  "Avoid calling mbedtls_pem_read_buffer() on non-null-terminated string"
                        //  so always pass "c_str()" rather than "data()"
                        // NB - incredibly, the key size they want passed in must include the NUL!!!
                        status = mbedtls_pk_parse_public_key ( & pk,
                            ( const unsigned char * ) key_text . c_str (), key_text . size () + 1 );
                    }
                    else
                    {
                        // this is not a label we support
                        mbedtls_pk_free ( & pk );
                        continue;
                    }

                    // check for mbedtls errors
                    if ( status != 0 )
                        throw MBEDTLSException ( __func__, __LINE__, status, "failed to parse PEM key" );

                    // extract the components
                    mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
                    mbedtls_mpi_init ( & N );
                    mbedtls_mpi_init ( & P );
                    mbedtls_mpi_init ( & Q );
                    mbedtls_mpi_init ( & D );
                    mbedtls_mpi_init ( & E );
                    mbedtls_mpi_init ( & DP );
                    mbedtls_mpi_init ( & DQ );
                    mbedtls_mpi_init ( & QP );
                    
                    try
                    {
                        JSONObjectRef props_ref = JSON :: makeObject ();
                        JSONObject & props = * props_ref;

                        // create a catch block to invalidate stored props
                        try
                        {
                            // set some JSON properties
                            props . setValue ( "use", JSON :: makeString ( use ) );
                            props . setValue ( "alg", JSON :: makeString ( alg ) );
                            props . setValue ( "kid", JSON :: makeString ( kid ) );
                            
                            if ( mbedtls_pk_get_type ( & pk ) == MBEDTLS_PK_RSA )
                            {
                                // set type property
                                props . setValue ( "kty", JSON :: makeString ( "RSA" ) );
                                
                                // extract the RSA context
                                mbedtls_rsa_context * rsa = mbedtls_pk_rsa ( pk );

                                // handle RSA PUBLIC key
                                if ( key_is_public )
                                {
                                    // extract the public-only portions
                                    status = mbedtls_rsa_export ( rsa, & N, nullptr, nullptr, nullptr, & E );
                                    if ( status != 0 )
                                    {
                                        throw MBEDTLSException ( __func__, __LINE__, status,
                                            "mbedtls_rsa_export failed to obtain key parameters" );
                                    }

                                    // write the key parameters into JSON
                                    writeKeyParameter ( props, "n", N );
                                    writeKeyParameter ( props, "e", E );

                                    // create the key
                                    // NB - MUST be last step within try block
                                    jwk = new JWK ( props_ref );
                                }
                                else
                                {
                                    // extract the full RSA portions
                                    status = mbedtls_rsa_export ( rsa, & N, & P, & Q, & D, & E );
                                    if ( status != 0 )
                                    {
                                        throw MBEDTLSException ( __func__, __LINE__, status,
                                            "mbedtls_rsa_export failed to obtain key parameters" );
                                    }
                                    status = mbedtls_rsa_export_crt ( rsa, & DP, & DQ, & QP );
                                    if ( status != 0 )
                                    {
                                        throw MBEDTLSException ( __func__, __LINE__, status,
                                            "mbedtls_rsa_export_crt failed to obtain key parameters" );
                                    }

                                    // write the key parameters into JSON
                                    writeKeyParameter ( props, "n", N );
                                    writeKeyParameter ( props, "e", E );
                                    writeKeyParameter ( props, "d", D );
                                    writeKeyParameter ( props, "p", P );
                                    writeKeyParameter ( props, "q", Q );
                                    writeKeyParameter ( props, "dp", DP );
                                    writeKeyParameter ( props, "dq", DQ );
                                    writeKeyParameter ( props, "qi", QP );

                                    // create the key
                                    // NB - MUST be last step within try block
                                    jwk = new JWK ( props_ref );
                                }
                            }
                            else if ( 0 )
                            {
                            }
                            else
                            {
                                // exception case
                                throw JWKException ( __func__, __LINE__, "INTERNAL ERROR - unknown mbedtls key type" );
                            }
                        }
                        catch ( ... )
                        {
                            props . invalidate ();
                            throw;
                        }
                    }
                    catch ( ... )
                    {
                        mbedtls_mpi_free ( & N );
                        mbedtls_mpi_free ( & P );
                        mbedtls_mpi_free ( & Q );
                        mbedtls_mpi_free ( & D );
                        mbedtls_mpi_free ( & E );
                        mbedtls_mpi_free ( & DP );
                        mbedtls_mpi_free ( & DQ );
                        mbedtls_mpi_free ( & QP );
                        throw;
                    }
                    
                    mbedtls_mpi_free ( & N );
                    mbedtls_mpi_free ( & P );
                    mbedtls_mpi_free ( & Q );
                    mbedtls_mpi_free ( & D );
                    mbedtls_mpi_free ( & E );
                    mbedtls_mpi_free ( & DP );
                    mbedtls_mpi_free ( & DQ );
                    mbedtls_mpi_free ( & QP );
                    
                }
                catch ( ... )
                {
                    mbedtls_pk_free ( & pk );
                    throw;
                }
                
                mbedtls_pk_free ( & pk );

                return jwk;
            }
        }
    }

}
