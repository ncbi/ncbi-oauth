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

#include <stdio.h>

namespace ncbi
{
    
    /* static functins
     **********************************************************************************/
    static
    std :: string string_to_json ( const std :: string & string )
    {
        std :: string quoted = "\"";
        size_t i, pos, count = string . size ();
        
        for ( i = pos = 0; i < count ; ++ i )
        {
            char ch = string [ i ];
            
            if ( isascii ( ch ) )
            {
                const char * esc = nullptr;
                switch ( ch )
                {
                    case '"':
                        esc = "\\\"";
                        break;
                    case '\\':
                        esc = "\\\\";
                        break;
                    case '/':
                        esc = "\\/";
                        break;
                    case '\b':
                        esc = "\\b";
                        break;
                    case '\f':
                        esc = "\\f";
                        break;
                    case '\n':
                        esc = "\\n";
                        break;
                    case '\r':
                        esc = "\\r";
                        break;
                    case '\t':
                        esc = "\\t";
                        break;
                    default:
                    {
                        if ( ! isprint ( ch ) )
                        {
                            // unicode escape hex sequence
                            char buff [ 32 ];
                            size_t len = snprintf( buff, sizeof buff, "\\u%04x", ( unsigned int ) ( unsigned char ) ch );

                            assert ( len == 6 );
                            
                            quoted += string . substr ( pos, i - pos );
                            pos = i + 1;
                        }
                    }
                }
                if ( esc != nullptr )
                {
                    quoted += string . substr ( pos, i - pos );
                    quoted += esc;
                    
                    pos = i + 1;
                }
            }
        }
        
        if ( pos < count )
            quoted += string . substr ( pos, count - pos );
       
        quoted +=  "\"";
        
        return quoted;
    }
    
    /* JSONValue
     **********************************************************************************/
    JSONValue & JSONValue :: operator [] ( int idx )
    {
        throw JSONException ( __FILE__, __LINE__,
                             "INTERNAL ERROR: operator [] is unsupported for this value type" );
    }
    
    JSONValue & JSONValue :: operator [] ( const std :: string &mbr )
    {
        throw JSONException ( __FILE__, __LINE__,
                             "INTERNAL ERROR: operator [] is unsupported for this value type" );
    }
    
    JSONValue & JSONValue :: operator = ( bool val )
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    JSONValue & JSONValue :: operator = ( long long int val )
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    JSONValue & JSONValue :: operator = ( long double val )
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    JSONValue & JSONValue :: operator = ( const std :: string & val )
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    JSONValue & JSONValue :: operator = ( const void *val )
    {
        JSONValue *jval = nullptr; 
        return *jval;
    }
    
    const JSONValue & JSONValue :: operator [] ( int idx ) const
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    const JSONValue & JSONValue :: operator [] ( const std :: string &mbr ) const
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    bool JSONValue :: toBool () const
    {
        return false;
    }
    
    long long int JSONValue :: toInt () const
    {
        return 0;
    }
    
    long double JSONValue :: toReal () const
    {
        return 0.0;
    }
    
