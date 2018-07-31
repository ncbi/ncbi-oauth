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

#include "jwt.hpp"
#include "base64-priv.hpp"

#include <iostream>

namespace ncbi
{
    JWT * JWT :: make ()
    {
        // make the header
        JWT *jwt = new JWT ();
        if ( jwt )
        {
            jwt -> header [ "alg" ] = "RS512";
            jwt -> header [ "typ" ] = "JWT";
            
            jwt -> payload [ "iss" ] = "Batman";
            jwt -> payload [ "sub" ] = "Joker";
            jwt -> payload [ "aud" ] = "Gotham";
            jwt -> payload [ "iat" ] = 0;
            jwt -> payload [ "exp" ] = 10;
        }
        
        return jwt;
    }
    
    JWT * JWT :: make ( const std :: string & encoding )
    {
        return nullptr;
    }
    
    std :: string JWT :: encode () const
    {
        std :: string encoding = encodeBase64URL ( header . toJSON () ) + ".";
        encoding += encodeBase64URL( payload.toJSON() );
        
        return encoding;
    }
    
    void JWT :: decode ( const std :: string &encoding )
    {
    }
    
    void JWT :: addClaim ( const std :: string & name, bool value )
    {
        payload [ name ] = value;
    }
    
    void JWT :: addClaim ( const std :: string & name, long long int value )
    {
        payload [ name ] = value;
    }
    
    void JWT :: addClaim ( const std :: string & name, long double value )
    {
        payload [ name ] = value;
    }
    
    void JWT :: addClaim ( const std :: string & name, const char * value )
    {
        payload [ name ] = value;
    }
    
    void JWT :: addClaim ( const std :: string & name, const std :: string & value )
    {
        payload [ name ] = value;
    }
}

