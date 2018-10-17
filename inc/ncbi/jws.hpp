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

#ifndef _hpp_ncbi_oauth_jwa_
#include <ncbi/jwa.hpp>
#endif

#include <vector>

namespace ncbi
{
    // JSON Web Signature - RFC 7515: Line 350
    // A data structure representing a digitally signed or MACed message
    typedef std :: string JWS;
    
    class JWSFactory
    {
    public:
        
        // RFC 7515: Line 376
        // sign using compact serialization
        // signing input is:
        //   ASCII(BASE64URL(UTF8(JWS Protected Header)) || '.' || BASE64URL(JWS Payload))
        JWS signCompact ( JSONObject & hdr, const void * payload, size_t bytes ) const;
        
        // check that the JOSE header is completely understood
        // validates signature
        // or throw exception
        void validate ( const JSONObject & hdr, const JWS & jws, size_t last_period ) const;
        
        // additional verifiers
        void addVerifier ( const std :: string & alg, const std :: string & name, const std :: string & key );
        
        // copy construction
        JWSFactory & operator = ( const JWSFactory & fact );
        JWSFactory ( const JWSFactory & fact );
        
        // create a standard factory
        JWSFactory ( const std :: string & alg, const std :: string & name,
             const std :: string & signing_key, const std :: string & verify_key );
        ~ JWSFactory ();
        
    private:
        
        const JWASigner * signer;
        const JWAVerifier * verifier;
        std :: vector < const JWAVerifier * > addl_verifiers;
    };
}

#endif /* _hpp_ncbi_oauth_jws_ */
