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

#include <ncbi/jwt.hpp>
#include <ncbi/jws.hpp>
#include "base64-priv.hpp"

#include <sys/time.h>
#include <uuid/uuid.h>

namespace ncbi
{

    long long int JWTMgr :: dflt_skew;

    JWTClaimSetRef JWTMgr :: makeClaimSet ()
    {
        // the claims are not supposed to be related to the header
        // but there are edge cases to be dealt with...
        JSONObjectRef jose = JSON :: makeObject ();
        JSONObjectRef claims = JSON :: makeObject ();

        // pair the two together
        return JWTClaimSetRef ( new JWTClaimSet ( jose, claims ) );
    }

    JWT JWTMgr :: sign ( const JWK & key, const JWTClaimSet & cs )
    {

        // lock the object against modification
        // clone the header and claims
        // then release the lock
        JSONObjectRef jose;
        JSONObjectRef claims;
        long long int duration;
        long long int not_before;
        {
            JWTLocker locker ( cs . obj_lock );
            jose = cs . jose -> cloneObject ();
            claims = cs . claims -> cloneObject ();
            duration = cs . duration;
            not_before = cs . not_before;
        }

        // 1. "Create a JWT Claims Set containing the desired claims.  Note that
        //     whitespace is explicitly allowed in the representation and no
        //     canonicalization need be performed before encoding."
        // RFC7519 section 7.1 step 1.
        //
        // the claims have already been populated by the client, but here
        // we stamp in some more claims, especially reserved ones.

        // TBD - apply policy about timeout

        // apply time stamps and timeouts
        finalizeClaims ( * claims, duration, not_before );

        // slam in a "jti"
        if ( ! claims -> exists ( "jti" ) )
        {
            std :: string jti = makeID ();
            claims -> addValue ( "jti", JSON :: makeString ( jti ) );
        }

        // 2. "Let the Message be the octets of the UTF-8 representation of the
        //     JWT Claims Set."
        // RFC7519 section 7.1 step 2.
        std :: string payload = claims -> toJSON ();

        // 3. "Create a JOSE Header containing the desired set of Header
        //     Parameters.  The JWT MUST conform to either the [JWS] or [JWE]
        //     specification.  Note that whitespace is explicitly allowed in the
        //     representation and no canonicalization need be performed before
        //     encoding."
        // RFC7519 section 7.1 step 3.
        //
        // nothing at this time.

        // 4. "Depending upon whether the JWT is a JWS or JWE, there are two cases:
        //     o If the JWT is a JWS, create a JWS using the Message as the JWS
        //       Payload; all steps specified in [JWS] for creating a JWS MUST
        //       be followed.
        //     o Else, ... "
        // RFC7519 section 7.1 step 4.

        return JWSMgr :: signCompact ( key, * jose, payload . data (), payload . size () );

        // 5. "If a nested signing or encryption operation will be performed,
        //     let the Message be the JWS or JWE, and return to Step 3, using a
        //     "cty" (content type) value of "JWT" in the new JOSE Header
        //     created in that step."
        // RFC7519 section 7.1 step 5.
        //
        // N/A

        // 6. "Otherwise, let the resulting JWT be the JWS or JWE."
        // RFC7519 section 7.1 step 6.
        //
        // done.
    }

    JWT JWTMgr :: nestedSign ( const JWK & key, const JWT & jwt )
    {
        // 1. "If a nested signing or encryption operation will be performed,
        //     let the Message be the JWS or JWE, and return to Step 3, using a
        //     "cty" (content type) value of "JWT" in the new JOSE Header
        //     created in that step."
        // RFC7519 section 7.1 step 5.

        JSONObjectRef jose = JSON :: makeObject ();
        jose -> addValue ( "cty", JSON :: makeString ( "JWT" ) );

        return JWSMgr :: signCompact ( key, * jose, jwt . data (), jwt . size () );
    }

    JWTClaimSetRef JWTMgr :: decode ( const JWKSet & keys, const JWT & jwt )
    {
        // use current time and default clock-skew-adjustment
        return decode ( keys, jwt, now (), dflt_skew );
    }

