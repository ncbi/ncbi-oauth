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

#include <ncbi/jws.hpp>
#include <ncbi/jwa.hpp>
#include <ncbi/jwp.hpp>
#include "base64-priv.hpp"

#include <sys/time.h>
#include <ctype.h>

namespace ncbi
{

    JWS JWSMgr :: signCompact ( const JWK & key,
        const void * payload, size_t bytes )
    {
        // JWS ( and JWE ) both utilize JOSE headers
        JSONObjectRef jose = JSON :: makeObject ();
        return signCompact ( key, * jose, payload, bytes );
    }

    JWS JWSMgr :: signCompact ( const JWK & key, JSONObject & jose,
        const void * payload, size_t bytes )
    {
        // test the payload
        if ( payload == nullptr || bytes == 0 )
            throw JWSException ( __func__, __LINE__, "invalid payload" );
        
        // test signing key
        if ( ! key . forSigning () )
            throw JWSException ( __func__, __LINE__, "INTERNAL ERROR - key not usable for signing" );

        std :: string alg = key . getAlg ();
        JWASignerRef signer = JWAMgr :: getSigner ( alg );

        // set the "alg" property in header
        jose . setValue ( "alg", JSON :: makeString ( alg ) );

        // set the "kid" property
        jose . setValue ( "kid", JSON :: makeString ( key . getID () ) );

        // convert the header to text
        std :: string hdr_json = jose . toJSON ();

        // encode the header with base64url
        JWS jws = encodeBase64URL ( hdr_json . data (), hdr_json . size () );

        // encode the payload with base64url
        std :: string pay_base64 = encodeBase64URL ( payload, bytes );

        // concatenate the two strings
        jws += '.';
        jws += pay_base64;

        // now we should be able to generate a signature
        std :: string sig_base64 = signer -> sign ( key, jws . data (), jws . size () );

        // concatenate the third string
        jws += '.';
        jws += sig_base64;

        return jws;
    }

    JWKRef JWSMgr :: extract ( JWPayload & payload, const JWKSet & val_keys, const JWS & jws )
    {
        // start by returning NO payload
        payload . erase ();

        // JWS should have some data
        if ( jws . empty () )
            throw JWSException ( __func__, __LINE__, "empty JWS" );

        // detect compact serialization
        int ch = jws [ 0 ];
        if ( ch < 0 )
            throw JWSException ( __func__, __LINE__, "badly formed JWS" );
        if ( isalnum ( ch ) || ch == '-' || ch == '_' )
            return extractCompact ( payload, val_keys, jws );

        // JWS JSON serialization is not currently supported
        throw JWSException ( __func__, __LINE__, "JWS JSON Serialization is not currently supported" );
    }

    JWKRef JWSMgr :: extractCompact ( JWPayload & payload, const JWKSet & val_keys, const JWS & jws )
    {
        // 1. "Parse the JWS representation to extract the serialized values for
        //     the components of the JWS.  When using the JWS Compact Serialization,
        //     these components are the base64url-encoded representations of the JWS
        //     Protected Header, the JWS Payload, and the JWS Signature.",
        //                                 ...
        //    "When using the JWS Compact Serialization, the JWS Protected Header,
        //     the JWS Payload, and the JWS Signature are represented as base64url-encoded
        //     values in that order, with each value being separated from the next by a
        //     single period ('.') character, resulting in exactly two delimiting period
        //     characters being used."
        // RFC7515 section 5.2 step 1 ( for compact serialization ).

        size_t pos = 0;
        size_t p = jws . find_first_of ( '.' );
        if ( p == std :: string :: npos )
            throw JWSException ( __func__, __LINE__, "Invalid JWS - expected: 3 sections" );

        // here's the protected-header ( the only kind in compact serialization )
        std :: string phdr_base64 = jws . substr ( 0, p );
        pos = p + 1;

        // find the next delimiter
        p = jws . find_first_of ( '.', pos );
        if ( p == std :: string :: npos )
            throw JWSException ( __func__, __LINE__, "Invalid JWS - expected: 3 sections" );

        // get the base64url-encoded payload
        std :: string pay_base64 = jws . substr ( pos, p );
        pos = p + 1;

        // take the whole header . payload as contents
        std :: string contents = jws . substr ( 0, p );

        // look for more sections
        p = jws . find_first_of ( '.', pos );
        if ( p != std :: string :: npos )
            throw JWSException ( __func__, __LINE__, "Invalid JWS - expected: 3 sections" );

        // strip off the signature
        std :: string signature = jws . substr ( pos );

        // 2. "Base64url-decode the encoded representation of the JWS Protected Header,
        //     following the restriction that no line breaks, whitespace, or other additional
        //     characters have been used."
        // RFC7515 section 5.2 step 2
        std :: string phdr_json = decodeBase64URLString ( phdr_base64, false );

        // 3. "Verify that the resulting octet sequence is a UTF-8-encoded representation
        //     of a completely valid JSON object conforming to RFC 7159 [RFC7159];
        //     let the JWS Protected Header be this JSON object."
        // RFC7515 section 5.2 step 3

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // ::::::     THIS IS EXACTLY THE PART THAT IS DANGEROUS     ::::::
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        //
        // the JWS specification has a serious security flaw in that it requests
        // ( encourages, indeed requires ) an implementation to take a piece of
        // data obtained from as yet unknown sources and throw it at a JSON parser,
        // since this is in practice the only way to verify conformance with the
        // step described above. Most libraries will use a commercial-grade JSON
        // parser that has been shown to correctly parse well-formed JSON, but is
        // highly unlikely to be armored against a maliciously-crafted header that
        // could break the parser and take over the executing process or host.

        JSON :: Limits lim;
        lim . recursion_depth = 20;
        JSONObjectRef jose = JSON :: parseObject ( lim, phdr_json );

        // 4. "If using the JWS Compact Serialization, let the JOSE Header be
        //     the JWS Protected Header."
        // RFC7515 section 5.2 step 4 ( for compact serialization ).
        //
        // done in step 3.

        // 5. "Verify that the implementation understands and can process all
        //     fields that it is required to support, whether required by this
        //     specification, by the algorithm being used, or by the "crit"
        //     Header Parameter value, and that the values of those parameters
        //     are also understood and supported."
        // RFC7515 section 5.2 step 5.
        verifyHeader ( * jose );

        // prescribed step 6, decoding the payload, is performed after verification

        // 6. prescribed steps 7..10 continue in verifyContents().
        JWKRef key = verifyContents ( val_keys, * jose, contents, signature );

        // 7. "Base64url-decode the encoded representation of the JWS Payload,
        //     following the restriction that no line breaks, whitespace, or
        //     other additional characters have been used."
        // RFC7515 section 5.2 step 6.
        payload = decodeBase64URL ( pay_base64, false );

        return key;
    }

