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

#ifndef _hpp_ncbi_jwk_
#define _hpp_ncbi_jwk_

#ifndef _hpp_ncbi_jwx_
#include <ncbi/jwx.hpp>
#endif

#include <atomic>
#include <string>
#include <vector>
#include <map>

/**
 * @file ncbi/jwk.hpp
 * @brief JSON Web Key Management - RFC 7517
 *
 * API to the JWK component, exposing only elements needed
 * by a client application.
 */

namespace ncbi
{
    class JWK;
    class JWKSet;

    /**
     * @class JWKMgr
     * @brief JSON Web Key Manager
     * globally accessible factory functions
     */
    class JWKMgr
    {
    public:

        /* DISCUSSION

           Life starts with an authentication, outside of the scope of JWK.

           Via the authentication token, a program requests its root tokens,
           that are stored in the form of a hierarchical tree like a file system.

           The tokens can be navigated and will contain private and shared
           branches. For interfacing with JWT/JWS/JWE/JWA/JWK, the program
           may access one (or more?) private signing keys as individual
           serialized JWKs, and will also access serialized JWKSets that
           are almost certainly shared.

           The program will then use what amounts to an opaque JWK for signing
           or encryption operations, the individual key being private or public
           respectively, and a JWKSet for verification or decryption operations,
           the contained keys being public or private respectively.

           Once the secret-store service is up and running, the actual private
           keys will be stored within the service process space, and the JWK
           objects within the client process space will be stubbed to remove
           sensitive properties.

           But if I read what I just wrote a second ago, it means that you
           obtain your starting JWKs and JWKSets from your root token tree,
           which works with this library behind the scenes.

           So by one way or another, the process in question obtains some token
           of authentication. If interactive, it may get a user's token. Otherwise,
           it may get its own token based upon attestation of some sort. The
           authentication token assigned and perhaps bound to the running process
           is then usable to obtain the root token tree for that identity. The
           program then navigates the tree to find JWKs and JWKSets of interest.

           The utility of converting JSON and PEM into JWKs and JWKSets within
           the JWKMgr is both for use by the token tree code as a client, and
           for programs that are interacting with external sources of key data.

           The program is then able to make use of JWT/JWS/JWE using these keys.
           JWA involvement is almost zero from the client's perspective, being
           used by JWS/JWE for implementation.

         */

        /**
         * @fn parseJWK
         * @brief inflate a JWK from JSON text
         * @param json_text a serialized version of a JWK
         * @return const JWK pointer
         */
        static const JWK * parseJWK ( const std :: string & json_text );

        /**
         * @fn parseJWKSet
         * @brief inflate a JWK from JSON text
         * @param json_text a serialized version of a JWKSet
         * @return const JWKSet pointer
         */
        static JWKSet * parseJWKSet ( const std :: string & json_text );

        /**
         * @fn parsePEM
         * @overload inflate public JWK from PEM text format
         * @param pem_text an encrypted PEM object
         * @param use the intended key usage "sig" or "enc"
         * @param alg the algorithm to apply with key, e.g. "RS256"
         * @param kid a globally unique identifier for this key
         * @return a const JWK pointer
         */
        static const JWK * parsePEM ( const std :: string & pem_text,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        /**
         * @fn parsePEM
         * @overload inflate private JWK from PEM text format
         * @param pem_text an encrypted PEM object
         * @param pwd password for decrypting PEM
         * @param use the intended key usage "sig" or "enc"
         * @param alg the algorithm to apply with key, e.g. "RS256"
         * @param kid a globally unique identifier for this key
         * @return a const JWK pointer
         */
        static const JWK * parsePEM ( const std :: string & pem_text, const std :: string & pwd,
            const std :: string & use, const std :: string & alg, const std :: string & kid );
    };

    /**
     * @class JWK
     * @brief a partially-opaque (translucent?) representation of a key
     */
    class JWK
    {
    public:

        /*=================================================*
         *                TYPE PREDICATES                  *
         *=================================================*/

