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

#ifndef _hpp_ncbi_jwp_
#define _hpp_ncbi_jwp_

#ifndef _hpp_ncbi_jwx_
#include <ncbi/jwx.hpp>
#endif


/**
 * @file ncbi/jwp.hpp
 * @brief a payload abstraction for JWT usage
 */

namespace ncbi
{

    /**
     * @class JWPayload
     * @brief contains binary data in a managed package
     */
    class JWPayload
    {
    public:

        /**
         * @fn data
         * @overload non-const version
         * @return non-const pointer to buffer
         */
        unsigned char * data ()
        { return buff; }

        /**
         * @fn data
         * @overload const version
         * @return const pointer to buffer
         */
        const unsigned char * data () const
        { return buff; }

        /**
         * @fn size
         * @return size of data in bytes
         */
        size_t size () const
        { return sz; }

        /**
         * @fn capacity
         * @return size of buffer in bytes
         */
        size_t capacity () const
        { return cap; }

        /**
         * @fn setSize
         * @brief record actual size
         * @param amt actual size in bytes
         */
        void setSize ( size_t amt );

        /**
         * @fn increaseCapacity
         * @brief increase buffer size
         * @param amt amount of size increment
         */
        void increaseCapacity ( size_t amt = 256 );

        /**
         * @fn JWPayload
         * @overload default constructor
         */
        JWPayload ();

        /**
         * @fn JWPayload
         * @overload create zero size payload with initial buffer capacity
         * @param initial_capacity value in bytes of initial buffer size
         */
        JWPayload ( size_t initial_capacity );

        /**
         * @fn JWPayload
         * @overload copy constructor
         * @param payload source from which contents are STOLEN
         * source object will be empty afterward
         */
        JWPayload ( const JWPayload & payload );

        /**
         * @fn operator =
         * @brief copy operator
         * @param payload source from which contents are STOLEN
         * @return C++ self-reference for use in idiomatic C++ expressions
         * source object will be empty afterward
         */
        JWPayload & operator = ( const JWPayload & payload );

        /**
         * @fn ~JWPayload
         * @brief delete buffer if present
         */
        ~ JWPayload ();

    private:

        mutable unsigned char * buff;
        mutable size_t sz, cap;
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWPException, JWX );

}

#endif // _hpp_ncbi_jwp_
