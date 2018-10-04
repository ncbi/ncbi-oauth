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
        return new JSONObject ();
    }

    std :: string JSONObject :: toString () const
    {
        throw JSONException ( __func__, __LINE__, "this value cannot be converted to a string" );
    }
    
    // JSONValue interface implementations
    std :: string JSONObject :: toJSON () const
    {
        std :: string to_string = "{";
        const char* sep = "";
        
        for ( auto it = members . begin (); it != members . end (); ++ it )
        {
            std :: string key =  it -> first;
            
            JSONValue* value = it -> second . second;
            
            to_string += sep;
            to_string += string_to_json ( key ) + ":" + value -> toJSON();
            
            sep = ",";
        }
        
        to_string += "}";
        
        return to_string;
    }
    
    JSONValue * JSONObject :: clone () const
    {
        JSONObject *copy = new JSONObject ();
        
        *copy = *this;
        
        return copy;
    }

    // asks whether object is empty
    bool JSONObject :: isEmpty () const
    {
        return members . empty ();
    }

    // does a member exist
    bool JSONObject :: exists ( const std :: string & name ) const
    {
        auto it = members . find ( name );

        if ( it == members . end () )
                return false;
        
        return true;
    }

    // return the number of members
    unsigned long int JSONObject :: count () const
    {
        return members . size ();
    }
        
    // return names/keys
    std :: vector < std :: string > JSONObject :: getNames () const
    {
        std :: vector < std :: string > names;
        
        for ( auto it = members . cbegin(); it != members . cend (); ++ it )
            names . push_back ( it -> first );
        
        return names;
    }
        
    // set entry to a new value
    // throws exception if entry exists and is final
    void JSONObject :: setValue ( const std :: string & name, JSONValue * val )
    {
        std :: pair < bool, JSONValue * > a_member;
        
        auto it = members . find ( name );
        
        // if doesnt exist, add
        if ( it == members . cend () )
        {
            std :: pair < bool, JSONValue * > pair ( false, val );
            members . emplace ( name, pair );
        }
        else
        {
            // if non modifiable, throw
            if ( it -> second . first )
                throw JSONException ( __func__, __LINE__, "Cannot overrite final member" );
            
            // overwrite value
            // TBD - need to look at threat safety
            delete it -> second . second;
            it -> second . second = val;
        }
    }

    // set entry to a final value
    // throws exception if entry exists and is final
   #pragma warning "this set final value seems to be leaking memory"
    void JSONObject :: setFinalValue ( const std :: string & name, JSONValue * val )
    {
        std :: pair < bool, JSONValue * > a_member;
        
        auto it = members . find ( name );
        
        // if doesnt exist, add
        if ( it == members . cend () )
        {
            std :: pair < bool, JSONValue * > pair ( true, val );
            members . emplace ( name, pair );
        }
        else
        {
            // if non modifiable, throw
            if ( it -> second . first )
                throw JSONException ( __func__, __LINE__, "Cannot overrite final member" );
            
            // overwrite value
            delete it -> second . second;
            it -> second . second = val;
        }
    }

    // get named value
    JSONValue & JSONObject :: getValue ( const std :: string & name )
    {
        auto it = members . find ( name );
        if ( it != members . cend () )
        {
            return * it -> second . second;
        }
        
        throw JSONException ( __func__, __LINE__, "Member not found" );
    }
    
    const JSONValue & JSONObject :: getValue ( const std :: string & name ) const
    {
        auto it = members . find ( name );
        if ( it != members . cend () )
        {
            return * it -> second . second;
        }
        
        throw JSONException ( __func__, __LINE__, "Member not found" );
    }
        
    // remove a named value
    void JSONObject :: removeValue ( const std :: string & name )
    {
        auto it = members . find ( name );
        if ( it != members . cend () && it -> second . first == false )
            members . erase ( it );
    }

    // C++ assignment
    JSONObject & JSONObject :: operator = ( const JSONObject & obj )
    {
        clear ();
        
        for ( auto it = obj . members . cbegin(); it != obj . members . cend (); ++ it )
        {
            std :: string name = it -> first;
            JSONValue *val = it -> second . second  -> clone ();
            
            if ( it -> second . first )
                setFinalValue ( name, val );
            else
                setValue ( name, val );
        }
        
        return * this;
    }
    
    JSONObject :: JSONObject ( const JSONObject & obj )
    {
        for ( auto it = obj . members . cbegin(); it != obj . members . cend (); ++ it )
        {
            std :: string name = it -> first;
            JSONValue *val = it -> second . second  -> clone ();
            
            if ( it -> second . first )
                setFinalValue ( name, val );
            else
                setValue ( name, val );
        }
    }

    JSONObject :: ~ JSONObject ()
    {
        clear ();
    }
    
    // used to empty out the object before copy
    void JSONObject :: clear ()
    {
        if ( ! members . empty () )
        {
            for ( auto it = members . cbegin(); it != members . cend (); )
            {
                // if its final, skip it
                if ( it -> second . first )
                {
                    ++ it;
                    continue;
                }
                
                delete  it -> second . second;
                it = members . erase ( it );
            }
        }
    }

    JSONObject :: JSONObject ()
    {
    }

}
