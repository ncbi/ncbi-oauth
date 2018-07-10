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
    void skip_whitespace ( const std :: string & text, size_t & pos )
    {
        size_t count = text . size ();
        
        while ( pos < count )
        {
            if ( ! isspace ( text [ pos ] ) )
                break;
            
            ++ pos;
        }
       
        if ( pos >= count )
            pos = std::string::npos;
    }
    
    static
    std :: string hex_to_utf8 ( const std :: string &text )
    {
        size_t index;
        unsigned int val = stoi ( text, &index, 16 );
        if ( index != 3 )
            throw "Malformed \\u escape sequence";

        std :: wstring_convert < std :: codecvt_utf8 < char32_t >, char32_t > conv;
        std :: string utf8 = conv . to_bytes ( val );

        return utf8;
    }
    
    /* JSONNullValue
     **********************************************************************************/
    JSONValue * JSONNullValue :: parse ( const std::string & json, size_t & pos )
    {
        assert ( json [ pos ] == 'n' );
        
        if ( json . compare ( pos, sizeof "null" - 1, "null" ) == 0 )
            pos += sizeof "null" - 1;
        else
            throw "expected keyword null";
        
        if ( pos < json . size () && isalnum ( json [ pos ] ) )
            throw "expected keyword null";
        
        return new JSONNullValue ();
    }
    
    /* JSONTypedValue
     **********************************************************************************/
    template<>
    JSONValue * JSONTypedValue < std :: string >
    :: parse ( const std::string &json, size_t & pos )
    {
        assert ( json [ pos ] == '"' );

        std :: string str;
        
        size_t esc = json . find_first_of ( "\\\"", ++ pos );
        if ( esc == std :: string :: npos )
            throw "improper string format";
        
        while ( 1 )
        {
            // add everything before the escape in
            // to the new string
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
                    std :: string utf8 = hex_to_utf8 ( json . substr ( pos + 1, 4 ) );
                    
                    str += utf8;
                    pos += 4;
                    
                    break;
                }
                    
                default:
                    throw "Invalid escape character";
            }
            
            // skip escaped character
            ++ pos;
            
            esc = json . find_first_of ( "\\\"", pos );
            if ( esc == std :: string :: npos )
                throw "sdfsdfsdf";
        }
        
        assert ( esc == pos );
        assert ( json [ pos ] == '"' );
        
        // set pos to point to next token
        ++ pos;
    
        return new JSONTypedValue < std :: string > ( str );
    }
    
    template<>
    JSONValue * JSONTypedValue < bool >
    :: parse ( const std::string &json, size_t & pos )
    {
        assert ( json [ pos ] == 'f' || json [ pos ] == 't' );
        
        bool val;
        
        if ( json . compare ( pos, sizeof "false" - 1, "false" ) == 0 )
        {
            val = false;
            pos += sizeof "false" - 1;
        }
        else if ( json . compare ( pos, sizeof "true" - 1, "true" ) == 0 )
        {
            val = true;
            pos += sizeof "true" - 1;
        }
        else if ( json [ pos ] == 'f' )
            throw "expected keyword false";
        else
            throw "expected keyword true";
        
        if ( pos < json . size () && isalnum ( json [ pos ] ) )
        {
            if ( json [ pos ] == 'f' )
                throw "expected keyword false";
            else
                throw "expected keyword true";
        }
        
        return new JSONTypedValue < bool > ( val );
    }
    
    // This will not be called, taken care of in
    // JSONValue * JSONTypedValue <long long int> :: parse
    template<>
    JSONValue * JSONTypedValue < long double >
    :: parse ( const std::string &json, size_t & pos )
    {
        return nullptr;
    }
    
    template<>
    JSONValue * JSONTypedValue < long long int >
    :: parse ( const std::string &json, size_t & pos )
    {
        assert ( isdigit ( json [ pos ] ) || json [ pos ] == '-' );
        
        size_t start = pos;
        
        if ( json [ pos ] == '-' )
            ++ pos;
        
        if ( ! isdigit ( json [ pos ] ) )
            throw "not a number";
        
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
                
                return new JSONTypedValue < long long int > ( num );
            }
            catch ( std :: out_of_range &e )
            {
                // fall out
            }
        }
        
        // must be floating point
        long double num = std :: stold ( num_str, &num_len );
        pos = start + num_len;

        return new JSONTypedValue < long double > ( num );
    }
    

    /* JSONValue
     **********************************************************************************/
    JSONValue * JSONValue :: parse ( const std :: string & json, size_t & pos )
    {
        skip_whitespace ( json, pos );
        if ( pos != std :: string :: npos )
        {
            switch ( json [ pos ] )
            {
                case '{':
                    return JSONObject :: parse ( json, pos );
                case '[':
                    return JSONArray :: parse ( json, pos );
                case '"':
                    return JSONTypedValue < std :: string > :: parse ( json, pos );
                case 'f':
                case 't':
                    return JSONTypedValue < bool > :: parse ( json, pos );
                case '-':
                    return JSONTypedValue < long long int > :: parse ( json, pos );
                case 'n':
                    return JSONNullValue :: parse ( json, pos );
                    break;
                default:
                    if ( isdigit ( json [ pos ] ) )
                        return JSONTypedValue < long long int > :: parse ( json, pos );
                    
                    // garbage
                    throw "garbage json text";
            }
        }

        return nullptr;
    }
   
    /* JSONArray
     **********************************************************************************/
    JSONArray * JSONArray :: parse ( const std :: string & json )
    {
        if ( json . empty () )
            throw "empty JSON array";
        
        size_t pos = 0;
        skip_whitespace( json, pos );
        if ( json [ pos ] != '[' )
            throw "expecting '['";
        
        return parse ( json, pos );
    }
    
    JSONArray * JSONArray :: parse ( const std :: string & json, size_t & pos )
    {
        assert ( json [ pos ] == '[' );
        
        JSONArray *array = new JSONArray;
        try
        {
            while ( 1 )
            {
                // skip over '[' and any whitespace
                // json [ 0 ] is '[' or ','
                skip_whitespace ( json, ++ pos );
                if ( pos == std :: string :: npos )
                    throw "expected ']'";
                
                if ( json [ pos ] == ']' )
                    break;
                
                JSONValue *value = JSONValue :: parse ( json, pos );
                if ( value == nullptr )
                    throw "Invalid value";
                
                array -> append ( value );
                
                // find and skip over ',' and skip any whitespace
                // exit loop if no ',' found
                skip_whitespace( json, pos );
                if ( pos == std :: string :: npos || json [ pos ] != ',' )
                    break;
            }
            
            // must end on ']'
            if ( pos == std :: string :: npos || json [ pos ] != ']' )
                throw "Malformed array";
            
            // skip over ']'
            ++ pos;
        }
        catch ( ... )
        {
            delete array;
            throw;
        }
        
        return array;
    }
    
    JSONObject * JSONObject :: parse ( const std :: string & json )
    {
        if ( json . empty () )
            throw "empty JSON object";
        
        size_t pos = 0;
        skip_whitespace( json, pos );
        if ( json [ pos ] != '{' )
            throw "expecting '{'";
        
        return parse ( json, pos );
    }
    
    JSONObject * JSONObject :: parse ( const std :: string & json, size_t & pos )
    {
        assert ( json [ pos ] == '{' );
        
        JSONObject *obj = new JSONObject;
        try
        {
            while ( 1 )
            {
                // skip over '{' and any whitespace
                // json [ pos ] is '{' or ',', start at json [ pos + 1 ]
                skip_whitespace ( json, ++ pos );
                if ( pos == std :: string :: npos )
                    throw "expected '}'";

                if ( json [ pos ] == '}' )
                    break;
                
                // get the name/key
                JSONValue *name = JSONTypedValue < std::string > :: parse ( json, pos );
                if ( name == nullptr )
                    throw "Invalid JSON text";
                
                // skip to ':'
                skip_whitespace ( json, pos );
                if ( pos == std :: string :: npos || json [ pos ] != ':' )
                    throw "expected ':'";
                
                // skip over ':'
                ++ pos;
                
                // get JSON value;
                JSONValue *value = JSONValue :: parse ( json, pos );
                if ( value == nullptr )
                    throw "Invalid value";
                
                obj -> addMember ( name -> toString(), value );
                delete name;
                
                // find and skip over ',' and skip any whitespace
                // exit loop if no ',' found
                skip_whitespace ( json, pos );
                if ( pos == std :: string :: npos || json [ pos ] != ',' )
                    break;
            }
            
            // must end on '}'
            if ( pos == std :: string :: npos || json [ pos ] != '}' )
                throw "Malformed array";
            
            ++ pos;
        }
        catch ( ... )
        {
            delete obj;
            throw;
        }
        
        
        return obj;
    }
    
}
