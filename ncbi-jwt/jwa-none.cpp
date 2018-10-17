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

#include <ncbi/jwa.hpp>
#include <ncbi/jwt.hpp>

#include <iostream>
#include <cstring>
#include <cassert>

namespace ncbi
{
    struct NONE_Signer : JWASigner
    {
        virtual std :: string sign ( const void * data, size_t bytes ) const
        {
            return "";
        }
        
        virtual JWASigner * clone () const
        {
            return new NONE_Signer ( alg, nam, key );
        }

        NONE_Signer ( const std :: string & alg,
                const std :: string & name, const std :: string & key )
            : JWASigner ( alg, name, key )
        {
        }
    };

    struct NONE_Verifier : JWAVerifier
    {
        virtual void verify ( const void * data, size_t bytes, const std :: string & signature ) const
        {
        }
        
        virtual JWAVerifier * clone () const
        {
            return new NONE_Verifier ( alg, nam, key );
        }

        NONE_Verifier ( const std :: string & alg,
                const std :: string & name, const std :: string & key )
            : JWAVerifier ( alg, name, key )
        {
        }
    };
    
    struct NONE_SignerFact : JWASignerFact
    {
        virtual JWASigner * make ( const std :: string & alg,
            const std :: string & name, const std :: string & key ) const
        {
            return new NONE_Signer ( alg, name, key );
        }

        NONE_SignerFact ()
        {
            gJWAFactory . registerSignerFact ( "none", this );
        }
    };

    struct NONE_VerifierFact : JWAVerifierFact
    {
        virtual JWAVerifier * make ( const std :: string & alg,
            const std :: string & name, const std :: string & key ) const
        {
            return new NONE_Verifier ( alg, name, key );
        }

        NONE_VerifierFact ()
        {
            gJWAFactory . registerVerifierFact ( "none", this );
        }
    };

    static NONE_SignerFact none_signer_fact;
    static NONE_VerifierFact none_verifier_fact;

    void includeJWA_none ( bool always_false )
    {
        if ( always_false )
        {
            std :: string empty;
            none_signer_fact . make ( empty, empty, empty );
            none_verifier_fact . make ( empty, empty, empty );
        }
    }
}
