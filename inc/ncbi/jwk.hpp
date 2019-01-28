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

#ifndef _hpp_ncbi_json_
#include <ncbi/json.hpp>
#endif

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
     * @typedef JWKRef
     * @brief shared reference to a JWK
     */
    typedef JWRS < const JWK > JWKRef;

    /**
     * @typedef JWKSetRef
     * @brief unique reference to a JWKSet
     */
    typedef JWRH < JWKSet > JWKSetRef;

    /**
     * @class JWKMgr
     * @brief JSON Web Key Manager
     * globally accessible factory functions
     */
    class JWKMgr
    {
    public:

        /**
         * @fn makeJWKSet
         * @brief create an empty JWKSet
         * @return JWKSetRef
         */
        static JWKSetRef makeJWKSet ();

        /**
         * @fn parseJWK
         * @brief inflate a JWK from JSON text
         * @param json_text a serialized version of a JWK
         * @return JWKRef
         */
        static JWKRef parseJWK ( const std :: string & json_text );

        /**
         * @fn parseJWKSet
         * @brief inflate a JWK from JSON text
         * @param json_text a serialized version of a JWKSet
         * @return JWKSetRef
         */
        static JWKSetRef parseJWKSet ( const std :: string & json_text );

        /**
         * @fn parsePEM
         * @overload inflate public JWK from PEM text format
         * @param pem_text an encrypted PEM object
         * @param use the intended key usage "sig" or "enc"
         * @param alg the algorithm to apply with key, e.g. "RS256"
         * @param kid a globally unique identifier for this key
         * @return JWKRef
         */
        static JWKRef parsePEM ( const std :: string & pem_text,
            const std :: string & use, const std :: string & alg, const std :: string & kid );

        /**
         * @fn parsePEM
         * @overload inflate private JWK from PEM text format
         * @param pem_text an encrypted PEM object
         * @param pwd password for decrypting PEM
         * @param use the intended key usage "sig" or "enc"
         * @param alg the algorithm to apply with key, e.g. "RS256"
         * @param kid a globally unique identifier for this key
         * @return JWKRef
         */
        static JWKRef parsePEM ( const std :: string & pem_text, const std :: string & pwd,
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
         * @return Boolean true if this key is intended for signing
         * detects private or symmetrical signing keys
         */
        bool forSigning () const noexcept;

        /**
         * @fn forVerifying
         * @return Boolean true if this key is intended for signature verification
         * detects public or symmetrical signature verification keys
         */
        bool forVerifying () const noexcept;

        /**
         * @fn forEncryption
         * @return Boolean true if this key is intended for encrypting
         * detects public or symmetrical encryption keys
         */
        bool forEncryption () const noexcept;

        /**
         * @fn forDecryption
         * @return Boolean true if this key is intended for decryption
         * detects private or symmetrical encryption keys
         */
        bool forDecryption () const noexcept;

        /**
         * @fn isPrivate
         * @return Boolean true if the key contains private material
         * a private key is required for signing or decryption
         */
        bool isPrivate () const noexcept;

        /**
         * @fn isSymmetric
         * @return Boolean true if the key is symmetric
         */
        bool isSymmetric () const noexcept;

        /**
         * @fn isRSA
         * @return Boolean true if the key is for use in RSA algorithms
         */
        bool isRSA () const noexcept;

        /**
         * @fn isEllipticCurve
         * @return Boolean true if the key is for use in elliptical curve algorithms
         */
        bool isEllipticCurve () const noexcept;


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
         * @fn getUse
         * @return std:: string with value of "use" property
         * this property is needed for public keys (section 4.2)
         * legal values are "sig" (signature) and "enc" (encryption)
         */
        std :: string getUse () const;

        /**
         * @fn getOperations
         * @return std::vector<std::string> with value of "key_ops" property
         * this property is an alternate for "use" (section 4.3)
         * registered values are:
         * "sign", "verify", "encrypt", "decrypt", "wrapKey",
         * "unwrapKey", "deriveKey", "deriveBits"
         */
        std :: vector < std :: string > getOperations () const;

        /**
         * @fn getAlg
         * @return std:: string with value of "alg" property
         * this property is OPTIONAL (section 4.4)
         */
        std :: string getAlg () const;

        /**
         * @fn getId
         * @return std::string with value of "kid" property
         * \exception PropertyNotFound if the property is not present
         * this property is considered OPTIONAL under RFC (section 4.5)
         * but has been configured to be MANDATORY in this implementation.
         */
        std :: string getID () const;


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


        /**
         * @fn ~JWK
         * @brief deletes any contents and destroys internal structures
         */        
        ~ JWK () noexcept;


    private:

        JWK ( const JSONObjectRef & props );

        JSONObjectRef props;

        friend class JWKMgr;
        friend class JWKSet;
    };


    /**
     * @class JWKSet
     * @brief a set of JWK objects, indexed by kid
     */
    class JWKSet
    {
    public:

        /**
         * @fn isEmpty
         * @return Boolean true if set has no keys
         */
        bool isEmpty () const noexcept;

        /**
         * @fn count
         * @return Natural number with the set cardinality
         */
        unsigned long int count () const noexcept;

        /**
         * @fn contains
         * @brief answers whether the indicated entry exists
         * @param kid std::string with the key identifier
         * @return Boolean true if entry exists
         */
        bool contains ( const std :: string & kid ) const noexcept;

        /**
         * @fn getKeyIDs
         * @return std::vector<std::string> of key identifiers
         */
        std :: vector < std :: string > getKeyIDs () const;

        /**
         * @fn addKey
         * @brief add a new JWK
         * @param key const JWKRef
         */
        void addKey ( JWKRef & key );

        /**
         * @fn getKey
         * @brief return JWK for key identifier
         * @param kid std::string with key id
         * @return JWKRef
         */
        JWKRef getKey ( const std :: string & kid ) const;

        /**
         * @fn removeKey
         * @brief remove and release JWK found with kid
         * @param kid std::string with key id
         * ignored if entry is not found
         */
        void removeKey ( const std :: string & kid );

        /**
         * @fn clone
         * @return creates a deep copy of set
         */
        JWKSetRef clone () const;

        /**
         * @fn invalidate
         * @brief overwrite potentially sensitive contents in memory
         */
        void invalidate () noexcept;

        /**
         * @fn operator =
         * @brief assignment operator
         * @param ks source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         * will delete any current contents
         * clones contents of source set
         */
        JWKSet & operator = ( const JWKSet & ks );

        /**
         * @fn JWKSet
         * @overload copy constructor
         * @param ks source of contents to clone
         * clones contents of source set
         */
        JWKSet ( const JWKSet & ks );

        /**
         * @fn ~JWKSet
         * @brief deletes any contents and destroys internal structures
         */        
        ~ JWKSet () noexcept;

    private:

        void extractKeys ();

        JWKSet ( const JSONObjectRef & kset );

        JSONObjectRef kset;
        std :: map < std :: string, std :: pair < unsigned long int, JWKRef > > map;

        friend class JWKMgr;
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWKException, JWX );
}

#endif // _hpp_ncbi_jwk_
