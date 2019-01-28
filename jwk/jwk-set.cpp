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

#include <ncbi/jwk.hpp>
#include <ncbi/jwa.hpp>

namespace ncbi
{

    bool JWKSet :: isEmpty () const noexcept
    {
        return map . empty ();
    }

    unsigned long int JWKSet :: count () const noexcept
    {
        return ( unsigned long int ) map . size ();
    }

    bool JWKSet :: contains ( const std :: string & kid ) const noexcept
    {
        auto it = map . find ( kid );
        return it != map . end ();
    }

    std :: vector < std :: string > JWKSet :: getKeyIDs () const
    {
        std :: vector < std :: string > kids;

        for ( auto it = map . begin (); it != map . end (); ++ it )
        {
            kids . push_back ( it -> first );
        }

        return kids;
    }

    void JWKSet :: addKey ( JWKRef & key )
    {
        std :: string kid = key -> getID ();

        auto it = map . find ( kid );
        if ( it != map . end () )
            throw JWKException ( __func__, __LINE__, "key-id '%s' exists", kid . c_str () );

        // locate the keys array from our props
        JSONArray & keys = kset -> getValue ( "keys" ) . toArray ();

        // obtain the new array index
        unsigned int idx = keys . count ();

        // clone the object
        JSONValueRef cpy ( key -> props -> clone () );

        // append to array
        keys . appendValue ( cpy );

        // verify that the insertion index was correct ( due to non-atomicity )
        assert ( keys [ idx ] . toObject () . getValue ( "kid" ) . toString () . compare ( kid ) == 0 );

        // insert idx and key into map under kid
        std :: pair < unsigned long int, JWKRef > entry ( idx, key );
        map . emplace ( kid, entry );
    }

    JWKRef JWKSet :: getKey ( const std :: string & kid ) const
    {
        auto it = map . find ( kid );
        if ( it == map . end () )
            throw JWKException ( __func__, __LINE__, "key-id '%s' not found", kid . c_str () );
        return it -> second . second;
    }

    void JWKSet :: removeKey ( const std :: string & kid )
    {
        auto it = map . find ( kid );
        if ( it != map . end () )
        {
            JSONArray & keys = kset -> getValue ( "keys" ) . toArray ();
            assert ( keys [ it -> second . first ] . toObject () . getValue ( "kid" ) . toString () . compare ( kid ) == 0 );
            keys . removeValue ( it -> second . first );
            map . erase ( it );
        }
    }

    JWKSetRef JWKSet :: clone () const
    {
        return JWKSetRef ( new JWKSet ( * this ) );
    }

    void JWKSet :: invalidate () noexcept
    {
        map . clear ();
        kset -> invalidate ();
    }

    JWKSet & JWKSet :: operator = ( const JWKSet & ks )
    {
        map . clear ();
        kset = ks . kset -> cloneObject ();

        extractKeys ();

        return * this;
    }

    JWKSet :: JWKSet ( const JWKSet & ks )
        : kset ( ks . kset -> cloneObject () )
    {
        extractKeys ();
    }

    JWKSet :: ~ JWKSet () noexcept
    {
        map . clear ();
        kset -> invalidate ();
    }

    void JWKSet :: extractKeys ()
    {
        const JSONArray & keys = kset -> getValue ( "keys" ) . toArray ();
        unsigned long int i, count = keys . count ();
        for ( i = 0; i < count; ++ i )
        {
            const JSONObject & key = keys [ i ] . toObject ();
            JSONObjectRef cpy ( key . cloneObject () );
            JWKRef jwk ( new JWK ( cpy ) );
            std :: string kid = key . getValue ( "kid" ) . toString ();
            std :: pair < unsigned long int, JWKRef > entry ( i, jwk );
            map . emplace ( kid, entry );
        }
    }

    JWKSet :: JWKSet ( const JSONObjectRef & _kset )
        : kset ( _kset )
    {
        extractKeys ();
    }

}
