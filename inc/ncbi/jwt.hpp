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

#ifndef _hpp_ncbi_jwt_
#define _hpp_ncbi_jwt_

#ifndef _hpp_ncbi_jwk_
#include <ncbi/jwk.hpp>
#endif

#include <atomic>
#include <string>
#include <set>

/**
 * @file ncbi/jwt.hpp
 * @brief JSON Web Tokens - RFC 7519
 */

namespace ncbi
{
    class JWK;
    class JWKSet;
    class JWTClaimSet;
    class UnverifiedJWTClaimSet;

    /**
     * @typedef JWT
     * @brief a JSON Web Token - RFC 7519: Line 233
     * A string representing a set of claims as a JSON object
     */
    typedef std :: string JWT;

    /**
     * @typedef StringOrURI
     * @brief a string that may be a URI - RFC 7519: Line 273
     * if the string contains a ':', then it MUST be a URI [RFC3986]
     */
    typedef std :: string StringOrURI;

    /**
     * @typedef JWTClaimSetRef
     * @brief a reference to a JWTClaimSet
     */
    typedef JWRH < JWTClaimSet > JWTClaimSetRef;

    /**
     * @typedef UnverifiedJWTClaimSetRef
     * @brief a reference to an UnverifiedJWTClaimSet
     */
    typedef JWRH < UnverifiedJWTClaimSet > UnverifiedJWTClaimSetRef;
    
    /**
     * @class JWTMgr
     * @brief JWT Management
     * globally accessible factory functions
     */
    class JWTMgr
    {
    public:

        /**
         * @fn makeClaimSet
         * @brief create an empty JWTClaimSet object
         * @return JWTClaimSetRef object reference
         */
        static JWTClaimSetRef makeClaimSet ();

        /**
         * @fn sign
         * @brief sign a JWTClaimSet object
         * @param key C++ reference to the JWK signing key
         * @param claims C++ reference to a JWTClaimSet object
         * @return JWT string
         */
        static JWT sign ( const JWK & key, const JWTClaimSet & claims );

        /**
         * @fn nestedSign
         * @brief apply another signature to a JWT
         * @param key C++ reference to the JWK signing key
         * @param jwt an existing JWT
         * @return JWT string
         */
        static JWT nestedSign ( const JWK & key, const JWT & jwt );

        /**
         * @fn decode
         * @overload decode a JWT into a JWTClaimSet object
         * @param keys set of keys to validate signature and potentially decrypt
         * @param jwt the JWT to decode
         * @exception JWSInvalidSignature
         * @exception JWTExpired
         * @return JWTClaimSet object
         * there are many possible errors that can occur when decoding.
         * the method will only return if all validations pass.
         */
        static JWTClaimSetRef decode ( const JWKSet & keys, const JWT & jwt );

        /**
         * @fn decode
         * @overload decode a JWT into a JWTClaimSet object with timestamp and skew
         * @param keys set of keys to validate signature and potentially decrypt
         * @param jwt the JWT to decode
         * @param cur_time an externally supplied timestamp for evaluating exiration
         * @param skew_secs an allowed margin of error between clock at origin and cur_time
         * @exception JWSInvalidSignature
         * @exception JWTExpired
         * @return JWTClaimSet object
         * there are many possible errors that can occur when decoding.
         * the method will only return if all validations pass.
         */
        static JWTClaimSetRef decode ( const JWKSet & keys, const JWT & jwt,
            long long int cur_time, long long int skew_secs = 0 );

        /**
         * @fn nestedDecode
         * @brief attempts to validate content and remove a layer of nesting
         * @param keys set of keys to validate signature and potentially decrypt
         * @param jwt an in/out parameter with the JWT to decode and returned inner JWT
         * @return JWKRef of verification or decryption key
         */
        static JWKRef nestedDecode ( const JWKSet & keys, JWT & jwt );

        /**
         * @fn inspect
         * @overload decode a JWT into an UnverifiedJWTClaimSet without any validity checks
         * @param jwt the JWT to decode
         * @return UnverifiedJWTClaimSet object
         */
        static UnverifiedJWTClaimSetRef inspect ( const JWT & jwt );

        /**
         * @fn inspect
         * @overload decode a JWT into an UnverifiedJWTClaimSet object with signature check
         * @param keys set of keys to validate signature and potentially decrypt
         * @param jwt the JWT to decode
         * @exception JWSInvalidSignature
         * @return UnverifiedJWTClaimSet object
         */
        static UnverifiedJWTClaimSetRef inspect ( const JWKSet & keys,
            const JWT & jwt );