        /**
         * @fn forSigning
         * @return Boolean true if this key is intended for signatures
         */
        virtual bool forSigning () const;

        /**
         * @fn forEncryption
         * @return Boolean true if this key is intended for encryption
         */
        virtual bool forEncryption () const;

        /**
         * @fn isPrivate
         * @return Boolean true if the key contains private material
         * a private key is required for signing or decryption
         */
        virtual bool isPrivate () const;

        /**
         * @fn isSymmetric
         * @return Boolean true if the key is symmetric
         */
        virtual bool isSymmetric () const;

        /**
         * @fn isRSA
         * @return Boolean true if the key is for use in RSA algorithms
         */
        virtual bool isRSA () const;

        /**
         * @fn isEllipticCurve
         * @return Boolean true if the key is for use in elliptical curve algorithms
         */
        virtual bool isEllipticCurve () const;


        /*=================================================*
         *          REGISTERED PROPERTY GETTERS            *
         *=================================================*/

        /**
         * @fn getType
         * @return std::string with value of "kty" property
         * this property is MANDATORY in a JWK (section 4.1)
         * legal values are { "oct", "RSA", "EC" }
         */
        std :: string getType () const;

        /**
         * @fn getId
         * @return std::string with value of "kid" property
         * \exception PropertyNotFound if the property is not present
         * this property is considered OPTIONAL under RFC (section 4.5)
         * but may be configured to be MANDATORY in this implementation.
         */
        std :: string getID () const;

        /**
         * @fn getAlg
         * @return std:: string with value of "alg" property
         * this property is OPTIONAL (section 4.4)
         */
        std :: string getAlg () const;

        /**
         * @fn getUse
         * @return std:: string with value of "use" property
         * this property is needed for public keys (section 4.2)
         * legal values are "sig" (signature) and "enc" (encryption)
         */
        std :: string getUse () const;


        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        /**
         * @fn toJSON
         * @return C++ std::string with JSON representation of properties
         * NB - this representation may not be universally compatible
         * with other systems. In particular, sensitive information
         * may be encrypted.
         */
        std :: string toJSON () const;

        /**
         * @fn readableJSON
         * @return C++ std::string with human-formatted JSON representation of properties..
         * differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        std :: string readableJSON ( unsigned int indent = 0 ) const;


        /*=================================================*
         *         PRIMITIVE MEMORY MANAGEMENT             *
         *=================================================*/

        /**
         * @fn duplicate
         * @brief create a duplicate pointer REFERENCE
         * @return const JWK pointer
         * the underlying implementation will either clone the object
         * or increment a reference count to the object, at its discretion.
         */
        const JWK * duplicate () const;

        /**
         * @fn release
         * @brief release a REFERENCE to the object
         * this message replaces the use of the delete operator
         * and the exact behavior will depend upon whether the
         * duplicate method was to clone or to increment a counter.
         * if duplicate cloned the object, then the method will
         * delete the object. Otherwise, it will decrement the
         * reference counter and only delete if the count goes to 0.
         */
        void release () const;

    protected:

        virtual ~ JWK ();

        JWK ( JSONObject * props );

        JSONObject * props;
        mutable std :: atomic < unsigned int > refcount;

        friend class JWKSet;
    };


    /**
     * @class JWKSet
     * @brief a set of JWK objects, indexed by kid
     */
    class JWKSet
    {
    public:

        bool isEmpty () const;
        unsigned long int count () const;

        bool contains ( const std :: string & kid ) const;

        std :: vector < std :: string > getKeyIDs () const;

        void addKey ( const JWK * jwk );
        const JWK * getKey ( const std :: string & kid ) const;
        void removeKey ( const std :: string & kid );

        JWKSet ( const JWKSet & ks );
        JWKSet & operator = ( const JWKSet & ks );

        JWKSet ();
        ~ JWKSet ();

    private:

        JSONObject * kset;
        std :: map < std :: string, const JWK * > map;
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWKException, JWX );
}

#endif // _hpp_ncbi_jwk_
