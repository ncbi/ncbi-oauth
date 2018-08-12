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

#ifndef _hpp_ncbi_oauth_jws_
#define _hpp_ncbi_oauth_jws_

#ifndef _hpp_ncbi_oauth_json_
#include <ncbi/json.hpp>
#endif

#include <vector>

namespace ncbi
{
    // JSON Web Signature
    // A data structure representing a digitally signed or MACed message
    typedef std :: string JWS;
    
    enum JWSType
    {
        jws_type_JWT
    };
    
    enum JWSAlgorithm
    {
        jws_alg_NONE,
        jws_alg_HS256,
        jws_alg_HS384,
        jws_alg_HS512,
        jws_alg_RS256,
        jws_alg_RS384,
        jws_alg_RS512
    };
    
    class JWSFactory
    {
    public:
        
        // sign using compact serialization
        // signing input is:
        //   ASCII(BASE64URL(UTF8(JWS Protected Header)) || '.' || BASE64URL(JWS Payload))
        JWS signCompact ( JWSType type, const void * payload, size_t bytes ) const;
        
        // verify a decoded JWS
        void verify ( const JSONObject & hdr, const void * payload, size_t pay_bytes, const void * signature, size_t sig_bytes ) const;
        
        // sign and verify keys
        void setSigningKeys ( JWSAlgorithm alg, const std :: string & sign, const std :: string & verify );
        
        // additional verification keys
        void addVerificationKey ( JWSAlgorithm alg, const std :: string verify );
        
        // copy construction
        JWSFactory & operator = ( const JWSFactory & fact );
        JWSFactory ( const JWSFactory & fact );
        
        // create a standard factory
        JWSFactory ();
        ~ JWSFactory ();
        
    private:
        
        std :: string sign_key;
        std :: string verify_key;
        std :: vector < std :: pair < std :: string, JWSAlgorithm > > addl_key;
        JWSAlgorithm alg;
    };
}

#endif /* _hpp_ncbi_oauth_jws_ */
