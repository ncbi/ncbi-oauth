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

#ifndef _hpp_ncbi_jwx_
#include <ncbi/jwx.hpp>
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
         * @return Boolean true if the algorithm name is recognized and accepted
         */
        static bool acceptJWKAlgorithm ( const std :: string & kty, const std :: string & alg );
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWAException, JWX );

}

#endif // _hpp_ncbi_jwa_
