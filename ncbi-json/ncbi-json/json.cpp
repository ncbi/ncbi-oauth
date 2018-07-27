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
                            size_t len = snprintf( buff, sizeof buff, "\\u%04x",
                                                  ( unsigned int ) ( unsigned char ) ch );

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
    
    bool JSONValue :: toBool () const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             toBool() is unsupported for this value type" ); // test hit
    }
    
    long long int JSONValue :: toInt () const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             toInt() is unsupported for this value type" ); // test hit
    }
    
    long double JSONValue :: toReal () const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             toReal() is unsupported for this value type" ); // test hit
    }
    
    std :: string JSONValue :: toString () const
    {
        return toJSON ();
    }
    
    JSONValue & JSONValue :: setPointerValue ( const char * val )
    {
        if ( val != nullptr )
        {
            return setStringValue ( val );
        }
        
        return setToNull ();
    }
    
    JSONValue & JSONValue :: getValueByIndex ( int idx )
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator [] is unsupported for this value type" );
    }
    
    JSONValue & JSONValue :: getValueByName ( const std :: string &mbr )
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator [] is unsupported for this value type" );
    }
    
    JSONValue & JSONValue :: setBooleanValue ( bool val )
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for Boolean" );
    }
    
    JSONValue & JSONValue :: setIntegerValue ( long long int val )
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for integers" );
    }
    
    JSONValue & JSONValue :: setRealValue ( long double val )
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for real numbers" );
    }
    
    JSONValue & JSONValue :: setStringValue ( const std :: string & val )
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for strings" );
    }
    
    JSONValue & JSONValue :: setToNull ()
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for 'null''" );
    }
    
    const JSONValue & JSONValue :: getConstValueByIndex ( int idx ) const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for this value type" );
    }
    
    const JSONValue & JSONValue :: getConstValueByName ( const std :: string &mbr ) const
    {
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             operator = is unsupported for this value type" );
    }

    /* JSONTmpValue
     **********************************************************************************/
    std :: string JSONTmpValue :: toJSON () const
    {
        return "null";
    }
    
    JSONTmpValue :: JSONTmpValue ( JSONArray *par, int idx )
    : parent ( par )
    , index ( idx )
    {
        if ( idx < 0 || ( size_t ) idx > par -> seq . size () )
        {
            throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                                 Invalid tmp array index" ); // Test hit
        }
    }
    
    JSONTmpValue :: JSONTmpValue ( JSONObject *par, const std :: string & _mbr )
    : parent ( par )
    , mbr ( _mbr )
    , index ( -1 )
    {
        if ( _mbr . empty () )
        {
            throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                                 Invalid temporary object value" ); // Test hit
        }
    }
    
    // replace self with JSONArray
    JSONValue & JSONTmpValue :: getValueByIndex ( int idx )
    {
        JSONValue & val = replaceSelf ( new JSONArray );
        return val . getValueByIndex ( idx );
    }
    
    // replace self with JSONObject
    JSONValue & JSONTmpValue :: getValueByName ( const std :: string & mbr )
    {
        JSONValue & val = replaceSelf ( new JSONObject );
        return val . getValueByName ( mbr );
    }
    
    // replace self with JSONBoolValue
    JSONValue & JSONTmpValue :: setBooleanValue ( bool val )
    {
        return replaceSelf ( new JSONBoolValue ( val ) );
    }
    
    // replace self with JSONIntegerValue
    JSONValue & JSONTmpValue :: setIntegerValue ( long long int val )
    {
        return replaceSelf ( new JSONIntegerValue ( val ) );
    }
    
    JSONValue & JSONTmpValue :: setRealValue ( long double val )
    {
        return replaceSelf ( new JSONRealValue ( val ) );
    }
    
    JSONValue & JSONTmpValue :: setStringValue ( const std :: string & val )
    {
        return replaceSelf ( new JSONStringValue ( val ) );
    }
    
    JSONValue & JSONTmpValue :: setToNull ()
    {
        return replaceSelf ( new JSONNullValue );
    }
    
    JSONValue & JSONTmpValue :: replaceSelf ( JSONValue * val )
    {
        if ( index >= 0 )
        {
            JSONArray * dad = ( JSONArray * ) parent;
            dad -> seq [ index ] = val;
        }
        else
        {
            JSONObject * dad = ( JSONObject * ) parent;
            dad -> addMember ( mbr, val );
        }
        
        delete this;
        return * val;
    }

    /* JSONNullValue
     **********************************************************************************/
    std :: string JSONNullValue :: toJSON () const
    {
        return "null";
    }
    
    JSONValue & JSONNullValue :: setToNull ()
    {
        return * this;
    }
    
    /* JSONBoolValue
     **********************************************************************************/
    std :: string JSONBoolValue :: toJSON () const
    {
        return std :: string ( value ? "true" : "false" );
    }
    
    bool JSONBoolValue :: toBool () const
    {
        return value;
    }
    
    JSONValue & JSONBoolValue :: setBooleanValue ( bool val )
    {
        value = val;
        return * this;
    }
    
    JSONBoolValue :: JSONBoolValue ( bool val )
    : value ( val )
    {}
    
    JSONBoolValue :: JSONBoolValue ( const JSONBoolValue & copy )
    : value ( copy . value )
    {}
    
    JSONBoolValue & JSONBoolValue :: operator = ( const JSONBoolValue & orig )
    {
        value = orig . value;
        return *this;
    }
    
    /* JSONIntegerValue
     **********************************************************************************/
    std :: string JSONIntegerValue :: toJSON () const
    {
        return std :: to_string ( value );
    }
    
    JSONValue & JSONIntegerValue :: setIntegerValue ( long long int val )
    {
        value = val;
        return * this;
    }
    
    long long JSONIntegerValue :: toInt () const
    {
        return value;
    }
    
    long double JSONIntegerValue :: toReal () const
    {
        return ( long double ) value;
    }
    
    JSONIntegerValue :: JSONIntegerValue ( long long int val )
    : value ( val )
    {}
    
    JSONIntegerValue :: JSONIntegerValue ( const JSONIntegerValue & copy )
    : value ( copy . value )
    {}
    
    JSONIntegerValue & JSONIntegerValue :: operator = ( const JSONIntegerValue & orig )
    {
        value = orig . value;
        return *this;
    }
    
    /* JSONReadValue
     **********************************************************************************/
    std :: string JSONRealValue :: toJSON () const
    {
        return value;
    }
    
    JSONValue & JSONRealValue :: setRealValue ( long double val )
    {
        // TBD - determine best setting with precision
        value = std :: to_string ( val );
        return * this;
    }
    
    long long int JSONRealValue :: toInt () const
    {
        return std :: stoll ( value );
    }
    
    long double JSONRealValue :: toReal () const
    {
        return std :: stold ( value );
    }
    
    JSONRealValue :: JSONRealValue ( long double val )
    {
        char buffer [ 1024 ];
        auto size = snprintf ( buffer, sizeof buffer, "%.20Lg", val );
        value = std :: string ( buffer, size );
    }
    
    JSONRealValue :: JSONRealValue ( const std :: string & val )
    : value ( val )
    {}
    
    JSONRealValue :: JSONRealValue ( const JSONRealValue & copy )
    : value ( copy . value )
    {}
    
    JSONRealValue & JSONRealValue :: operator = ( const JSONRealValue & orig )
    {
        value = orig . value;
        return *this;
    }
    
    /* JSONStringValue
     **********************************************************************************/
    std :: string JSONStringValue :: toJSON () const
    {
        return string_to_json ( value );
    }
    
    std :: string JSONStringValue :: toString () const
    {
        return value;
    }
    
    bool JSONStringValue :: toBool () const
    {
        if ( value . compare ( "true" ) == 0 )
            return true;
        else if ( value . compare ( "false" ) == 0 )
            return false;
        
        throw JSONException ( __FILE__, __LINE__, "Not a boolean value" );
    }
    
    long long JSONStringValue :: toInt () const
    {
        size_t num_len;

        long long int int_val = std :: stoll ( value, &num_len );
        if ( num_len == value . size () )
            return int_val;
        else if ( num_len > 0 )
        {
            num_len = 0;
            std :: stold ( value, &num_len );
            if ( num_len == value . size () )
                return int_val;
        }
        
        throw JSONException ( __FILE__, __LINE__, "Not an integer value" );
    }
    
    long double JSONStringValue :: toReal () const
    {
        size_t num_len;
        
        long double val = std :: stold ( value, &num_len );
        if ( num_len == value . size () )
            return val;
        
        throw JSONException ( __FILE__, __LINE__, "Not a floating point value" );
    }
    
    JSONValue & JSONStringValue :: setStringValue ( const std :: string & val )
    {
        value = val;
        return * this;
    }
    
    JSONStringValue :: JSONStringValue ( const std :: string & val )
    : value ( val )
    {}
    
    JSONStringValue :: JSONStringValue ( const JSONStringValue & copy )
    : value ( copy . value )
    {}
    
    JSONStringValue & JSONStringValue :: operator = ( const JSONStringValue & orig )
    {
        value = orig . value;
        return *this;
    }
    
    /* JSONArray
     *
     **********************************************************************************/
    JSONValue & JSONArray :: getValueByIndex ( int idx )
    {
        if ( idx >= 0 && idx <= seq . size () )
        {
            if ( idx == seq . size () )
                seq . push_back ( new JSONTmpValue ( this, idx ) ); 
            
            return * seq [ idx ];
        }
        
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             Array index out of bounds" ); // Test hit
    }
    
    const JSONValue & JSONArray :: getConstValueByIndex ( int idx ) const
    {
        if ( idx > 0 && idx < seq . size () )
            return * seq [ idx ];
        
        throw JSONException ( __FILE__, __LINE__, "INTERNAL ERROR: \
                             Array index out of bounds" );
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
    JSONValue & JSONObject :: getValueByName ( const std :: string & name )
    {
        JSONValue *jval = nullptr;
        std :: pair < bool, JSONValue * > a_member;
        
        auto it = members . find ( name );
        
        // if doesnt exist, add
        if ( it == members . end () )
        {
            jval = new JSONTmpValue ( this, name );
            std :: pair < bool, JSONValue * > pair ( false, jval );
            members . emplace ( name, pair );
        }
        else
        {
            // retrieve existing value
            jval = it -> second . second;
        }
        
        return * jval;
    }
    
    const JSONValue & JSONObject :: getConstValueByName ( const std :: string & name ) const
    {
        auto it = members . find ( name );
        JSONValue *jval = it -> second . second;
        return *jval;
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
                throw JSONException ( __FILE__, __LINE__, "Cannot overrite final member" );

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
                throw JSONException ( __FILE__, __LINE__, "Cannot overrite final member" );
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


