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

#include <ncbi/jwa.hpp>
#include <ncbi/jwk.hpp>
#include "jwa-registry.hpp"
#include "base64-priv.hpp"

#include <mbedtls/md.h>
#include <mbedtls/error.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    static
    JWAException MBEDTLSException ( const char * func, unsigned int line, int err, const char * msg )
    {
        char buffer [ 256 ];
        mbedtls_strerror ( err, buffer, sizeof buffer );
        return JWAException ( func, line, "%s: %s", msg, buffer );
    }

    struct HMAC_Signer : JWASigner
    {
        virtual std :: string sign ( const JWK & key,
            const void * data, size_t bytes ) const override
        {
            if ( ! key . isSymmetric () )
                throw JWAException ( __func__, __LINE__, "bad key type" );

            // simple context initialization
            mbedtls_md_context_t ctx;
            mbedtls_md_init ( & ctx );

            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            // allocates internal structures and binds to "info"
            int status = mbedtls_md_setup ( & ctx, info, 1 );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to setup HMAC context" );

            // bind the key to the context
            std :: string k = getKeyProp ( key, "k" );
            status = mbedtls_md_hmac_starts ( & ctx, ( const unsigned char * ) k . data (), k . size () );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to bind key to HMAC context" );

            // hash the data
            mbedtls_md_hmac_update ( & ctx, ( const unsigned char * ) data, bytes );

            // extract the digest - maximum size is 512 bits
            unsigned char digest [ 512 / 8 ];
            assert ( sizeof digest >= dsize );
            mbedtls_md_hmac_finish ( & ctx, digest );

            mbedtls_md_free ( & ctx );

            // encode as base64url
            return encodeBase64URL ( digest, dsize );
        }

        HMAC_Signer ( mbedtls_md_type_t type )
            : md_type ( type )
        {
        }

        mbedtls_md_type_t md_type;
    };

    struct HMAC_Verifier : JWAVerifier
    {
        virtual bool verify ( const JWK & key, const void * data, size_t bytes,
            const JWPayload & signature ) const override
        {
            if ( ! key . isSymmetric () )
                throw JWAException ( __func__, __LINE__, "bad key type" );

            // simple context initialization
            mbedtls_md_context_t ctx;
            mbedtls_md_init ( & ctx );

            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );

            // selects the digest algorithm and allocates internal structures
            int status = mbedtls_md_setup ( & ctx, info, 1 );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to setup HMAC context" );

            // bind the key to the context
            std :: string k = getKeyProp ( key, "k" );
            status = mbedtls_md_hmac_starts ( & ctx, ( const unsigned char * ) k . data (), k . size () );
            if ( status != 0 )
                throw MBEDTLSException ( __func__, __LINE__, status, "failed to bind key to HMAC context" );

            // hash the data
            mbedtls_md_hmac_update ( & ctx, ( const unsigned char * ) data, bytes );

            // extract the digest - maximum size is 512 bits
            unsigned char digest [ 512 / 8 ];
            assert ( sizeof digest >= dsize );
            mbedtls_md_hmac_finish ( & ctx, digest );

            mbedtls_md_free ( & ctx );

            // test: the lengths must match
            if ( signature . size () != dsize )
                return false;

            // the digest must match
            if ( memcmp ( digest, signature . data (), dsize ) != 0 )
                return false;

            // signature verified
            return true;
        }

        HMAC_Verifier ( mbedtls_md_type_t type )
            : md_type ( type )
        {
        }

        mbedtls_md_type_t md_type;
    };

    static struct HMAC_Registry
    {
        HMAC_Registry ()
        {
            std :: string alg = "HS256";
            gJWARegistry . registerSigner ( alg, new HMAC_Signer ( MBEDTLS_MD_SHA256 ) );
            gJWARegistry . registerVerifier ( alg, new HMAC_Verifier ( MBEDTLS_MD_SHA256 ) );

            alg = "HS384";
            gJWARegistry . registerSigner ( alg, new HMAC_Signer ( MBEDTLS_MD_SHA384 ) );
            gJWARegistry . registerVerifier ( alg, new HMAC_Verifier ( MBEDTLS_MD_SHA384 ) );

            alg = "HS512";
            gJWARegistry . registerSigner ( alg, new HMAC_Signer ( MBEDTLS_MD_SHA512 ) );
            gJWARegistry . registerVerifier ( alg, new HMAC_Verifier ( MBEDTLS_MD_SHA512 ) );
        }

        void avoidDeadStrip ()
        {
            gJWARegistry . doNothing ();
        }
        
    } hmac_registry;

    void includeJWA_hmac ( bool always_false )
    {
        if ( always_false )
            hmac_registry . avoidDeadStrip ();
    }
}
