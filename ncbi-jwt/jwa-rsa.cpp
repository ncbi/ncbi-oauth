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
            // checks if a context contains an RSA private key
            // and perform basic consistency checks.
            if ( mbedtls_rsa_check_privkey ( & ctx ) != 0  )
                throw JWTException ( __func__, __LINE__, "failed to verify RSA context and private key" );
            
            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );
            
            // Compute hash
            unsigned char hash [ 32 ];
            if ( mbedtls_md ( info, ( const unsigned char * ) data, bytes, hash ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to compute hash" );
            
            /*
             * \param ctx      The RSA context.
             * \param f_rng    The RNG function. Needed for PKCS#1 v2.1 encoding and for
             *                 #MBEDTLS_RSA_PRIVATE.
             * \param p_rng    The RNG context.
             * \param mode     #MBEDTLS_RSA_PUBLIC or #MBEDTLS_RSA_PRIVATE.
             * \param md_alg   The message-digest algorithm used to hash the original data.
             *                 Use #MBEDTLS_MD_NONE for signing raw data.
             * \param hashlen  The length of the message digest. Only used if \p md_alg is #MBEDTLS_MD_NONE.
             * \param hash     The buffer holding the message digest.
             * \param sig      The buffer to hold the ciphertext.
             */
            unsigned char digest [ 512 / 8 ];
            if ( mbedtls_rsa_pkcs1_sign ( & ctx, NULL, NULL, MBEDTLS_RSA_PRIVATE, md_type, sizeof hash, hash, digest ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to extract digest" );
            
            // encode as base64url
            return encodeBase64URL ( digest, dsize );
        }
        
        virtual JWASigner * clone () const
        {
            return new RSA_Signer ( alg, nam, key, md_type );
        }
        
        RSA_Signer ( const std :: string & alg, const std :: string & name,
                     const std :: string & key, mbedtls_md_type_t type )
        : JWASigner ( alg, name, key )
        , ctx ( cctx )
        , md_type ( type )
        {
            /*
            mbedtls_mpi N;              public modulus.
            mbedtls_mpi E;              public exponent.
            mbedtls_mpi D;              private exponent.
            mbedtls_mpi P;              first prime factor.
            mbedtls_mpi Q;              second prime factor.
            mbedtls_mpi DP;             <code>D % (P - 1)</code>.
            mbedtls_mpi DQ;             <code>D % (Q - 1)</code>.
            mbedtls_mpi QP;             <code>1 / (Q % P)</code>.
            mbedtls_mpi RN;             cached <code>R^2 mod N</code>.
            mbedtls_mpi RP;             cached <code>R^2 mod P</code>.
            mbedtls_mpi RQ;             cached <code>R^2 mod Q</code>.
            mbedtls_mpi Vi;             cached blinding value.
            mbedtls_mpi Vf;             cached un-blinding value.
            */
            
            // simple context initialization
            mbedtls_rsa_init ( & ctx, MBEDTLS_RSA_PKCS_V15, md_type );
            
            mbedtls_mpi_init ( & N ); mbedtls_mpi_init ( & P ); mbedtls_mpi_init ( & Q );
            mbedtls_mpi_init ( & D ); mbedtls_mpi_init ( & E ); mbedtls_mpi_init ( & DP );
            mbedtls_mpi_init ( & DQ ); mbedtls_mpi_init ( & QP );
            
            if ( ( mbedtls_mpi_read_string ( & N , 16, key . data () ) ) != 0  ||
                 ( mbedtls_mpi_read_string ( & E , 16, key . data () ) ) != 0  ||
                 ( mbedtls_mpi_read_string ( & D , 16, key . data () ) ) != 0  ||
                 ( mbedtls_mpi_read_string ( & P , 16, key . data () ) ) != 0  ||
                 ( mbedtls_mpi_read_string ( & Q , 16, key . data () ) ) != 0  ||
                 ( mbedtls_mpi_read_string ( & DP , 16, key . data () ) ) != 0 ||
                 ( mbedtls_mpi_read_string ( & DQ , 16, key . data () ) ) != 0 ||
                 ( mbedtls_mpi_read_string ( & QP , 16, key . data () ) ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to read context data from key" );
            
            if ( mbedtls_rsa_import ( & ctx, & N, & P, & Q, & D, & E ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to import data to RSA context" );
            
            if ( mbedtls_rsa_complete ( & ctx ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to initialize RSA context" );
        }
        
        ~ RSA_Signer ()
        {
            mbedtls_rsa_free ( & ctx );
            mbedtls_mpi_free( & N ); mbedtls_mpi_free( & P ); mbedtls_mpi_free( & Q );
            mbedtls_mpi_free( & D ); mbedtls_mpi_free( & E ); mbedtls_mpi_free( & DP );
            mbedtls_mpi_free( & DQ ); mbedtls_mpi_free( & QP );
        }
        
        //size_t dsize;
        mbedtls_mpi N, P, Q, D, E, DP, DQ, QP;
        mbedtls_rsa_context cctx, & ctx;
        mbedtls_md_type_t md_type;
    };
    
    struct RSA_Verifier : JWAVerifier
    {
        virtual void verify ( const void * data, size_t bytes, const std :: string & sig_base64 ) const
        {
            // TBD the following quoteded code
            /*
             * Extract the RSA signature from the text file
             
             mbedtls_snprintf( filename, sizeof(filename), "%s.sig", argv[1] );
             
             if( ( f = fopen( filename, "rb" ) ) == NULL )
             {
             mbedtls_printf( "\n  ! Could not open %s\n\n", filename );
             goto exit;
             }
             
             i = 0;
             while( fscanf( f, "%02X", &c ) > 0 &&
             i < (int) sizeof( buf ) )
             buf[i++] = (unsigned char) c;
             
             fclose( f );
             
             if( i != rsa.len )
             {
             mbedtls_printf( "\n  ! Invalid RSA signature format\n\n" );
             goto exit;
             }
             */
            
            // get info from the type
            const mbedtls_md_info_t * info = mbedtls_md_info_from_type ( md_type );
            size_t dsize = mbedtls_md_get_size ( info );
            
            // Compute hash
            unsigned char hash [ 32 ];
            if ( mbedtls_md ( info, ( const unsigned char * ) data, bytes, hash ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to compute hash" );
            
            unsigned char digest [ 512 / 8 ];
            if ( mbedtls_rsa_pkcs1_verify ( & ctx, NULL, NULL, MBEDTLS_RSA_PUBLIC, md_type, sizeof hash, hash, digest ) != 0 )
                throw JWTException ( __func__, __LINE__, "failed to verify signature" );
            
            Base64Payload signature = decodeBase64URL ( sig_base64 );
            
            if ( signature . size () != dsize )
                throw JWTException ( __func__, __LINE__, "signature mismatch" );
            
            // the digest must match
            if ( memcmp ( digest, signature . data (), dsize ) != 0 )
                throw JWTException ( __func__, __LINE__, "signature mismatch" );
        }
        
        virtual JWAVerifier * clone () const
        {
            return new RSA_Verifier ( alg, nam, key, md_type );
        }
        
        RSA_Verifier ( const std :: string & alg, const std :: string & name,
                       const std :: string & key, mbedtls_md_type_t type )
        : JWAVerifier ( alg, name, key )
        , ctx ( cctx )
        , md_type ( type )
        {
            mbedtls_rsa_init ( & ctx, MBEDTLS_RSA_PKCS_V15, md_type );
            
            if ( ( mbedtls_mpi_read_string ( & ctx . N , 16, key . data () ) ) != 0  ||
                ( mbedtls_mpi_read_string ( & ctx . E , 16, key . data () ) ) != 0  )
                throw JWTException ( __func__, __LINE__, "failed to read context data from key" );
            
            ctx . len = ( mbedtls_mpi_bitlen ( & ctx . N ) + 7 ) >> 3;
        }
        
        ~ RSA_Verifier ()
        {
            mbedtls_rsa_free ( & ctx );
        }
        
        size_t dsize;
        mbedtls_rsa_context cctx, & ctx;
        mbedtls_md_type_t md_type;
    };
    
    struct RSA_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const std :: string & alg,
                                  const std :: string & name, const std :: string & key ) const
        {
            return new RSA_Signer ( alg, name, key, md_type );
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
        virtual JWAVerifier * make ( const std :: string & alg,
                                    const std :: string & name, const std :: string & key ) const
        {
            return new RSA_Verifier ( alg, name, key, md_type );
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