        /**
         * @fn validateStringOrURI
         * @overload checks a std::string for conformance
         * @param str a std::string to validate
         * @exception JWTInvalidStringOrURI
         */
        static void validateStringOrURI ( const std :: string & str );

        /**
         * @fn validateStringOrURI
         * @overload checks a const JSONValue pointer for conformance
         * @param str a std::string to validate
         * @exception JWTInvalidStringOrURI
         */
        static void validateStringOrURI ( const JSONValueRef & value );

        /**
         * @fn now
         * @return long long int timestamp in seconds since epoch
         */
        static long long int now ();

        /**
         * @fn makeID
         * @return std :: string with new JWT id
         */
        static std :: string makeID ();

        /**
         * @fn setDefaultSkewAdjustment
         * @brief set clock skew for timeout operations
         * @param dflt_skew_seconds absolute value of skew adjustment
         * in order to enable JWT interchange between hosts that are
         * not perfectly synchronized to absolute time-of-day,
         * an absolute value quantity will adjust for clock skew.
         */
        static void setDefaultSkewAdjustment ( long long int dflt_skew_seconds );

    private:

        static void finalizeClaims ( JSONObject & claims,
            long long int duration, long long int not_before );

        static void verifyJWSHeader ( const JSONObject & jose );
        static void verifyPayload ( const JSONObject & jose, const JSONObject & payload,
            long long int cur_time, long long int skew_secs );

        static long long int dflt_skew;

        friend class JWTClaimSet;
    };

    /**
     * @struct JWTLock
     * @brief an embedded object that allows busy-wait locking against modification
     */
    struct JWTLock
    {

        JWTLock ();
        ~ JWTLock ();
        
        mutable std :: atomic_flag flag;
    };

    /**
     * @class JWTLocker
     * @brief a C++ utility class to lock and guarantee unlock of a JWTLock
     */
    class JWTLocker
    {
    public:

        /**
         * @fn JWTLocker
         * @brief attempt to lock a JWTLock object
         * @param lock the JWTLock
         * @exception JWTLockBusy
         */
        JWTLocker ( const JWTLock & lock );

        /**
         * @fn ~JWTLocker
         * @brief releases lock on JWTLock
         */
        ~ JWTLocker ();
        
    private:

        const JWTLock & lock;             //!< hold on object being locked
    };

    /**
     * @class JWTClaimSet
     * @brief an object for holding a set of valid claims
     * claims may be registered or application specific
     */
    class JWTClaimSet
    {
    public:

        /*=================================================*
         *            REGISTERED CLAIM SETTERS             *
         *=================================================*/

        /**
         * @fn setIssuer
         * @brief set "iss" claim value (section 4.1.1)
         * @param iss a StringOrURI representing the issuer
         */
        void setIssuer ( const StringOrURI & iss );

        /**
         * @fn setSubject
         * @brief set "sub" claim value (section 4.1.2)
         * @param sub a StringOrURI representing the subject
         */
        void setSubject ( const StringOrURI & sub );

        /**
         * @fn addAudience
         * @brief set or add to "aud" claim array (section 4.1.3)
         * @param aud a StringOrURI representing an audience
         */
        void addAudience ( const StringOrURI & aud );

        /**
         * @fn setDuration
         * @brief set token duration affecting "exp" claim value (section 4.1.4)
         * @param dur_seconds the token duration in seconds
         * the "exp" claim will only be set upon signing,
         * at which time the absolute time at which the token
         * becomes valid ( "nbf" ) will be added to the duration
         * to produce "exp" in absolute time.
         */
        void setDuration ( long long int dur_seconds );

        /**
         * @fn setNotBefore
         * @brief set "nbf" claim value (section 4.1.5)
         * @param nbf_seconds the absolute time at which token becomes valid
         * if this value is not set, the token will become valid immediately
         */
        void setNotBefore ( long long int nbf_seconds );

        /**
         * @fn setID
         * @brief set "jti" claim value (section 4.1.7)
         * @param nbf_seconds the absolute time at which token becomes valid
         * if this value is not set, the token will become valid immediately
         */
        void setID ( const std :: string & unique_jti );


        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value (section 4.1.1)
         * @return StringOrURI representing the issuer
         */
        StringOrURI getIssuer () const;