    JWTClaimSetRef JWTMgr :: decode ( const JWKSet & val_keys, const JWT & _jwt,
        long long cur_time, long long skew_secs )
    {

        // prepare for restart on nested JWT
        JWT jwt ( _jwt );
        bool nested = false;

        // restarted when JWT is nested
        while ( 1 )
        {

            //  1. "Verify that the JWT contains at least one period ('.') character."
            // RFC7519 section 7.2 step 1.
            size_t pos = 0;
            size_t p = jwt . find_first_of ( '.' );
            if ( p == std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

            //  2. "Let the Encoded JOSE Header be the portion of the JWT before the
            //      first period ('.') character."
            // RFC7519 section 7.2 step 2.
            std :: string hdr_base64 = jwt . substr ( 0, p );
            pos = p + 1;

            //  3. "Base64url decode the Encoded JOSE Header following the restriction
            //      that no line breaks, whitespace, or other additional characters
            //      have been used."
            // RFC7519 section 7.2 step 3.
            std :: string hdr_json = decodeBase64URLString ( hdr_base64, false );

            //  4. "Verify that the resulting octet sequence is a UTF-8-encoded
            //      representation of a completely valid JSON object conforming to
            //      RFC 7159 [RFC7159]; let the JOSE Header be this JSON object."
            // RFC7519 section 7.2 step 4.

            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            // ::::::     THIS IS EXACTLY THE PART THAT IS DANGEROUS     ::::::
            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            //
            // the JWT specification has a serious security flaw in that it requests
            // ( encourages, indeed requires ) an implementation to take a piece of
            // data obtained from as yet unknown sources and throw it at a JSON parser,
            // since this is in practice the only way to verify conformance with the
            // step described above. Most libraries will use a commercial-grade JSON
            // parser that has been shown to correctly parse well-formed JSON, but is
            // highly unlikely to be armored against a maliciously-crafted header that
            // could break the parser and take over the executing process or host.

            JSON :: Limits lim;
            lim . recursion_depth = 20;
            JSONObjectRef jose = JSON :: parseObject ( lim, hdr_json );

            //  5. "Verify that the resulting JOSE Header includes only parameters
            //      and values whose syntax and semantics are both understood and
            //      supported or that are specified as being ignored when not
            //      understood."
            // RFC7519 section 7.2 step 5.
            //
            // this only makes sense if you know whether it was JWS or JWE, and
            // we don't know that yet. Furthermore, the way it is worded, there
            // is no modular way to have JWT look for only its things and then
            // let JWS or JWE look for their things, because the standards state
            // that the headers cannot contain anything they don't understand,
            // which implies that JWS and JWE have to know everything about JWT,
            // and JWT has to know everything about both JWS and JWE.
            //
            // so this step is suspended until we detect underlying encapsulation.

            //  6. "Determine whether the JWT is a JWS or a JWE using any of the
            //      methods described in Section 9 of [JWE]."
            // RFC7519 section 7.2 step 6.
            //
            //     "There are several ways of distinguishing whether an object is a
            //      JWS or JWE. All these methods will yield the same result for all
            //      legal input values; they may yield different results for malformed
            //      inputs.
            //      o  If the object is using the JWS Compact Serialization or the JWE
            //         Compact Serialization, the number of base64url-encoded segments
            //          separated by period ('.') characters differs for JWSs and JWEs.
            //         JWSs have three segments separated by two period ('.') characters.
            //         JWEs have five segments separated by four period ('.') characters.
            //      o  The JOSE Header for a JWS can also be distinguished from the JOSE
            //         Header for a JWE by determining whether an "enc" (encryption
            //         algorithm) member exists.  If the "enc" member exists, it is a
            //         JWE; otherwise, it is a JWS."
            // RFC7516 section 9
            //
            // we can do both, because even though a quick check for "enc" is easiest,
            // the string still needs to be broken into its constituent parts.

            // find the next delimiter
            p = jwt . find_first_of ( '.', pos );
            if ( p == std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

            JWKRef key;
            std :: string message;
            std :: string pay_base64;
            std :: string contents;
            std :: string signature;

            //  7. "Depending upon whether the JWT is a JWS or JWE, there are two cases:"
            // RFC7519 section 7.2 step 7.
            if ( jose -> exists ( "enc" ) )
            {
                // 7b. "If the JWT is a JWE, follow the steps specified in [JWE] for validating
                //      a JWE. Let the Message be the resulting plaintext."
                // RFC7519 section 7.2 step 7b.

                throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - JWE is not supported at this time." );
            }
            else
            {

                // 7a. "If the JWT is a JWS, follow the steps specified in [JWS] for validating
                //      a JWS.  Let the Message be the result of base64url decoding the JWS Payload."
                // RFC7519 section 7.2 step 7a.

                // RFC7515 section 5.2 step 1 ( for compact serialization ).

                // get the base64url-encoded payload
                pay_base64 = jwt . substr ( pos, p );
                pos = p + 1;

                // take the whole header . payload as contents
                contents = jwt . substr ( 0, p );

                // strip off the signature
                signature = jwt . substr ( pos );

                // look for more sections
                p = jwt . find_first_of ( '.', pos );
                if ( p != std :: string :: npos )
                    throw JWTException ( __func__, __LINE__, "Invalid JWT/JWS - expected: 3 sections" );

                // RFC7515 section 5.2 steps 2..4 - completed above
                
                // RFC7515 section 5.2 step 5.
                verifyJWSHeader ( * jose );

                // RFC7515 section 5.2 steps 7..10.
                key = JWSMgr :: verifyContents ( val_keys, * jose, contents, signature );

                // RFC7515 section 5.2 step 6
                message = decodeBase64URLString ( pay_base64, false );
            }

            //  8. "If the JOSE Header contains a "cty" (content type) value of "JWT",
            //      then the Message is a JWT that was the subject of nested signing
            //      or encryption operations.  In this case, return to Step 1, using
            //      the Message as the JWT."
            // RFC7519 section 7.2 step 8.
            //
            // notice that this sounds kinda cool, but it's not really going to work
            // the way it's designed, or at least the way one might think. For example,
            // the way to apply nesting during signing is not with a loop and an array
            // of signing keys, but with successive calls to sign, starting with claims
            // and then signing the resulting JWTs on each call. This allows separate
            // authorities to apply their signatures in a nested fashion, i.e.
            // "I didn't sign the inner JWT, but I applied my signature on it to
            // pass it down the line."
            //
            // if that's a fair assessment, it means that recipients are going to
            // want to decode the nested JWT and receive another JWT in return that
            // can be passed on to another stage. The only time you want the claims
            // is when it's no longer nested.
            //
            // so having this facility right here will remove but discard the outer
            // signatures as long as all of the keys are in the set.

            if ( jose -> exists ( "cty" ) )
            {
                if ( jose -> getValue ( "cty" ) . toString () . compare ( "JWT" ) == 0 )
                {
                    nested = true;
                    jwt = message;
                    continue;
                }
            }

            //  9. "Otherwise, base64url decode the Message following the
            //      restriction that no line breaks, whitespace, or other additional
            //      characters have been used."
            // RFC7519 section 7.2 step 9.
            //
            // Huh? I'm afraid this is misstated...
            // "Message," as it is being used, was defined as the decoded payload
            // up above ( "Let the Message be the result of base64url decoding the
            // JWS Payload." RFC7519 section 7.2 step 7a. Cool! )

            // this should not happen unless I'm really off...
            if ( ! message . empty () )
            {
                int ch = message [ 0 ];
                if ( isalnum ( ch ) || ch == '-' || ch == '_' )
                {
                    // geee, this is base64url encoded after all!
                    pay_base64 = message;
                    message = decodeBase64URLString ( pay_base64, false );
                }
            }

            // 10. "Verify that the resulting octet sequence is a UTF-8-encoded
            //      representation of a completely valid JSON object conforming to
            //      RFC 7159 [RFC7159]; let the JWT Claims Set be this JSON object."
            // RFC7519 section 7.2 step 10.

            lim . recursion_depth = 50;
            JSONObjectRef claims = JSON :: parseObject ( lim, message );
        
            // can still fail based upon expiration
            verifyPayload ( * jose, * claims, cur_time, skew_secs );

            // it is done.
            return JWTClaimSetRef ( new JWTClaimSet ( key, jose, claims ) );
        }
    }

    JWKRef JWTMgr :: nestedDecode ( const JWKSet & val_keys, JWT & jwt )
    {
        
        //  1. "Verify that the JWT contains at least one period ('.') character."
        // RFC7519 section 7.2 step 1.
        size_t pos = 0;
        size_t p = jwt . find_first_of ( '.' );
        if ( p == std :: string :: npos )
            throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

        //  2. "Let the Encoded JOSE Header be the portion of the JWT before the
        //      first period ('.') character."
        // RFC7519 section 7.2 step 2.
        std :: string hdr_base64 = jwt . substr ( 0, p );
        pos = p + 1;

        //  3. "Base64url decode the Encoded JOSE Header following the restriction
        //      that no line breaks, whitespace, or other additional characters
        //      have been used."
        // RFC7519 section 7.2 step 3.
        std :: string hdr_json = decodeBase64URLString ( hdr_base64, false );

        //  4. "Verify that the resulting octet sequence is a UTF-8-encoded
        //      representation of a completely valid JSON object conforming to
        //      RFC 7159 [RFC7159]; let the JOSE Header be this JSON object."
        // RFC7519 section 7.2 step 4.
        
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // ::::::     THIS IS EXACTLY THE PART THAT IS DANGEROUS     ::::::
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        //
        // the JWT specification has a serious security flaw in that it requests
        // ( encourages, indeed requires ) an implementation to take a piece of
        // data obtained from as yet unknown sources and throw it at a JSON parser,
        // since this is in practice the only way to verify conformance with the
        // step described above. Most libraries will use a commercial-grade JSON
        // parser that has been shown to correctly parse well-formed JSON, but is
        // highly unlikely to be armored against a maliciously-crafted header that
        // could break the parser and take over the executing process or host.
        
        JSON :: Limits lim;
        lim . recursion_depth = 20;
        JSONObjectRef jose = JSON :: parseObject ( lim, hdr_json );

        //  5. "If the JOSE Header contains a "cty" (content type) value of "JWT",
        //      then the Message is a JWT that was the subject of nested signing
        //      or encryption operations.  In this case, return to Step 1, using
        //      the Message as the JWT."
        // RFC7519 section 7.2 step 8.
        //

        if ( ! jose -> exists ( "cty" ) ||
             jose -> getValue ( "cty" ) . toString () . compare ( "JWT" ) != 0 )
        {
            throw JWTException ( __func__, __LINE__, "not a nested JWT" );
        }

        //  6. "Verify that the resulting JOSE Header includes only parameters
        //      and values whose syntax and semantics are both understood and
        //      supported or that are specified as being ignored when not
        //      understood."
        // RFC7519 section 7.2 step 5.
        //
        // this only makes sense if you know whether it was JWS or JWE, and
        // we don't know that yet. Furthermore, the way it is worded, there
        // is no modular way to have JWT look for only its things and then
        // let JWS or JWE look for their things, because the standards state
        // that the headers cannot contain anything they don't understand,
        // which implies that JWS and JWE have to know everything about JWT,
        // and JWT has to know everything about both JWS and JWE.
        //
        // so this step is suspended until we detect underlying encapsulation.
        
        //  7. "Determine whether the JWT is a JWS or a JWE using any of the
        //      methods described in Section 9 of [JWE]."
        // RFC7519 section 7.2 step 6.
        //
        //     "There are several ways of distinguishing whether an object is a
        //      JWS or JWE. All these methods will yield the same result for all
        //      legal input values; they may yield different results for malformed
        //      inputs.
        //      o  If the object is using the JWS Compact Serialization or the JWE
        //         Compact Serialization, the number of base64url-encoded segments
        //          separated by period ('.') characters differs for JWSs and JWEs.
        //         JWSs have three segments separated by two period ('.') characters.
        //         JWEs have five segments separated by four period ('.') characters.
        //      o  The JOSE Header for a JWS can also be distinguished from the JOSE
        //         Header for a JWE by determining whether an "enc" (encryption
        //         algorithm) member exists.  If the "enc" member exists, it is a
        //         JWE; otherwise, it is a JWS."
        // RFC7516 section 9
        //
        // we can do both, because even though a quick check for "enc" is easiest,
        // the string still needs to be broken into its constituent parts.

        // find the next delimiter
        p = jwt . find_first_of ( '.', pos );
        if ( p == std :: string :: npos )
            throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

        JWKRef key;
        std :: string message;
        std :: string pay_base64;
        std :: string contents;
        std :: string signature;

        //  8. "Depending upon whether the JWT is a JWS or JWE, there are two cases:"
        // RFC7519 section 7.2 step 7.
        if ( jose -> exists ( "enc" ) )
        {
            // 8b. "If the JWT is a JWE, follow the steps specified in [JWE] for validating
            //      a JWE. Let the Message be the resulting plaintext."
            // RFC7519 section 7.2 step 7b.

            throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - JWE is not supported at this time." );
        }
        else
        {

            // 8a. "If the JWT is a JWS, follow the steps specified in [JWS] for validating
            //      a JWS.  Let the Message be the result of base64url decoding the JWS Payload."
            // RFC7519 section 7.2 step 7a.
            
            // RFC7515 section 5.2 step 1 ( for compact serialization ).

            // get the base64url-encoded payload
            pay_base64 = jwt . substr ( pos, p );
            pos = p + 1;

            // take the whole header . payload as contents
            contents = jwt . substr ( 0, p );

            // strip off the signature
            signature = jwt . substr ( pos );

            // look for more sections
            p = jwt . find_first_of ( '.', pos );
            if ( p != std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT/JWS - expected: 3 sections" );

            // RFC7515 section 5.2 steps 2..4 - completed above
                
            // RFC7515 section 5.2 step 5.
            verifyJWSHeader ( * jose );

            // RFC7515 section 5.2 steps 7..10.
            key = JWSMgr :: verifyContents ( val_keys, * jose, contents, signature );
            
            // RFC7515 section 5.2 step 6
            message = decodeBase64URLString ( pay_base64, false );
        }

        //  9. "If the JOSE Header contains a "cty" (content type) value of "JWT",
        //      then the Message is a JWT that was the subject of nested signing
        //      or encryption operations.  In this case, return to Step 1, using
        //      the Message as the JWT."
        // RFC7519 section 7.2 step 8.

        jwt = message;
        return key;
    }

    UnverifiedJWTClaimSetRef JWTMgr :: inspect ( const JWT & _jwt )
    {

        // prepare for restart on nested JWT
        JWT jwt ( _jwt );
        bool nested = false;

        // restarted when JWT is nested
        while ( 1 )
        {

            //  1. "Verify that the JWT contains at least one period ('.') character."
            // RFC7519 section 7.2 step 1.
            size_t pos = 0;
            size_t p = jwt . find_first_of ( '.' );
            if ( p == std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

            //  2. "Let the Encoded JOSE Header be the portion of the JWT before the
            //      first period ('.') character."
            // RFC7519 section 7.2 step 2.
            std :: string hdr_base64 = jwt . substr ( 0, p );
            pos = p + 1;

            //  3. "Base64url decode the Encoded JOSE Header following the restriction
            //      that no line breaks, whitespace, or other additional characters
            //      have been used."
            // RFC7519 section 7.2 step 3.
            std :: string hdr_json = decodeBase64URLString ( hdr_base64, false );

            //  4. "Verify that the resulting octet sequence is a UTF-8-encoded
            //      representation of a completely valid JSON object conforming to
            //      RFC 7159 [RFC7159]; let the JOSE Header be this JSON object."
            // RFC7519 section 7.2 step 4.

            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            // ::::::     THIS IS EXACTLY THE PART THAT IS DANGEROUS     ::::::
            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            //
            // the JWT specification has a serious security flaw in that it requests
            // ( encourages, indeed requires ) an implementation to take a piece of
            // data obtained from as yet unknown sources and throw it at a JSON parser,
            // since this is in practice the only way to verify conformance with the
            // step described above. Most libraries will use a commercial-grade JSON
            // parser that has been shown to correctly parse well-formed JSON, but is
            // highly unlikely to be armored against a maliciously-crafted header that
            // could break the parser and take over the executing process or host.

            JSON :: Limits lim;
            lim . recursion_depth = 20;
            JSONObjectRef jose = JSON :: parseObject ( lim, hdr_json );

            //  5. "Verify that the resulting JOSE Header includes only parameters
            //      and values whose syntax and semantics are both understood and
            //      supported or that are specified as being ignored when not
            //      understood."
            // RFC7519 section 7.2 step 5.
            //
            // this only makes sense if you know whether it was JWS or JWE, and
            // we don't know that yet. Furthermore, the way it is worded, there
            // is no modular way to have JWT look for only its things and then
            // let JWS or JWE look for their things, because the standards state
            // that the headers cannot contain anything they don't understand,
            // which implies that JWS and JWE have to know everything about JWT,
            // and JWT has to know everything about both JWS and JWE.
            //
            // for inspection, this part is skipped entirely. (!)

            //  6. "Determine whether the JWT is a JWS or a JWE using any of the
            //      methods described in Section 9 of [JWE]."
            // RFC7519 section 7.2 step 6.
            //
            //     "There are several ways of distinguishing whether an object is a
            //      JWS or JWE. All these methods will yield the same result for all
            //      legal input values; they may yield different results for malformed
            //      inputs.
            //      o  If the object is using the JWS Compact Serialization or the JWE
            //         Compact Serialization, the number of base64url-encoded segments
            //          separated by period ('.') characters differs for JWSs and JWEs.
            //         JWSs have three segments separated by two period ('.') characters.
            //         JWEs have five segments separated by four period ('.') characters.
            //      o  The JOSE Header for a JWS can also be distinguished from the JOSE
            //         Header for a JWE by determining whether an "enc" (encryption
            //         algorithm) member exists.  If the "enc" member exists, it is a
            //         JWE; otherwise, it is a JWS."
            // RFC7516 section 9
            //
            // we can do both, because even though a quick check for "enc" is easiest,
            // the string still needs to be broken into its constituent parts.

            // find the next delimiter
            p = jwt . find_first_of ( '.', pos );
            if ( p == std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

            std :: string message;
            std :: string pay_base64;
            std :: string contents;
            std :: string signature;

            //  7. "Depending upon whether the JWT is a JWS or JWE, there are two cases:"
            // RFC7519 section 7.2 step 7.
            if ( jose -> exists ( "enc" ) )
            {
                // 7b. "If the JWT is a JWE, follow the steps specified in [JWE] for validating
                //      a JWE. Let the Message be the resulting plaintext."
                // RFC7519 section 7.2 step 7b.

                throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - JWE is not supported at this time." );
            }
            else
            {

                // 7a. "If the JWT is a JWS, follow the steps specified in [JWS] for validating
                //      a JWS.  Let the Message be the result of base64url decoding the JWS Payload."
                // RFC7519 section 7.2 step 7a.

                // RFC7515 section 5.2 step 1 ( for compact serialization ).

                // get the base64url-encoded payload
                pay_base64 = jwt . substr ( pos, p );
                pos = p + 1;

                // take the whole header . payload as contents
                contents = jwt . substr ( 0, p );

                // strip off the signature
                signature = jwt . substr ( pos );

                // look for more sections
                p = jwt . find_first_of ( '.', pos );
                if ( p != std :: string :: npos )
                    throw JWTException ( __func__, __LINE__, "Invalid JWT/JWS - expected: 3 sections" );

                // RFC7515 section 5.2 steps 2..4 - completed above
                
                // RFC7515 section 5.2 step 5.
                // SKIPPED FOR INSPECTION

                // RFC7515 section 5.2 steps 7..10.
                // SKIPPED FOR INSPECTION

                // RFC7515 section 5.2 step 6
                message = decodeBase64URLString ( pay_base64, false );
            }

            //  8. "If the JOSE Header contains a "cty" (content type) value of "JWT",
            //      then the Message is a JWT that was the subject of nested signing
            //      or encryption operations.  In this case, return to Step 1, using
            //      the Message as the JWT."
            // RFC7519 section 7.2 step 8.
            //
            // notice that this sounds kinda cool, but it's not really going to work
            // the way it's designed, or at least the way one might think. For example,
            // the way to apply nesting during signing is not with a loop and an array
            // of signing keys, but with successive calls to sign, starting with claims
            // and then signing the resulting JWTs on each call. This allows separate
            // authorities to apply their signatures in a nested fashion, i.e.
            // "I didn't sign the inner JWT, but I applied my signature on it to
            // pass it down the line."
            //
            // if that's a fair assessment, it means that recipients are going to
            // want to decode the nested JWT and receive another JWT in return that
            // can be passed on to another stage. The only time you want the claims
            // is when it's no longer nested.
            //
            // so having this facility right here will remove but discard the outer
            // signatures as long as all of the keys are in the set.

            if ( jose -> exists ( "cty" ) )
            {
                if ( jose -> getValue ( "cty" ) . toString () . compare ( "JWT" ) == 0 )
                {
                    nested = true;
                    jwt = message;
                    continue;
                }
            }

            //  9. "Otherwise, base64url decode the Message following the
            //      restriction that no line breaks, whitespace, or other additional
            //      characters have been used."
            // RFC7519 section 7.2 step 9.
            //
            // Huh? I'm afraid this is misstated...
            // "Message," as it is being used, was defined as the decoded payload
            // up above ( "Let the Message be the result of base64url decoding the
            // JWS Payload." RFC7519 section 7.2 step 7a. Cool! )

            // this should not happen unless I'm really off...
            if ( ! message . empty () )
            {
                int ch = message [ 0 ];
                if ( isalnum ( ch ) || ch == '-' || ch == '_' )
                {
                    // geee, this is base64url encoded after all!
                    pay_base64 = message;
                    message = decodeBase64URLString ( pay_base64, false );
                }
            }

            // 10. "Verify that the resulting octet sequence is a UTF-8-encoded
            //      representation of a completely valid JSON object conforming to
            //      RFC 7159 [RFC7159]; let the JWT Claims Set be this JSON object."
            // RFC7519 section 7.2 step 10.

            lim . recursion_depth = 50;
            JSONObjectRef claims = JSON :: parseObject ( lim, message );
        
            // can still fail based upon expiration
            // SKIPPED FOR INSPECTION

            // it is done.
            return UnverifiedJWTClaimSetRef ( new UnverifiedJWTClaimSet ( jose, claims ) );
        }
    }

    UnverifiedJWTClaimSetRef JWTMgr :: inspect ( const JWKSet & val_keys, const JWT & _jwt )
    {

        // prepare for restart on nested JWT
        JWT jwt ( _jwt );
        bool nested = false;

        // restarted when JWT is nested
        while ( 1 )
        {

            //  1. "Verify that the JWT contains at least one period ('.') character."
            // RFC7519 section 7.2 step 1.
            size_t pos = 0;
            size_t p = jwt . find_first_of ( '.' );
            if ( p == std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

            //  2. "Let the Encoded JOSE Header be the portion of the JWT before the
            //      first period ('.') character."
            // RFC7519 section 7.2 step 2.
            std :: string hdr_base64 = jwt . substr ( 0, p );
            pos = p + 1;

            //  3. "Base64url decode the Encoded JOSE Header following the restriction
            //      that no line breaks, whitespace, or other additional characters
            //      have been used."
            // RFC7519 section 7.2 step 3.
            std :: string hdr_json = decodeBase64URLString ( hdr_base64, false );

            //  4. "Verify that the resulting octet sequence is a UTF-8-encoded
            //      representation of a completely valid JSON object conforming to
            //      RFC 7159 [RFC7159]; let the JOSE Header be this JSON object."
            // RFC7519 section 7.2 step 4.

            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            // ::::::     THIS IS EXACTLY THE PART THAT IS DANGEROUS     ::::::
            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            //
            // the JWT specification has a serious security flaw in that it requests
            // ( encourages, indeed requires ) an implementation to take a piece of
            // data obtained from as yet unknown sources and throw it at a JSON parser,
            // since this is in practice the only way to verify conformance with the
            // step described above. Most libraries will use a commercial-grade JSON
            // parser that has been shown to correctly parse well-formed JSON, but is
            // highly unlikely to be armored against a maliciously-crafted header that
            // could break the parser and take over the executing process or host.

            JSON :: Limits lim;
            lim . recursion_depth = 20;
            JSONObjectRef jose = JSON :: parseObject ( lim, hdr_json );

            //  5. "Verify that the resulting JOSE Header includes only parameters
            //      and values whose syntax and semantics are both understood and
            //      supported or that are specified as being ignored when not
            //      understood."
            // RFC7519 section 7.2 step 5.
            //
            // this only makes sense if you know whether it was JWS or JWE, and
            // we don't know that yet. Furthermore, the way it is worded, there
            // is no modular way to have JWT look for only its things and then
            // let JWS or JWE look for their things, because the standards state
            // that the headers cannot contain anything they don't understand,
            // which implies that JWS and JWE have to know everything about JWT,
            // and JWT has to know everything about both JWS and JWE.
            //
            // for inspection, this part is skipped entirely. (!)

            //  6. "Determine whether the JWT is a JWS or a JWE using any of the
            //      methods described in Section 9 of [JWE]."
            // RFC7519 section 7.2 step 6.
            //
            //     "There are several ways of distinguishing whether an object is a
            //      JWS or JWE. All these methods will yield the same result for all
            //      legal input values; they may yield different results for malformed
            //      inputs.
            //      o  If the object is using the JWS Compact Serialization or the JWE
            //         Compact Serialization, the number of base64url-encoded segments
            //          separated by period ('.') characters differs for JWSs and JWEs.
            //         JWSs have three segments separated by two period ('.') characters.
            //         JWEs have five segments separated by four period ('.') characters.
            //      o  The JOSE Header for a JWS can also be distinguished from the JOSE
            //         Header for a JWE by determining whether an "enc" (encryption
            //         algorithm) member exists.  If the "enc" member exists, it is a
            //         JWE; otherwise, it is a JWS."
            // RFC7516 section 9
            //
            // we can do both, because even though a quick check for "enc" is easiest,
            // the string still needs to be broken into its constituent parts.

            // find the next delimiter
            p = jwt . find_first_of ( '.', pos );
            if ( p == std :: string :: npos )
                throw JWTException ( __func__, __LINE__, "Invalid JWT - expected: 3 or 5 sections" );

            JWKRef key;
            std :: string message;
            std :: string pay_base64;
            std :: string contents;
            std :: string signature;

            //  7. "Depending upon whether the JWT is a JWS or JWE, there are two cases:"
            // RFC7519 section 7.2 step 7.
            if ( jose -> exists ( "enc" ) )
            {
                // 7b. "If the JWT is a JWE, follow the steps specified in [JWE] for validating
                //      a JWE. Let the Message be the resulting plaintext."
                // RFC7519 section 7.2 step 7b.

                throw JWTException ( __func__, __LINE__, "UNIMPLEMENTED - JWE is not supported at this time." );
            }
            else
            {

                // 7a. "If the JWT is a JWS, follow the steps specified in [JWS] for validating
                //      a JWS.  Let the Message be the result of base64url decoding the JWS Payload."
                // RFC7519 section 7.2 step 7a.

                // RFC7515 section 5.2 step 1 ( for compact serialization ).

                // get the base64url-encoded payload
                pay_base64 = jwt . substr ( pos, p );
                pos = p + 1;

                // take the whole header . payload as contents
                contents = jwt . substr ( 0, p );

                // strip off the signature
                signature = jwt . substr ( pos );

                // look for more sections
                p = jwt . find_first_of ( '.', pos );
                if ( p != std :: string :: npos )
                    throw JWTException ( __func__, __LINE__, "Invalid JWT/JWS - expected: 3 sections" );

                // RFC7515 section 5.2 steps 2..4 - completed above
                
                // RFC7515 section 5.2 step 5.
                // SKIPPED FOR INSPECTION

                // RFC7515 section 5.2 steps 7..10.
                key = JWSMgr :: verifyContents ( val_keys, * jose, contents, signature );

                // RFC7515 section 5.2 step 6
                message = decodeBase64URLString ( pay_base64, false );
            }

            //  8. "If the JOSE Header contains a "cty" (content type) value of "JWT",
            //      then the Message is a JWT that was the subject of nested signing
            //      or encryption operations.  In this case, return to Step 1, using
            //      the Message as the JWT."
            // RFC7519 section 7.2 step 8.
            //
            // notice that this sounds kinda cool, but it's not really going to work
            // the way it's designed, or at least the way one might think. For example,
            // the way to apply nesting during signing is not with a loop and an array
            // of signing keys, but with successive calls to sign, starting with claims
            // and then signing the resulting JWTs on each call. This allows separate
            // authorities to apply their signatures in a nested fashion, i.e.
            // "I didn't sign the inner JWT, but I applied my signature on it to
            // pass it down the line."
            //
            // if that's a fair assessment, it means that recipients are going to
            // want to decode the nested JWT and receive another JWT in return that
            // can be passed on to another stage. The only time you want the claims
            // is when it's no longer nested.
            //
            // so having this facility right here will remove but discard the outer
            // signatures as long as all of the keys are in the set.

            if ( jose -> exists ( "cty" ) )
            {
                if ( jose -> getValue ( "cty" ) . toString () . compare ( "JWT" ) == 0 )
                {
                    nested = true;
                    jwt = message;
                    continue;
                }
            }

            //  9. "Otherwise, base64url decode the Message following the
            //      restriction that no line breaks, whitespace, or other additional
            //      characters have been used."
            // RFC7519 section 7.2 step 9.
            //
            // Huh? I'm afraid this is misstated...
            // "Message," as it is being used, was defined as the decoded payload
            // up above ( "Let the Message be the result of base64url decoding the
            // JWS Payload." RFC7519 section 7.2 step 7a. Cool! )

            // this should not happen unless I'm really off...
            if ( ! message . empty () )
            {
                int ch = message [ 0 ];
                if ( isalnum ( ch ) || ch == '-' || ch == '_' )
                {
                    // geee, this is base64url encoded after all!
                    pay_base64 = message;
                    message = decodeBase64URLString ( pay_base64, false );
                }
            }

            // 10. "Verify that the resulting octet sequence is a UTF-8-encoded
            //      representation of a completely valid JSON object conforming to
            //      RFC 7159 [RFC7159]; let the JWT Claims Set be this JSON object."
            // RFC7519 section 7.2 step 10.

            lim . recursion_depth = 50;
            JSONObjectRef claims = JSON :: parseObject ( lim, message );
        
            // can still fail based upon expiration
            // SKIPPED FOR INSPECTION

            // it is done.
            return UnverifiedJWTClaimSetRef ( new UnverifiedJWTClaimSet ( key, jose, claims ) );
        }
    }

    void JWTMgr :: validateStringOrURI ( const std :: string & str )
    {
    }

    void JWTMgr :: validateStringOrURI ( const JSONValueRef & value )
    {
        validateStringOrURI ( value -> toString () );
    }

#if JWT_TESTING
    static long long jwt_static_cur_time;

    void jwt_setStaticCurrentTime ( long long cur_time )
    {
        jwt_static_cur_time = cur_time;
    }
#endif
    
    long long int JWTMgr :: now ()
    {
#if JWT_TESTING
        if ( jwt_static_cur_time > 0 )
            return jwt_static_cur_time;
#endif
        return ( long long int ) time ( nullptr );
    }

    std :: string JWTMgr :: makeID ()
    {
        // this is nice enough:
        // the size of a binary UUID buffer is defined to accompany
        // the function that operates upon it. Of course, it does
        // not allow for any changes in this size over time and the
        // effects of dynamic linking or system updates. The functions
        // could have been protected against this by accepting a runtime
        // indication of what the software thinks is the buffer size.
        //
        // normally, we are supposed to simply say
        //    uuid_t uuid;
        // but while safe for the time period around which the
        // library was compiled, it is unsafe over time.
        //
        // to build in some buffer against this, we declare
        // a larger buffer.
        const size_t uuid_size = sizeof ( uuid_t );
        const size_t oversize_uuid_buffer_space = uuid_size * 4;
        unsigned char uuid [ oversize_uuid_buffer_space ];
        assert ( sizeof uuid % 16 == 0 );
        uuid_generate ( uuid );

        // this is absolutely AWFUL, especially for secure software.
        // passing in a simple pointer to a buffer with no indication of
        // buffer size? Again, there is the assumption that everyone KNOWS
        // the length of an ASCII representatio of a UUID ( 36 bytes
        // plus 1 NUL byte ) and that works for the current time period.
        const size_t ascii_UUID_char_count = 36;
        const size_t oversize_UUID_buffer_space = ascii_UUID_char_count * 4 + 1;
        const size_t aligned_buffer_size = ( ( oversize_UUID_buffer_space + 15 ) / 16 ) * 16;
        char buffer [ aligned_buffer_size ];
        assert ( sizeof buffer % 16 == 0 );
        uuid_unparse ( uuid, buffer );

        // had the function above returned the actual string size, assumed
        // to be 36, we could use it when constructing this string object.
        // but it's safer to scan for first NUL and use that.
        return std :: string ( buffer );
    }

    void JWTMgr :: setDefaultSkewAdjustment ( long long int dflt_skew_seconds )
    {
        if ( dflt_skew_seconds < 0 || dflt_skew_seconds > 3600 )
            throw JWTException ( __func__, __LINE__, "skew adjustment out of range: %lld", dflt_skew_seconds );

        dflt_skew = dflt_skew_seconds;
    }

    void JWTMgr :: finalizeClaims ( JSONObject & claims,
        long long int duration, long long int not_before )
    {
        long long int cur = now ();

        claims . addValue ( "iat", JSON :: makeInteger ( cur ) );

        if ( not_before <= 0 )
            not_before = cur;
        else
        {
            if ( not_before < cur )
                not_before = cur;

            claims . addValue ( "nbf", JSON :: makeInteger ( not_before ) );
        }

        if ( duration >= 0 )
        {
            claims . addValue ( "exp", JSON :: makeInteger ( not_before + duration ) );
        }
    }

    void JWTMgr :: verifyJWSHeader ( const JSONObject & jose )
    {
    }

    void JWTMgr :: verifyPayload ( const JSONObject & jose, const JSONObject & payload,
        long long int cur_time, long long int skew_secs )
    {
    }

}
