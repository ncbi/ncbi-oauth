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

#include <mbedtls/rsa.h>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    /*
     +-------------------+---------------------------------+
     | "alg" Param Value | Digital Signature Algorithm     |
     +-------------------+---------------------------------+
     | RS256             | RSASSA-PKCS1-v1_5 using SHA-256 |
     | RS384             | RSASSA-PKCS1-v1_5 using SHA-384 |
     | RS512             | RSASSA-PKCS1-v1_5 using SHA-512 |
     +-------------------+---------------------------------+
     The RSASSA-PKCS1-v1_5 SHA-256 digital signature for a JWS is
     validated as follows: submit the JWS Signing Input, the JWS
     Signature, and the public key corresponding to the private key used
     by the signer to the RSASSA-PKCS1-v1_5-VERIFY algorithm using SHA-256
     as the hash function.
     */
    
    struct RSA_Signer : JWASigner
    {
        virtual std :: string sign ( const void * data, size_t bytes ) const
        {
#if 0
            // encode as base64url
            return encodeBase64URL ( digest, dsize );
#else
            return "";
#endif
        }
        
        virtual JWASigner * clone () const
        {
            return new RSA_Signer ( nam, alg, key, md_type );
        }
        
        RSA_Signer ( const std :: string & name, const std :: string & alg,
                     const std :: string & key, mbedtls_md_type_t type )
            : JWASigner ( name, alg, key )
            , ctx ( cctx )
            , md_type ( type )
        {
        }
        
        ~ RSA_Signer ()
        {
            mbedtls_md_free ( & ctx );
        }
        
        size_t dsize;
        mbedtls_md_context_t cctx, & ctx;
        mbedtls_md_type_t md_type;
    };
    
    struct RSA_Verifier : JWAVerifier
    {
        virtual void verify ( const void * data, size_t bytes, const std :: string & sig_base64 ) const
        {

        }
        
        virtual JWAVerifier * clone () const
        {
            return new RSA_Verifier ( nam, alg, key, md_type );
        }
        
        RSA_Verifier ( const std :: string & name, const std :: string & alg,
                       const std :: string & key, mbedtls_md_type_t type )
            : JWAVerifier ( name, alg, key )
            , ctx ( cctx )
            , md_type ( type )
        {
        }
        
        ~ RSA_Verifier ()
        {
            mbedtls_md_free ( & ctx );
        }
        
        size_t dsize;
        mbedtls_md_context_t cctx, & ctx;
        mbedtls_md_type_t md_type;
    };
    
    struct RSA_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const std :: string & name,
            const std :: string & alg, const std :: string & key ) const
        {
            return new RSA_Signer ( name, alg, key, md_type );
        }
        
        RSA_SignerFact ( const std :: string & alg, mbedtls_md_type_t type )
            : md_type ( type )
        {
            gJWAFactory . registerSignerFact ( alg, this );
        }
        
        mbedtls_md_type_t md_type;
    };
    
    struct RSA_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const std :: string & name,
            const std :: string & alg, const std :: string & key ) const
        {
            return new RSA_Verifier ( name, alg, key, md_type );
        }
        
        RSA_VerifierFact ( const std :: string & alg, mbedtls_md_type_t type )
            : md_type ( type )
        {
            gJWAFactory . registerVerifierFact ( alg, this );
        }
        
        mbedtls_md_type_t md_type;
    };
    
    static struct RSA_Registry
    {
        RSA_Registry ( const std :: string & alg, mbedtls_md_type_t md_type )
            : signer_fact ( alg, md_type )
            , verifier_fact ( alg, md_type )
        {
        }
        
        RSA_SignerFact signer_fact;
        RSA_VerifierFact verifier_fact;
        
    } rs256 ( "RS256", MBEDTLS_MD_SHA256 ),
      rs384 ( "RS384", MBEDTLS_MD_SHA384 ),
      rs512 ( "RS512", MBEDTLS_MD_SHA512 );
    
    void includeJWA_rsa ( bool always_false )
    {
        if ( always_false )
        {
            std :: string empty;
            rs256 . signer_fact . make ( empty, empty, empty );
            rs384 . signer_fact . make ( empty, empty, empty );
            rs512 . signer_fact . make ( empty, empty, empty );
        }
    }
}
