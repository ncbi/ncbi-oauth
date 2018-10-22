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

#include <assert.h>
#include <ctype.h>
#include <codecvt>
#include <locale>


namespace ncbi
{
    /* static functions
     **********************************************************************************/
    
    // skip whitespace
    // return the position of the first not whitespace character or npos
    static
    bool skip_whitespace ( const std :: string & text, size_t & pos )
    {
        size_t count = text . size ();
        
        while ( pos < count )
        {
            if ( ! isspace ( text [ pos ] ) )
                break;
            
            ++ pos;
        }
       
        if ( pos >= count )
        {
            pos = std::string::npos;
            return false;
        }
        
        return true;
    }
    
    static
    std :: string hex_to_utf8 ( const std :: string &text )
    {
        size_t index;
        
        try
        {
            unsigned int val = stoi ( text, &index, 16 );
            if ( index != 4 )
                throw JSONException ( __func__, __LINE__, "Invalid \\u escape sequence" ); // test hit
            
            std :: wstring_convert < std :: codecvt_utf8 < char32_t >, char32_t > conv;
            std :: string utf8 = conv . to_bytes ( val );
            
            return utf8;
        }
        catch ( ... )
        {
            throw JSONException ( __func__, __LINE__, "Invalid \\u escape sequence" ); // test hit
        }
    }
    
    static
    void test_depth ( const JSONValue :: Limits & lim, unsigned int & depth )
    {
        if ( ++ depth > lim . recursion_depth )
            throw JSONException ( __func__, __LINE__, "parsing recursion exceeds maximum depth" );
    }
    
    /* JSONValue :: Limits
     **********************************************************************************/
    JSONValue :: Limits :: Limits ()
    : json_string_size ( 4 * 1024 * 1024 )
    , recursion_depth ( 32 )
    , numeral_length ( 256 )
    , string_size ( 64 * 1024 )
    , array_elem_count ( 4 * 1024 )
    , object_mbr_count ( 256 )
    {
    }
    
    JSONValue :: Limits JSONValue :: default_limits;
    
    /* JSONWrapper
     **********************************************************************************/
    JSONValue * JSONWrapper :: parse ( const std::string & json, size_t & pos )
    {
        assert ( json [ pos ] == 'n' );
        
        if ( json . compare ( pos, sizeof "null" - 1, "null" ) == 0 )
            pos += sizeof "null" - 1;
        else
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'null'") ; // test hit
        
        if ( pos < json . size () && isalnum ( json [ pos ] ) )
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'null'" ); // test hit
        
        return JSONValue :: makeNull ();
    }
    
    /* JSONBoolean
     **********************************************************************************/
    JSONValue * JSONBoolean :: parse ( const std::string &json, size_t & pos )
    {
        assert ( json [ pos ] == 'f' || json [ pos ] == 't' );
        
        bool tf;
        size_t start = pos;
        
        if ( json . compare ( start, sizeof "false" - 1, "false" ) == 0 )
        {
            tf = false;
            pos += sizeof "false" - 1;
        }
        else if ( json . compare ( start, sizeof "true" - 1, "true" ) == 0 )
        {
            tf = true;
            pos += sizeof "true" - 1;
        }
        else if ( json [ start ] == 'f' )
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'false'" ); // test hit
        else
            throw JSONException ( __func__, __LINE__, "Expected keyword: 'true'" ); // test hit
        
        // if there was any extra characters following identification of a valid bool token
        if ( pos < json . size () && isalnum ( json [ pos ] ) )
        {
            if ( json [ start ] == 'f' )
                throw JSONException ( __func__, __LINE__, "Expected keyword: 'false'" ); // test hit
            else
                throw JSONException ( __func__, __LINE__, "Expected keyword: 'true'" ); // test hit
        }
        

        JSONValue *val = JSONValue :: makeBool ( tf );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );
        
