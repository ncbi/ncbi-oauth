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

#ifndef _hpp_ncbi_jwa_
#define _hpp_ncbi_jwa_

#ifndef _hpp_ncbi_jwr_
#include <ncbi/jwr.hpp>
#endif

#include <map>
#include <set>
#include <string>

/**
 * @file ncbi/jwa.hpp
 * @brief JSON Web Algorithm Management - RFC 7518
 *
 * API to the JWA component, exposing only elements needed
 * by a client application.
 */

namespace ncbi
{
    class JWK;
    class JWPayload;
    class JWASigner;
    class JWAVerifier;

    /**
     * @typedef JWASignerRef
     * @brief shared reference to a JWASigner
     */
    typedef JWRS < const JWASigner > JWASignerRef;

    /**
     * @typedef JWAVerifierRef
     * @brief shared reference to a JWAVerifier
     */
    typedef JWRS < const JWAVerifier > JWAVerifierRef;

    /**
     * @class JWAMgr
     * @brief globally accessible static access to JWA component
     *
     * Not properly a class, but a namespace for access functions.
     */
    class JWAMgr
    {
    public:

        /**
         * @fn acceptJWKAlgorithm
         * @brief a predicate function on whether a named algorithm is accepted
         * @param kty a key type of 'oct', 'RSA', or 'EC'
         * @param alg an algorithm name
         * @return Boolean true if the algorithm name is recognized
         * NB - a return value of true indicates that the key appears legitimate,
         * which is different from saying we will process it for signatures or
         * encryption. Instead, it means that we can understand JWKs or JWKSets.
         */
        static bool acceptJWKAlgorithm ( const std :: string & kty, const std :: string & alg ) noexcept;

        /**
         * @fn getSigner
         * @brief locate a signing algorithm by name
         * @param alg the name of a signing JWA
         * @return JWASignerRef interface to the algorithm implementation
         */
        static JWASignerRef getSigner ( const std :: string & alg );

        /**
         * @fn getVerifier
         * @brief locate a signnature verification algorithm by name
         * @param alg the name of a verifier JWA
         * @return JWAVerifierRef interface to the algorithm implementation
         */
        static JWAVerifierRef getVerifier ( const std :: string & alg );

    };


    /**
     * @class JWASigner
     * @brief a general interface to a specific signing algorithm implementation
     */
    class JWASigner
    {
    public:

        /**
         * @fn sign
         * @brief creates a base64URL-encoded signature of data
         * @param key a JWK appropriate for signing
         * @param data a non-null pointer to sequence of bytes being signed
         * @param bytes the number of bytes in "data"
         * @return a base64URL-encoded digital signature
         */
        virtual std :: string sign ( const JWK & key,
            const void * data, size_t bytes ) const = 0;

        /**
         * @fn ~JWASigner
         * @brief destroys and releases dynamically allocated resources
         */
        virtual ~ JWASigner ();

    protected:

        static std :: string getKeyProp ( const JWK & key, const std :: string & name );

        // accessible only to implementations
        JWASigner ();

    private:

        // triply emphasizes that this class cannot be instatiated
        void operator = ( const JWASigner & ignore );
        JWASigner ( const JWASigner & ignore );
    };


    /**
     * @class JWAVerifier
     * @brief a general interface to a specific signature verification algorithm
     */
    class JWAVerifier
    {
    public:

        /**
         * @fn verify
         * @brief verifies a signature against actual data
         * @param key a JWK appropriate for signature verification
         * @param data a non-null pointer to sequence of bytes being signed
         * @param bytes the number of bytes in "data"
         * @param binary_signature the signature to verify
         * @return true if the signature matches
         */
        virtual bool verify ( const JWK & key, const void * data, size_t bytes,
            const JWPayload & binary_signature ) const = 0;

        /**
         * @fn ~JWAVerifier
         * @brief destroys and releases dynamically allocated resources
         */
        virtual ~ JWAVerifier ();

    protected:

        static std :: string getKeyProp ( const JWK & key, const std :: string & name );

        // accessible only to implementations
        JWAVerifier ();

    private:

        // triply emphasizes that this class cannot be instatiated
        void operator = ( const JWAVerifier & ignore );
        JWAVerifier ( const JWAVerifier & ignore );
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWAException, JWX );

}

#endif // _hpp_ncbi_jwa_
