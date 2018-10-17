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
#include <ncbi/jwt.hpp>
#include "base64-priv.hpp"

#include <mbedtls/md.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    struct HMAC_Signer : JWASigner
    {
        virtual std :: string sign ( const void * data, size_t bytes ) const
        {
            // start up with key
            mbedtls_md_hmac_starts ( & ctx, ( const unsigned char * ) key . data (), key . size () );

            // hash the data
            mbedtls_md_hmac_update ( & ctx, ( const unsigned char * ) data, bytes );

            // extract the digest
            unsigned char digest [ 32 ];
            mbedtls_md_hmac_finish ( & ctx, digest );

            // encode as base64url
            return encodeBase64URL ( digest, sizeof digest );
        }
        
        virtual JWASigner * clone () const
        {
            return new HMAC_Signer ( alg, nam, key, ctx );
        }

        HMAC_Signer ( const std :: string & alg, const std :: string & name,
                const std :: string & key, mbedtls_md_context_t & _ctx )
            : JWASigner ( alg, name, key )
            , ctx ( _ctx )
        {
        }

        mbedtls_md_context_t & ctx;
    };

    struct HMAC_Verifier : JWAVerifier
    {
        virtual void verify ( const void * data, size_t bytes, const std :: string & signature ) const
        {
        }
        
        virtual JWAVerifier * clone () const
        {
            return new HMAC_Verifier ( alg, nam, key, ctx );
        }

        HMAC_Verifier ( const std :: string & alg, const std :: string & name,
                const std :: string & key, mbedtls_md_context_t & _ctx )
            : JWAVerifier ( alg, name, key )
            , ctx ( _ctx )
        {
        }

        mbedtls_md_context_t & ctx;
    };
    
    struct HMAC_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const std :: string & alg,
            const std :: string & name, const std :: string & key ) const
        {
            HMAC_SignerFact * self = const_cast < HMAC_SignerFact * > ( this );
            return new HMAC_Signer ( alg, name, key, self -> ctx );
        }

        HMAC_SignerFact ( const char * name, mbedtls_md_type_t md_type )
        {
            mbedtls_md_init ( & ctx );
            int status = mbedtls_md_setup ( & ctx, mbedtls_md_info_from_type ( md_type ), 1 );
            if ( status == 0 )
                gJWAFactory . registerSignerFact ( name, this );
        }

        ~ HMAC_SignerFact ()
        {
            mbedtls_md_free ( & ctx );
        }

        mbedtls_md_context_t ctx;
    };

    struct HMAC_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const std :: string & alg,
            const std :: string & name, const std :: string & key ) const
        {
            HMAC_VerifierFact * self = const_cast < HMAC_VerifierFact * > ( this );
            return new HMAC_Verifier ( alg, name, key, self -> ctx );
        }

        HMAC_VerifierFact ( const char * name, mbedtls_md_type_t md_type )
        {
            mbedtls_md_init ( & ctx );
            int status = mbedtls_md_setup ( & ctx, mbedtls_md_info_from_type ( md_type ), 1 );
            if ( status == 0 )
                gJWAFactory . registerVerifierFact ( name, this );
        }

        ~ HMAC_VerifierFact ()
        {
            mbedtls_md_free ( & ctx );
        }

        mbedtls_md_context_t ctx;
    };

    static HMAC_SignerFact hs256_signer_fact ( "HS256", MBEDTLS_MD_SHA256 );
    static HMAC_VerifierFact hs256_verifier_fact ( "HS256", MBEDTLS_MD_SHA256 );

    void includeJWA_hmac ( bool always_false )
    {
        if ( always_false )
        {
            std :: string empty;
            hs256_signer_fact . make ( empty, empty, empty );
            hs256_verifier_fact . make ( empty, empty, empty );
        }
    }
}
