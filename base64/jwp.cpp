/*===========================================================================
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

#include <ncbi/jwp.hpp>
#include "memset-priv.hpp"

#include <cstdint>
#include <cstring>
#include <cassert>
#include <string>

namespace ncbi
{
    // JWPayload
    void JWPayload :: setSize ( size_t amt )
    {
        if ( amt != 0 || buff != nullptr )
        {
            if ( amt > cap || buff == nullptr )
                throw JWPException ( __func__, __LINE__, "illegal payload size" );

            sz = amt;
            buff [ amt ] = 0;
        }
    }
    
    void JWPayload :: increaseCapacity ( size_t amt )
    {
        unsigned char * new_buff = new unsigned char [ cap + amt + 1 ];
        memmove ( new_buff, buff, sz );
        delete [] buff;
        cap += amt;
        buff = new_buff;
    }

    void JWPayload :: erase () noexcept
    {
        if ( buff != nullptr )
        {
            memset_while_respecting_language_semantics
                ( buff, cap, 0, cap, ( const char * ) buff );

            delete [] buff;
            buff = nullptr;
        }

        sz = cap = 0;
    }

    JWPayload :: JWPayload () noexcept
        : buff ( nullptr )
        , sz ( 0 )
        , cap ( 0 )
    {
    }

    JWPayload :: JWPayload ( size_t initial_capacity )
        : buff ( nullptr )
        , sz ( 0 )
        , cap ( initial_capacity )
    {
        buff = new unsigned char [ cap + 1 ];
    }

    JWPayload :: JWPayload ( const JWPayload & payload ) noexcept
        : buff ( payload . buff )
        , sz ( payload . sz )
        , cap ( payload . cap )
    {
        payload . buff = nullptr;
        payload . sz = payload . cap = 0;
    }

    JWPayload & JWPayload :: operator = ( const JWPayload & payload ) noexcept
    {
        delete [] buff;

        buff = payload . buff;
        sz = payload . sz;
        cap = payload . cap;
        
        payload . buff = nullptr;
        payload . sz = payload . cap = 0;

        return * this;
    }

    JWPayload :: ~ JWPayload () noexcept
    {
        erase ();
    }
    
}
