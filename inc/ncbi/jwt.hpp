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

#include <string>

namespace ncbi
{
    class JWTFixture_BasicConstruction;
    
    class JWT
    {
    public:
        
        // make an empty JWT
        static JWT make ();
        
        // make a JWT from an encoded string
        static JWT decode ( const std :: string & encoding, const std :: string & pub_key );
        
        // create a simple, unsigned JWT
        std :: string encode () const;
        
        // access the JOSE header
        const JSONObject & header () const;
        
        // access the payload
        JSONObject & payload ();
        const JSONObject & payload () const;
        
        // C++ assignment
        JWT & operator = ( const JWT & jwt );
        JWT ( const JWT & jwt );
        
        virtual ~JWT ();
        
    private:
        
        JWT ( JSONObject * hdr, JSONObject * pay );

        JSONObject * hdr;
        JSONObject * pay;

        friend class JWTFixture_BasicConstruction;
    };
}

#endif /* _hpp_ncbi_oauth_jwt_ */