        return val;
    }
   
    /* JSONNumber
     **********************************************************************************/
    JSONValue * JSONNumber :: parse  ( const JSONValue :: Limits & lim, const std::string &json, size_t & pos )
    {
        assert ( isdigit ( json [ pos ] ) || json [ pos ] == '-' );
        
        size_t start = pos;
        
        if ( json [ pos ] == '-' )
            ++ pos;
        
        if ( ! isdigit ( json [ pos ] ) )
            throw JSONException ( __func__, __LINE__, "Expected: digit" ); // test hit
        
        // check for 0
        if ( json [ pos ] == '0' )
            ++ pos;
        else
        {
            // just find the end of the number
            while ( isdigit ( json [ ++ pos ] ) )
                ;
        }
        
        bool is_float = false;
        switch ( json [ pos ] )
        {
            case '.':
            {
                // skip digits in search of float indicator
                while ( isdigit ( json [ ++ pos ] ) )
                    is_float = true;
                
                // must have at least one digit
                if ( ! is_float )
                    break; // we have an integer
                
                // if a character other than was [eE] found, break
                if ( toupper ( json [ pos ] ) != 'E' )
                    break;
                
                // no break - we have an [eE], fall through
            }
            case 'E':
            case 'e':
            {
                switch ( json [ ++ pos ] )
                {
                    case '+':
                    case '-':
                        ++ pos;
                        break;
                }
                
                while ( isdigit ( json [ pos ] ) )
                {
                    is_float = true;
                    ++ pos;
                }
                
                break;
            }
        }
        
        // "pos" could potentially be a little beyond the end of
        // a legitimate number - let the conversion routines tell us
        std :: string num_str = json . substr ( start, pos - start );
        
        size_t num_len = 0;
        if ( ! is_float )
        {
            try
            {
                long long int num = std :: stoll ( num_str, &num_len );
                pos = start + num_len;
                
                return JSONValue :: makeInteger ( num );
            }
            catch ( std :: out_of_range &e )
            {
                // fall out
            }
        }
        
        // must be floating point
        std :: stold ( num_str, &num_len );
        pos = start + num_len;
        
        if ( num_len > lim . numeral_length )
            throw JSONException ( __func__, __LINE__, "numeral size exceeds allowed limit" );
        
        JSONValue *val = JSONValue :: makeNumber ( num_str . substr ( 0, num_len ) );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );
        
        return val;
    }
    
    /* JSONString
     **********************************************************************************/
    JSONValue * JSONString :: parse  ( const JSONValue :: Limits & lim, const std::string &json, size_t & pos )
    {
        assert ( json [ pos ] == '"' );
        
        std :: string str;
        
        // Find ending '"' or control characters
        size_t esc = json . find_first_of ( "\\\"", ++ pos );
        if ( esc == std :: string :: npos )
            throw JSONException ( __func__, __LINE__, "Invalid begin of string format" ); // test hit
        
        while ( 1 )
        {
            // add everything before the escape in
            // to the new string
            if ( str . size () + ( esc - pos ) > lim . string_size )
                throw JSONException ( __func__, __LINE__, "string size exceeds allowed limit" );
            
            str += json . substr ( pos, esc - pos );
            pos = esc;
            
            // found end of string
            if ( json [ pos ] != '\\' )
                break;
            
            // found '\'
            switch ( json [ ++ pos ] )
            {
                case '"':
                    str += '"';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '/':
                    str += '/';
                    break;
                case 'b':
                    str += '\b';
                    break;
                case 'f':
                    str += '\f';
                    break;
                case 'n':
                    str += '\n';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case 't':
                    str += '\t';
                    break;
                case 'u':
                {
                    // start at the element after 'u'
#pragma warning "still need to deal with this properly"
                    std :: string unicode = json . substr ( pos + 1, 4 );
                    std :: string utf8 = hex_to_utf8 ( unicode );
                    
                    str += utf8;
                    pos += 4;
                    
                    break;
                }
                    
                default:
                    throw JSONException ( __func__, __LINE__, "Invalid escape character" ); // test hit
            }
            
            // skip escaped character
            ++ pos;
            
            // Find ending '"' or control characters
            esc = json . find_first_of ( "\\\"", pos );
            if ( esc == std :: string :: npos )
                throw JSONException ( __func__, __LINE__, "Invalid end of string format" ); // test hit
        }
        
        assert ( esc == pos );
        assert ( json [ pos ] == '"' );
        
        // set pos to point to next token
        ++ pos;
        
        if ( str . size () > lim . string_size )
            throw JSONException ( __func__, __LINE__, "string size exceeds allowed limit" );
        
        JSONValue *val = JSONValue :: makeString ( str );
        if ( val == nullptr )
            throw JSONException ( __func__, __LINE__, "Failed to make JSONValue" );
        
        return val;
    }

    /* JSONValue
     **********************************************************************************/
    JSONValue * JSONValue :: test_parse ( const std :: string & json, bool consume_all )
    {
        size_t pos = 0;
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );
        
        if ( json . size () > default_limits . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );
        
        JSONValue *val = parse ( default_limits, json, pos, 0 );
        
        if ( consume_all && pos < json . size () )
        {
            delete val;
            throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" ); // test hit
        }
        
        return val;
    }
    
    JSONValue * JSONValue :: parse ( const Limits & lim, const std :: string & json, size_t & pos, unsigned int depth )
    {
        if ( skip_whitespace ( json, pos ) )
        {
            switch ( json [ pos ] )
            {
                case '{':
                    return JSONObject :: parse ( lim, json, pos, depth );
                case '[':
                    return JSONArray :: parse ( lim, json, pos, depth );
                case '"':
                    return JSONString :: parse ( lim, json, pos );
                case 'f':
                case 't':
                    return JSONBoolean :: parse ( json, pos );
                case '-':
                    return JSONNumber :: parse ( lim, json, pos );
                case 'n':
                    return JSONWrapper :: parse ( json, pos );
                default:
                    if ( isdigit ( json [ pos ] ) )
                        return JSONNumber :: parse ( lim, json, pos );
                    
                    // garbage
                    throw JSONException ( __func__, __LINE__, "Invalid JSON format" ); // test hit
            }
        }

        return nullptr;
    }
   
    /* JSONArray
     **********************************************************************************/
    JSONArray * JSONArray :: parse ( const Limits & lim, const std :: string & json, size_t & pos, unsigned int depth )
    {
        assert ( json [ pos ] == '[' );
        
        JSONArray *array = new JSONArray ();
        try
        {
            while ( 1 )
            {
                // skip over '[' and any whitespace
                // json [ 0 ] is '[' or ','
                if ( ! skip_whitespace ( json, ++ pos ) )
                    throw JSONException ( __func__, __LINE__, "Expected: ']'" ); // test hit
                
                if ( json [ pos ] == ']' )
                    break;
             
                // use scope to invalidate value 
                {
                    JSONValue *value = JSONValue :: parse ( lim, json, pos, depth );
                    if ( value == nullptr )
                        throw JSONException ( __func__, __LINE__, "Failed to create JSON object" );

                    array -> appendValue ( value );
                    
                    if ( array -> count () > default_limits . array_elem_count )
                        throw JSONException ( __func__, __LINE__, "Array element count exceeds limit" );
                }
                
                // find and skip over ',' and skip any whitespace
                // exit loop if no ',' found
                if ( ! skip_whitespace ( json, pos ) || json [ pos ] != ',' )
                    break;
            }
            
            // must end on ']'
            if ( pos == std :: string :: npos || json [ pos ] != ']' )
                throw JSONException ( __func__, __LINE__, "Excpected: ']'" ); // Test hit
            
            // skip over ']'
            ++ pos;
        }
        catch ( ... )
        {
            delete array;
            throw;
        }
        
        // JSONArray must be valid
        assert ( array != nullptr );
        return array;
    }
    
    /* JSONObject
     **********************************************************************************/
    
    // make an object from JSON source
    JSONObject * JSONObject :: parse ( const std :: string & json )
    {
        return parse ( default_limits, json );
    }
    
    JSONObject * JSONObject :: parse ( const JSONValue :: Limits & lim, const std :: string & json )
    {
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );
        
        if ( json . size () > lim . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );
        
        size_t pos = 0;
        
        if ( ! skip_whitespace ( json, pos ) || json [ pos ] != '{' )
            throw JSONException ( __func__, __LINE__, "Expected: '{'" );

        JSONObject *obj = parse ( lim, json, pos, 0 );
        
        if ( pos < json . size () )
            throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" );
        
        return obj;
    }
    
    JSONObject * JSONObject :: parse ( const Limits & lim, const std :: string & json, size_t & pos, unsigned int depth )
    {
        test_depth ( lim, depth );

        assert ( json [ pos ] == '{' );
        
        JSONObject *obj = new JSONObject ();
        try
        {
            while ( 1 )
            {
                // skip over '{' and any whitespace
                // json [ pos ] is '{' or ',', start at json [ pos + 1 ]
                if ( ! skip_whitespace ( json, ++ pos ) )
                    throw JSONException ( __func__, __LINE__, "Expected: '}'" ); // test hit

                if ( json [ pos ] == '}' )
                    break;
                
                if ( json [ pos ] != '"' )
                    throw JSONException ( __func__, __LINE__, "Expected: 'name' " );
                
                JSONValue *name = JSONString :: parse ( lim, json, pos );
                if ( name == nullptr )
                    throw JSONException ( __func__, __LINE__, "Failed to create JSON object" );
                
                // skip to ':'
                if ( ! skip_whitespace ( json, pos ) || json [ pos ] != ':' )
                {
                    delete name;
                    throw JSONException ( __func__, __LINE__, "Expected: ':'" ); // test hit
                }
                
                // skip over ':'
                ++ pos;
                
                // get JSON value;
                JSONValue *value = JSONValue :: parse ( lim, json, pos, depth );
                if ( value == nullptr )
                {
                    delete name;
                    throw JSONException ( __func__, __LINE__, "Failed to create JSON object" );
                }
                
                obj -> setValue ( name -> toString(), value );
                
                if ( obj -> count () > default_limits . object_mbr_count )
                {
                    delete name;
                    throw JSONException ( __func__, __LINE__, "Array element count exceeds limit" );
                }
                
                delete name;
                
                // find and skip over ',' and skip any whitespace
                // exit loop if no ',' found
                if ( ! skip_whitespace ( json, pos ) || json [ pos ] != ',' )
                    break;
            }
            
            // must end on '}'
            if ( pos == std :: string :: npos || json [ pos ] != '}' )
                throw JSONException ( __func__, __LINE__, "Expected: '}'" ); // test hit
            
            // skip over '}'
            ++ pos;
        }
        catch ( ... )
        {
            delete obj;
            throw;
        }
        
        
        // JSONObject must be valid
        assert ( obj != nullptr );
        return obj;
    }
    
    
    /* JSON
     **********************************************************************************/
    
    // make an object from JSON source
    JSONValue * JSON :: parse ( const std :: string & json )
    {
        return parse ( JSONValue :: default_limits, json );
    }
    
    JSONValue * JSON :: parse ( const JSONValue :: Limits & lim, const std :: string & json )
    {
        if ( json . empty () )
            throw JSONException ( __func__, __LINE__, "Empty JSON source" );
        
        if ( json . size () > lim . json_string_size )
            throw JSONException ( __func__, __LINE__, "JSON source exceeds allowed size limit" );
        
        size_t pos = 0;
        
        if ( ! skip_whitespace ( json, pos ) )
            throw JSONException ( __func__, __LINE__, "Expected: '{' or '['" );

        JSONValue *val = nullptr;
        switch ( json [ pos ] )
        {
        case '{':
            val = JSONObject :: parse ( lim, json, pos, 0 );
            break;
        case '[':
            val = JSONArray :: parse ( lim, json, pos, 0 );
            break;
        default:
            throw JSONException ( __func__, __LINE__, "Expected: '{' or '['" );
        }
        
        if ( pos < json . size () )
            throw JSONException ( __func__, __LINE__, "Trailing byes in JSON text" );
        
        return val;
    }
}