    std :: string JSONValue :: toString () const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: toString() is unsupported for this value type" );
    }

    /* JSONTmpValue
     **********************************************************************************/
    std :: string JSONTmpValue :: toJSON () const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: toJSON() is unsupported for this value type" );
    }
    
    JSONTmpValue :: JSONTmpValue ( JSONValue *parent, int idx )
    : parent ( parent )
    , index ( idx )
    {
    }

    /* JSONNullValue
     **********************************************************************************/

    
    /* JSONTypedValue
     **********************************************************************************/
        
    template <>
    std :: string JSONTypedValue < bool > :: toJSON () const
    {
        return std :: string ( val ? "true" : "false" );
    }
    
    template <>
    std :: string JSONTypedValue < long long int > :: toJSON () const
    {
        char buffer [ 1024 ];
        auto size = snprintf ( buffer, sizeof buffer, "%lld", val );
        return std :: string ( buffer, size );
    }
    
    template <>
    std :: string JSONTypedValue < long double > :: toJSON () const
    {
        char buffer [ 1024 ];
        auto size = snprintf ( buffer, sizeof buffer, "%.20Lg", val );
        return std :: string ( buffer, size );
    }
    
    template <>
    std :: string JSONTypedValue < std :: string > :: toJSON () const
    {
        return string_to_json ( val );
    }
    
    template <>
    std :: string JSONTypedValue < std :: string > :: toString () const
    {
        return val;
    }
    
    /* JSONArray
     *
     **********************************************************************************/
    JSONValue & JSONArray :: operator [] ( int idx )
    {
        if ( idx >= 0 && idx < seq . size () )
        {
            if ( idx == seq . size () )
                seq [ idx ] = new JSONTmpValue ( this, idx );
            
            return * seq [ idx ];
        }
        
        throw JSONException ( __FILE__, __LINE__,
                             "INTERNAL ERROR: TBD" );
    }
    
    const JSONValue & JSONArray :: operator [] ( int idx ) const
    {
        if ( idx > 0 && idx < seq . size () )
            return * seq [ idx ];
        
        throw JSONException ( __FILE__, __LINE__,
                             "INTERNAL ERROR: TBD" );
    }
    
    JSONValue & JSONArray :: operator [] ( const std :: string & mbr )
    {
        return JSONValue :: operator [] ( mbr );
    }
    
    const JSONValue & JSONArray :: operator [] ( const std :: string & mbr ) const
    {
        return JSONValue :: operator [] ( mbr );
    }
    
    std :: string JSONArray :: toJSON () const
    {
        std :: string to_string = "[";
        
        const char* sep = "";
        auto size = seq . size ();
        
        for ( size_t i = 0; i < size; ++ i )
        {
            const JSONValue* value = seq [ i ];
            
            to_string += sep;
            to_string += value -> toJSON();
            
            sep = ",";
        }
        
        to_string += "]";
        
        return to_string;
    }
    
    void JSONArray :: append ( JSONValue * elem )
    {
        seq . push_back ( elem );
    }
    
    JSONArray & JSONArray :: operator = ( const JSONArray & array )
    {
        clear ();
        
        auto size = array . seq . size ();
        
        for ( size_t i = 0; i < size; ++ i )
        {
            seq [ i ] = array . seq [ i ];
        }
        
        return *this;
    }
    
    void JSONArray :: clear ()
    {
        while ( ! seq . empty() )
        {
            auto elem = seq . back();
            seq . pop_back();
            delete elem;
        }
    }
    
    JSONArray :: ~JSONArray()
    {
        try
        {
            clear ();
        }
        catch ( ... )
        {
        }
    }
    

    
    /* JSONObject
     **********************************************************************************/
    JSONValue & JSONObject :: operator [] ( const std :: string & mbr )
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    const JSONValue & JSONObject :: operator [] ( const std :: string & mbr ) const
    {
        JSONValue *jval = nullptr;
        return *jval;
    }
    
    JSONValue & JSONObject :: operator [] ( int idx )
    {
        return JSONValue :: operator [] ( idx );
    }
    
    const JSONValue & JSONObject :: operator [] ( int idx ) const
    {
        return JSONValue :: operator [] ( idx );
    }
    
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
    
    JSONValue * JSONObject :: getValue ( const std :: string & name )
    {
        auto it = members . find ( name );
        
        JSONValue* value = it -> second . second;
        
        return value;
    }
    
    const JSONValue * JSONObject :: getValue ( const std :: string & name ) const
    {
        auto it = members . find ( name );
        
        const JSONValue* value = it -> second . second;
        
        return value;
    }
    
    void JSONObject :: addMember ( const std :: string & name, JSONValue * value, bool is_final )
    {
        std :: pair < bool, JSONValue * > a_member;
        
        auto it = members . find ( name );

        // if doesnt exist, add
        if ( it == members . end () )
        {
            std :: pair < bool, JSONValue * > pair ( is_final, value );
            members . emplace ( name, pair );
        }
        else
        {
            // if non modifiable, throw
            if ( it -> second . first )
                throw "Cannot overrite final member";

            // overwrite value
            it -> second . second = value;
        }
    }
    
    bool JSONObject :: removeMember ( const std :: string & name )
    {
        auto it = members . find ( name );
        
        if ( it != members . end () )
        {
            if ( it -> second . first )
                throw "This member is final";
            else
            {
                members . erase ( it );
                return true;
            }
        }
        
        return false;
    }
    
    std :: vector < std :: string > JSONObject :: getNames () const
    {
        std :: vector < std :: string > names;
        
        for ( auto it = members . cbegin(); it != members . cend (); ++ it )
            names . push_back ( it -> first );
        
        return names;
    }

}