    JWKRef JWSMgr :: verifyContents ( const JWKSet & val_keys, JSONObject & jose,
        const std :: string & content, const std :: string & signature )
    {
        // 1. "Base64url-decode the encoded representation of the JWS Signature,
        //     following the restriction that no line breaks, whitespace, or
        //     other additional characters have been used."
        // RFC7515 section 5.2 step 7.
        JWPayload binary_sig = decodeBase64URL ( signature, false );

        // 2. "Validate the JWS Signature against the JWS Signing Input
        //     ASCII(BASE64URL(UTF8(JWS Protected Header)) || '.' ||
        //     BASE64URL(JWS Payload)) in the manner defined for the algorithm
        //     being used, which MUST be accurately represented by the value of
        //     the "alg" (algorithm) Header Parameter, which MUST be present.
        //     See Section 10.6 for security considerations on algorithm
        //     validation."
        // RFC7515 section 5.2 step 8.

        // extract "alg" property from header
        // validates that the property is present
        std :: string alg = jose . getValue ( "alg" ) . toString ();

        // verifier algorithm - will throw an exception if
        // we weren't configured to accept the algorithm for signatures.
        // this will catch the "none" case, for example.
        JWAVerifierRef verifier = JWAMgr :: getVerifier ( alg );

        // dereference the content
        const void * data = content . data ();
        size_t bytes = content . size ();

        // will hold the key ( if any ) that verified signature
        JWKRef key;

        // will hold the id of the key ( if any ) that verified signature
        std :: string kid, jose_kid;

        // use "kid" as a hint from header
        bool kid_not_in_keyset = false;
        if ( jose . exists ( "kid" ) )
        {
            // header claims to have been signed with a given id
            kid = jose . getValue ( "kid" ) . toString ();

            // see if our accept-set contains a verification key
            if ( val_keys . contains ( kid ) )
            {
                // key ids match - get the key
                key = val_keys . getKey ( kid );

                // must be a signature verification key
                if ( ! key -> forVerifying () )
                    throw JWSException ( __func__, __LINE__, "algorithm mismatch" );

                // the key ALGORITHM has to also match
                if ( alg . compare ( key -> getAlg () ) != 0 )
                    throw JWSException ( __func__, __LINE__, "algorithm mismatch" );

                // the SIGNATURE has to match
                if ( ! verifier -> verify ( * key, data, bytes, binary_sig ) )
                    throw JWSException ( __func__, __LINE__, "signature mismatch" );

                // checks out okay
                return key;
            }

            else
            {
                // depending upon policy, we may wish to throw an exception if
                // the stated kid in the JWS is not in the key-set, rather than
                // searching for a matching key.
                kid_not_in_keyset = true;
                jose_kid = kid;
            }
        }

        // get the key ids for key-set contents
        std :: vector < std :: string > key_ids = val_keys . getKeyIDs ();

        // loop through the keys
        size_t i, count = key_ids . size ();
        for ( i = 0; i < count; ++ i )
        {
            // a random key-id
            kid = key_ids [ i ];

            // get the corresponding key
            key = val_keys . getKey ( kid );

            // if this is a verification key...
            if ( key -> forVerifying () )
            {
                // if the algorithms match...
                if ( alg . compare ( key -> getAlg () ) == 0 )
                {
                    // and the signature matches...
                    if ( verifier -> verify ( * key, data, bytes, binary_sig ) )
                        
                        // done!
                        return key;
                }
            }
        }

        // 3. "If the JWS JSON Serialization is being used, repeat this process
        //     (steps 4-8) for each digital signature or MAC value contained in
        //     the representation."
        // RFC7515 section 5.2 step 9.
        //
        // not applicable here - this is compact serialization

        // 4. "If none of the validations in step 9 succeeded, then the JWS MUST
        //      be considered invalid. In the JWS Compact Serialization case,
        //      the result can simply indicate whether or not the JWS was successfully
        //      validated."
        // RFC7515 section 5.2 step 10.

        // detect case when a "kid" was mentioned but was not in the JWKSet
        if ( kid_not_in_keyset )
        {
            throw JWSException ( __func__, __LINE__, "key-id provided but signature "
                "could not be verified - no match for key-id '%s'", jose_kid . c_str () );
        }

        // signature could not be verified because no matching key was found
        throw JWSException ( __func__, __LINE__, "signature could not be verified - no matching key" );
    }

    void JWSMgr :: verifyHeader ( const JSONObject & jose )
    {
    }

}
