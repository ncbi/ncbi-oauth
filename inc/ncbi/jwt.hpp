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

#ifndef _hpp_ncbi_jwx_
#include <ncbi/jwx.hpp>
#endif

#ifndef _hpp_ncbi_json_
#include <ncbi/json.hpp>
#endif

#include <atomic>
#include <string>

/**
 * @file ncbi/jwt.hpp
 * @brief JSON Web Tokens - RFC 7519
 */

namespace ncbi
{
    class JWK;
    class JWKSet;
    class JWTClaimSet;
    class InvalidJWTClaimSet;

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
         * @return JWTClaimSet object
         */
        static JWTClaimSet makeClaimSet ();

        /**
         * @fn sign
         * @brief sign a JWTClaimSet object
         * @param claims C++ reference to a JWTClaimSet object
         * @param key C++ reference to the JWK signing key
         * @return JWT string
         */
        static JWT sign ( const JWTClaimSet & claims, const JWK & key );

        /**
         * @fn decode
         * @overload decode a JWT into a JWTClaimSet object
         * @param jwt the JWT to decode
         * @param keys set of keys to validate signature and potentially decrypt
         * @exception JWSInvalidSignature
         * @exception JWTExpired
         * @return JWTClaimSet object
         * there are many possible errors that can occur when decoding.
         * the method will only return if all validations pass.
         */
        static JWTClaimSet decode ( const JWT & jwt, const JWKSet & keys );

        /**
         * @fn decode
         * @overload decode a JWT into a JWTClaimSet object with timestamp and skew
         * @param jwt the JWT to decode
         * @param keys set of keys to validate signature and potentially decrypt
         * @param cur_time an externally supplied timestamp for evaluating exiration
         * @param skew_secs an allowed margin of error between clock at origin and cur_time
         * @exception JWSInvalidSignature
         * @exception JWTExpired
         * @return JWTClaimSet object
         * there are many possible errors that can occur when decoding.
         * the method will only return if all validations pass.
         */
        static JWTClaimSet decode ( const JWT & jwt, const JWKSet & val_keys,
            long long cur_time, long long skew_secs = 0 );

        /**
         * @fn inspect
         * @brief decode a JWT into an InvalidJWTClaimSet object without most validity checks
         * @param jwt the JWT to decode
         * @param keys set of keys to validate signature and potentially decrypt
         * @param override_signature_check if true, inspect even after signature failure
         * @exception JWSInvalidSignature
         * @return InvalidJWTClaimSet object
         * there are many possible errors that can occur when decoding.
         * the method will only return if all validations pass.
         */
        static InvalidJWTClaimSet inspect ( const JWT & jwt, const JWKSet & keys,
            bool override_signature_check = false );

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
        static void validateStringOrURI ( const JSONValue * value );

        /**
         * @fn now
         * @return long long int timestamp in seconds since epoch
         */
        static long long int now ();
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
         * @brief set "iss" claim value
         * @param iss a StringOrURI representing the issuer
         */
        void setIssuer ( const StringOrURI & iss );

        /**
         * @fn setSubject
         * @brief set "sub" claim value
         * @param sub a StringOrURI representing the subject
         */
        void setSubject ( const StringOrURI & sub );

        /**
         * @fn addAudience
         * @brief set or add to "aud" claim array
         * @param aud a StringOrURI representing an audience
         */
        void addAudience ( const StringOrURI & aud );

        /**
         * @fn setNotBefore
         * @brief set "nbf" claim value
         * @param nbf_seconds the absolute time at which token becomes valid
         * if this value is not set, the token will become valid immediately
         */
        void setNotBefore ( long long int nbf_seconds );

        /**
         * @fn setDuration
         * @brief set token duration affecting "exp" claim value
         * @param dur_seconds the token duration in seconds
         * the "exp" claim will only be set upon signing,
         * at which time the absolute time at which the token
         * becomes valid ( "nbf" ) will be added to the duration
         * to produce "exp" in absolute time.
         */
        void setDuration ( long long int dur_seconds );


        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value
         * @return StringOrURI representing the issuer
         */
        StringOrURI getIssuer () const;

        /**
         * @fn getSubject
         * @brief get "sub" claim value
         * @return StringOrURI representing the subject
         */
        StringOrURI getSubject () const;

        /**
         * @fn getAudience
         * @brief get "aud" claim array
         * @return std :: vector < StringOrURI > representing all audiences
         */
        std :: vector < StringOrURI > getAudience () const;

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value or current time if not set
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const;

        /**
         * @fn getExpiration
         * @brief get "exp" claim value if set
         * @return long long int representing the absolute time of token expiration
         */
        long long int getExpiration () const;

        /**
         * @fn getDuration
         * @brief get token duration affecting "exp" claim value
         * @return long long int representing seconds between "exp" and token validity
         */
        long long int getDuration () const;


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
        void addClaim ( const std :: string & name, JSONValue * value );

        /**
         * @fn addClaimOrDeleteValue
         * @brief calls addClaim() and deletes value upon exceptions
         * @param name std::string with unique claim name
         * @param value a non-null JSONValue pointer
         * @exception JSONUniqueConstraintViolation if name exists
         * @exception JSONNullValue if val == nullptr
         * In C++, the temptation is to create JSONValue objects inline
         * to the addClaim expression. When using addClaim() directly,
         * this can lead to orphaned objects. Use this method instead to
         * indicate that the value object should be deleted upon error.
         */
        void addClaimOrDeleteValue ( const std :: string & name, JSONValue * value );

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

    protected:

    private:

        JSONObject * claims;              //!< claims are stored in a JSON object
        long long duration;               //!< offset to produce "exp" from start
        long long not_before;             //!< absolute time of "nbf"
        JWTLock obj_lock;                 //!< busy lock to prevent modification
     };

    /**
     * @class InvalidJWTClaims
     * @brief an object with unvalidated claims
     * in order to inspect a JWT even when invalid,
     * this class exists to hold the contents.
     */
    class InvalidJWTClaims
    {
    public:

        /*=================================================*
         *            REGISTERED CLAIM GETTERS             *
         *=================================================*/

        /**
         * @fn getIssuer
         * @brief get "iss" claim value
         * @return StringOrURI representing the issuer
         */
        StringOrURI getIssuer () const;

        /**
         * @fn getSubject
         * @brief get "sub" claim value
         * @return StringOrURI representing the subject
         */
        StringOrURI getSubject () const;

        /**
         * @fn getAudience
         * @brief get "aud" claim array
         * @return std :: vector < StringOrURI > representing all audiences
         */
        std :: vector < StringOrURI > getAudience () const;

        /**
         * @fn getNotBefore
         * @brief get "nbf" claim value or current time if not set
         * @return long long int representing the absolute time of token validity
         */
        long long int getNotBefore () const;

        /**
         * @fn getExpiration
         * @brief get "exp" claim value if set
         * @return long long int representing the absolute time of token expiration
         */
        long long int getExpiration () const;


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

    private:

        const JSONObject * claims;        //!< claims are stored in a JSON object
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWTException, JWX );

}

#endif // _hpp_ncbi_jwt_
