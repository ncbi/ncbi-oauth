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

#ifndef _hpp_ncbi_jwr_
#define _hpp_ncbi_jwr_

#ifndef _hpp_ncbi_jwx_
#include <ncbi/jwx.hpp>
#endif

#include <memory>

/**
 * @file ncbi/jwr.hpp
 * @brief C++ reference-objects for JWT usage
 *
 * this type of structure is commonly known as a "smart-pointer"
 * and C++11 has introduced a number of versions while deprecating
 * the "auto_ptr".
 *
 * our JWT infrastructure will try to make use of these classes
 * to the extent possible. their direct usage has led to contortions
 * around actual behavior where this differs from desired behavior,
 * which results in avoidance of direct usage in order to ensure
 * controled and predictable behavior.
 */

namespace ncbi
{

    /**
     * @class JWRH
     * @brief the hot-potato reference
     *
     * ensures that only a single reference holds the object pointer,
     * while allowing for that pointer to be passed freely between
     * references.
     *
     * the point of such a reference is exactly for tracking an allocation
     * through a message chain, and ensuring that an exception at any
     * point will cause the object to be collected, while at the same
     * time allowing for efficient transfer of the pointer.
     */
    template < class T >
    class JWRH
    {
    public:

        T * release () const noexcept
        { return p . release (); }

        void reset ( T * ptr ) const noexcept
        { p . reset ( ptr ); }

        bool operator ! () const noexcept
        { return ! p; }

        T * operator -> () const noexcept
        { return p . get (); }

        T & operator * () const noexcept
        { return * p . get (); }

        bool operator == ( const JWRH < T > & r ) const noexcept
        { return p == r . p; }

        bool operator == ( std :: nullptr_t ) const noexcept
        { return p == nullptr; }

        bool operator == ( T * ptr ) const noexcept
        { return p == ptr; }

        bool operator != ( const JWRH < T > & r ) const noexcept
        { return p != r . p; }

        bool operator != ( std :: nullptr_t ) const noexcept
        { return p != nullptr; }

        bool operator != ( T * ptr ) const noexcept
        { return p != ptr; }

        JWRH & operator = ( T * ptr ) noexcept
        {
            p . reset ( ptr );
            return * this;
        }

        JWRH & operator = ( const JWRH < T > & r ) noexcept
        {
            p . reset ( r . p . release () );
            return * this;
        }

        JWRH ( const JWRH & r ) noexcept
            : p ( r . p . release () )
        {
        }

        JWRH ( T * ptr ) noexcept
            : p ( ptr )
        {
        }

        JWRH () noexcept {}
        ~ JWRH () noexcept {}

    private:

        mutable std :: unique_ptr < T > p;
    };

    /**
     * @class JWRS
     * @brief a shared, counted reference
     *
     * for our purposes, this allows for co-equal shared ownership
     * that is useful for immutable objects like JWKs.
     */
    template < class T >
    class JWRS
    {
    public:

        T * release () const noexcept
        { return p . release (); }

        void reset ( T * ptr ) const noexcept
        { p . reset ( ptr ); }

        bool unique () const noexcept
        { return p . unique (); }

        bool operator ! () const noexcept
        { return ! p; }

        T * operator -> () const noexcept
        { return p . get (); }

        T & operator * () const noexcept
        { return * p . get (); }

        bool operator == ( const JWRH < T > & r ) const noexcept
        { return p == r . p; }

        bool operator == ( std :: nullptr_t ) const noexcept
        { return p == nullptr; }

        bool operator == ( T * ptr ) const noexcept
        { return p == ptr; }

        bool operator != ( const JWRH < T > & r ) const noexcept
        { return p != r . p; }

        bool operator != ( std :: nullptr_t ) const noexcept
        { return p != nullptr; }

        bool operator != ( T * ptr ) const noexcept
        { return p != ptr; }

        JWRS & operator = ( T * ptr ) noexcept
        {
            p . reset ( ptr );
            return * this;
        }

        JWRS & operator = ( const JWRS < T > & r ) noexcept
        {
            p = r . p;
            return * this;
        }

        JWRS ( const JWRS & r ) noexcept
            : p ( r . p )
        {
        }

        JWRS ( T * ptr ) noexcept
            : p ( ptr )
        {
        }

        JWRS () noexcept {}
        ~ JWRS () noexcept {}

    private:

        mutable std :: shared_ptr < T > p;
    };

#if 0
    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JWRException, JWX );
#endif

}

#endif // _hpp_ncbi_jwr_
