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

#include "json-priv.hpp"

namespace ncbi
{
    // make an empty object
    JSONObject * JSONObject :: make ()
    {
        return 0;
    }

    // JSONValue interface implementations
    std :: string JSONObject :: toJSON () const
    {
        return 0;
    }
    
    JSONValue * JSONObject :: clone ()
    {
        return 0;
    }

    // asks whether object is empty
    bool JSONObject :: isEmpty () const
    {
        return false;
    }

    // does a member exist
    bool JSONObject :: exists ( const std :: string & name ) const
    {
        return false;
    }

    // return the number of members
    unsigned long int JSONObject :: count () const
    {
        return 0;
    }
        
    // return names/keys
    std :: vector < std :: string > JSONObject :: getNames () const
    {
        throw 123;
    }
        
    // set entry to a new value
    // throws exception if entry exists and is final
    void JSONObject :: setValue ( const std :: string & name, JSONValue * val )
    {
    }

    // set entry to a final value
    // throws exception if entry exists and is final
    void JSONObject :: setFinalValue ( const std :: string & name, JSONValue * val )
    {
    }

    // get named value
    JSONValue & JSONObject :: getValue ( const std :: string & name )
    {
        throw 123;
    }
    
    const JSONValue & JSONObject :: getValue ( const std :: string & name ) const
    {
        throw 123;
    }
        
    // remove a named value
    // returns nullptr if not found
    JSONValue * JSONObject :: removeValue ( const std :: string & name )
    {
        return 0;
    }

    // C++ assignment
    JSONObject & JSONObject :: operator = ( const JSONObject & obj )
    {
        return * this;
    }
    
    JSONObject :: JSONObject ( const JSONObject & obj )
    {
    }

    JSONObject :: ~ JSONObject ()
    {
    }

    JSONObject :: JSONObject ()
    {
    }

}
