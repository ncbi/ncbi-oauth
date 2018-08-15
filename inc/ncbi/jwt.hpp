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

#ifndef _hpp_ncbi_oauth_jwt_
#define _hpp_ncbi_oauth_jwt_

#ifndef _hpp_ncbi_oauth_json_
#include <ncbi/json.hpp>
#endif

namespace ncbi
{
    class JWSFactory;
    class JWTFactory;
    class JWTFixture_BasicConstruction;
    
    // a JSON Web Token - RFC 7519: Line 233
    // A string representing a set of claims as a JSON object
    typedef std :: string JWT;
    
    // RFC 7519: Line 273
    // if the string contains a ':', then it MUST be a URI [RFC3986]
    typedef std :: string StringOrURI;
    
    /* JWTException
     **********************************************************************************/
    class JWTException : public std :: logic_error
    {
    public:
        
        virtual const char * what () const
        throw ();
        
        explicit JWTException ( const char * function, unsigned int line, const char * message );
        virtual ~JWTException ()
        throw ();
        
    private:
        
        std :: string msg;
        const char * fl_msg;
    };
    
    // A JSON object that contains the claims conveyed by the JWT
    class JWTClaims
    {
    public:
        
        // registered claims
        void setIssuer ( const StringOrURI & iss );
        void setSubject ( const StringOrURI & sub );
        void addAudience ( const StringOrURI & aud );
        void setDuration ( long long int dur_seconds );
        void setNotBefore ( long long int nbf_seconds );

        // claims can be any valid JSONValue
        void addClaim ( const std :: string & name, JSONValue * value, bool isFinal = false );
        const JSONValue & getClaim ( const std :: string & name ) const;
        
        // C++ assignment
        JWTClaims & operator = ( const JWTClaims & jwt );
        JWTClaims ( const JWTClaims & jwt );
        
    private:
        
        // any std :: string parameter typed as StringOrURI MUST be validated
        // throws an exception for an invalid string
        static void validateStringOrURI ( const std :: string & str );
        
        JWTClaims ();
        JWTClaims ( JSONObject * claims );

        JSONObject * claims;

        friend class JWTFactory;
        friend class JWTFixture_BasicConstruction;
    };

    class JWTFactory
    {
    public:

        // make a new, more or less empty JWT object
        JWTClaims make () const;
        
        // decode a signed JWT
        JWTClaims decode ( const JWSFactory & jws_fact, const JWT & jwt ) const;
        
        // create a signed JWT as a compact JWS from the claims set
        JWT signCompact ( const JWSFactory & jws_fact, const JWTClaims & claims ) const;

        // registered claim factory parameters
        void setIssuer ( const StringOrURI & iss );
        void setSubject ( const StringOrURI & sub );
        void addAudience ( const StringOrURI & aud );
        void setDuration ( long long int dur_seconds );
        void setNotBefore ( long long int nbf_seconds );

        // copy construction
        JWTFactory & operator = ( const JWTFactory & jwt_fact );
        JWTFactory ( const JWTFactory & jwt_fact );
        
        // create a standard factory
        JWTFactory ();
        ~ JWTFactory ();

    private:
        
        // make a new identifier
        std :: string newJTI () const;
        
        // return timestamp in seconds since epoch
        static long long int now ();

        std :: string iss;
        std :: string sub;
        std :: vector < std :: string > aud;
        long long duration;
        long long not_before;

        static std :: atomic < unsigned long long > id_seq;
    };
}

#endif /* _hpp_ncbi_oauth_jwt_ */