        /**
         * @fn getSubject
         * @brief get "sub" claim value (section 4.1.2)
         * @return StringOrURI representing the subject
         */
        StringOrURI getSubject () const;

        /**
         * @fn getAudience
         * @brief get "aud" claim array (section 4.1.3)
         * @return std :: vector < StringOrURI > representing all audiences
         */
        std :: vector < StringOrURI > getAudience () const;

        /**
         * @fn getExpiration
         * @brief get "exp" claim value if set (section 4.1.4)
         * @return long long int representing the absolute time of token expiration
         */
        long long int getExpiration () const;

        /**
         * @fn getDuration
         * @brief get token duration affecting "exp" claim value
         * @return long long int representing seconds between "exp" and token validity
         */
        long long int getDuration () const;

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value if set (section 4.1.5)
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const;

        /**
         * @fn getIssuedAt
         * @brief get "iat" claim value if set (section 4.1.6)
         * @return long long int representing the absolute time of token creation
         */
        long long int getIssuedAt () const;

        /**
         * @fn getID
         * @brief get "jti" claim value if set (section 4.1.7)
         * @return std::string with ticket id
         */
        std :: string getID () const;


        /*=================================================*
         *               APPLICATION CLAIMS                *
         *=================================================*/

        /**
         * @fn addClaim
         * @brief adds an unregistered claim
         * @param name std::string with unique claim name
         * @param value a non-null JSONValue pointer
         * @exception JSONUniqueConstraintViolation if name exists
         * @exception JSONNullValue if val == nullptr
         */
        void addClaim ( const std :: string & name, const JSONValueRef & value );

        /**
         * @fn getNames
         * @return std::vector < std::string > of claim names
         */
        std :: vector < std :: string > getNames () const;

        /**
         * @fn getClaim
         * @brief attempts to find a claim by name and return its value
         * @param name std::string with the claim name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         */
        const JSONValue & getClaim ( const std :: string & name ) const;


        /*=================================================*
         *        APPLICATION JOSE HEADER MEMBERS          *
         *=================================================*/

        /**
         * @fn addHeader
         * @brief adds a JOSE header
         * @param name std::string with unique header member name
         * @param value a non-null JSONValue pointer
         * @exception JSONUniqueConstraintViolation if name exists
         * @exception JSONNullValue if val == nullptr
         *
         * the JOSE header is supposed to be generated ENTIRELY
         * by the JWS or JWE component since its purpose is to
         * describe the treatment given. Yet, we see that this is
         * abused and requires some level of support to interface
         * with existing implementations.
         */
        void addHeader ( const std :: string & name, const JSONValueRef & value );

        /**
         * @fn getHdrNames
         * @return std::vector < std::string > of header names
         */
        std :: vector < std :: string > getHdrNames () const;

        /**
         * @fn getHeader
         * @brief attempts to find a header by name and return its value
         * @param name std::string with the header name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         */
        const JSONValue & getHeader ( const std :: string & name ) const;
 

        /*=================================================*
         *                VERIFICATION KEY                 *
         *=================================================*/

        /**
         * @fn getVerificationKey
         * @brief retrieve signing key used to deode JWT
         * @return JWKRef to the signature verification key
         */
        JWKRef getVerificationKey () const;


        /*=================================================*
         *                  SERIALIZATION                  *
         *=================================================*/

        /**
         * @fn toJSON
         * @return C++ std::string with JSON representation of claims set
         */
        std :: string toJSON () const;

        /**
         * @fn readableJSON
         * @return C++ std::string with human-formatted JSON representation of claims set.
         * differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        std :: string readableJSON ( unsigned int indent = 0 ) const;


        /*=================================================*
         *                   C++ SUPPORT                   *
         *=================================================*/

        /**
         * @fn operator =
         * @brief assignment operator
         * @param claims source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         * will delete any current contents
         * clones contents of source object.
         */
        JWTClaimSet & operator = ( const JWTClaimSet & claims );

        /**
         * @fn JWTClaimSet
         * @overload copy constructor
         * @param claims source of contents to clone
         * clones contents of source object.
         */
        JWTClaimSet ( const JWTClaimSet & claims );

        /**
         * @fn ~JWTClaimSet
         * @brief deletes any contents and destroys internal structures
         */        
        virtual ~ JWTClaimSet ();

    private:

        JWTClaimSet ( const JSONObjectRef & jose, const JSONObjectRef & claims );
        JWTClaimSet ( const JWKRef & key,
            const JSONObjectRef & jose, const JSONObjectRef & claims );

        struct Reserved
        {
            Reserved ();
            ~ Reserved ();

            std :: set < std :: string > claims;
            std :: set < std :: string > hdrs;
        };

        static Reserved reserved;

        JSONObjectRef jose;               //!< headers are stored in a JSON object
        JSONObjectRef claims;             //!< claims are stored in a JSON object
        JWKRef verification_key;          //!< key used to verify innermost signature
        long long int duration;           //!< offset to produce "exp" from start
        long long int not_before;         //!< absolute time of "nbf"
        JWTLock obj_lock;                 //!< busy lock to prevent modification

        friend class JWTMgr;
     };

    /**
     * @class UnverifiedJWTClaimSet
     * @brief an object with unvalidated claims
     * in order to inspect a JWT even when invalid,
     * this class exists to hold the contents.
     */
    class UnverifiedJWTClaimSet
    {
    public:

        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value (section 4.1.1)
         * @return StringOrURI representing the issuer
         */
        StringOrURI getIssuer () const;

        /**
         * @fn getSubject
         * @brief get "sub" claim value (section 4.1.2)
         * @return StringOrURI representing the subject
         */
        StringOrURI getSubject () const;

        /**
         * @fn getAudience
         * @brief get "aud" claim array (section 4.1.3)
         * @return std :: vector < StringOrURI > representing all audiences
         */
        std :: vector < StringOrURI > getAudience () const;

        /**
         * @fn getExpiration
         * @brief get "exp" claim value if set (section 4.1.4)
         * @return long long int representing the absolute time of token expiration
         */
        long long int getExpiration () const;

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value if set (section 4.1.5)
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const;

        /**
         * @fn getIssuedAt
         * @brief get "iat" claim value if set (section 4.1.6)
         * @return long long int representing the absolute time of token creation
         */
        long long int getIssuedAt () const;

        /**
         * @fn getID
         * @brief get "jti" claim value if set (section 4.1.7)
         * @return long long int representing the absolute time of token creation
         */
        std :: string getID () const;


        /*=================================================*
         *               APPLICATION CLAIMS                *
         *=================================================*/

        /**
         * @fn getNames
         * @return std::vector < std::string > of claim names
         */
        std :: vector < std :: string > getNames () const;

        /**
         * @fn getClaim
         * @brief attempts to find a claim by name and return its value
         * @param name std::string with the claim name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         */
        const JSONValue & getClaim ( const std :: string & name ) const;


        /*=================================================*
         *        APPLICATION JOSE HEADER MEMBERS          *
         *=================================================*/

        /**
         * @fn getHdrNames
         * @return std::vector < std::string > of header names
         */
        std :: vector < std :: string > getHdrNames () const;

        /**
         * @fn getHeader
         * @brief attempts to find a header by name and return its value
         * @param name std::string with the header name
         * @exception JSONNoSuchName if name is not a member of claims set
         * @return const JSONValue reference to existing value
         */
        const JSONValue & getHeader ( const std :: string & name ) const;


        /*=================================================*
         *                VERIFICATION KEY                 *
         *=================================================*/

        /**
         * @fn getVerificationKey
         * @brief retrieve signing key used to deode JWT
         * @return JWKRef to the signature verification key
         */
        JWKRef getVerificationKey () const;


        ~ UnverifiedJWTClaimSet ();

    private:

        // created in response to an inspection
        UnverifiedJWTClaimSet ( const JSONObjectRef & jose, const JSONObjectRef & claims );
        UnverifiedJWTClaimSet ( const JWKRef & key,
           const JSONObjectRef & jose, const JSONObjectRef & claims );

        // hidden from use
        void operator = ( const UnverifiedJWTClaimSet & ignore );
        UnverifiedJWTClaimSet ( const UnverifiedJWTClaimSet & ignore );

        const JSONObjectRef jose;         //!< claims are stored in a JSON object
        const JSONObjectRef claims;       //!< claims are stored in a JSON object
        JWKRef verification_key;          //!< key used to verify innermost signature

        friend class JWTMgr;
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWTException, JWX );

}

#endif // _hpp_ncbi_jwt_
