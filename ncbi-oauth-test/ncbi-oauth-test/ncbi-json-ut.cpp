//
//  ncbi-json-ut.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/5/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "gtest/gtest.h"
#include "../../inc/json.hpp"

#include <iostream>

namespace ncbi
{
    class JSONFixture : public :: testing :: Test
    {
    public:
        enum JSONType { Object, Array, Value };
        
        void SetUp ()
        {
            pos = 0;
            jObj = nullptr;
        }
        
        void TearDown ()
        {
            delete jObj;
        }
        
        void parse_throw ( JSONType type, const std :: string &json )
        {
            pos = 0;
            
            switch ( type )
            {
                case Object:
                    EXPECT_ANY_THROW ( JSONObject :: parse ( json ) );
                    break;
                case Array:
                    EXPECT_ANY_THROW ( JSONArray :: parse ( json ) );
                    break;
                case Value:
                    EXPECT_ANY_THROW ( JSONValue :: parse ( json, pos ) );
                    break;
            }
        }
        
        void parse ( JSONType type, const std :: string &json, bool consume_all = true )
        {
            pos = 0;
            
            switch ( type )
            {
                case Object:
                {
                    JSONObject *obj = JSONObject::parse ( json );
                    ASSERT_TRUE ( obj != nullptr );
                    jObj = obj;
                    break;
                }
                case Array:
                {
                    JSONArray *array = JSONArray::parse ( json );
                    ASSERT_TRUE ( array != nullptr );
                    jObj = array;
                    break;
                }
                case Value:
                {
                    JSONValue *val = JSONValue::parse ( json, pos );
                    ASSERT_TRUE ( val != nullptr );
                    if ( consume_all )
                        ASSERT_TRUE ( pos == json . size () );
                    else
                        ASSERT_TRUE ( pos < json . size () );
                    
                    jObj = val;
                    break;
                }
            }
        }
        
        void parse_and_verify_eq ( JSONType type, const std :: string &json, const std :: string &expected,
                                  bool consume_all = true )
        {
            parse ( type, json, consume_all );
            EXPECT_STREQ ( jObj -> toString() . c_str(), expected . c_str () );
        }
    
    protected:
        size_t pos;
        JSONValue *jObj;
    };
    
    /* JSONException
     */
    TEST_F ( JSONFixture, JSONObject_Throw )
    {
        parse_throw ( Object, "" );  // Empty JSON object
        parse_throw ( Object, "{" ); // Expected '}'
        parse_throw ( Object, "}" ); // Expected '{'
    }
    
    TEST_F ( JSONFixture, JSONArray_Throw )
    {
        parse_throw ( Array, "" );  // Empty JSON array
        parse_throw ( Array, "[" ); // Expected ']'
        parse_throw ( Array, "]" ); // Expected '['
    }
    
    TEST_F ( JSONFixture, JSONValue_String_Throw )
    {
        parse_throw ( Value, "\"" ); // Invalid begin of string format
        parse_throw ( Value, "\"\\" ); // Invalid escape character
        parse_throw ( Value, "\"\\y" ); // Invalid escape character
        parse_throw ( Value, "\"\\u" ); // Invalid \u escape sequence
        parse_throw ( Value, "\"\\uabc" ); // Invalid \u escape sequence
        parse_throw ( Value, "\"\\uabcz" ); // Invalid \u escape sequence
        parse_throw ( Value, "\"\\u0061" ); // Invalid end of string format
    }
    
    /* Object
     * {}
     * { members }
     */
    TEST_F ( JSONFixture, JSONObject_Empty )
    {
        parse_and_verify_eq( Object , "{}", "{}" );
    }
    
    /* Array
     * []
     * [ elements ]
     */
    TEST_F ( JSONFixture, JSONArray_Empty )
    {
        parse_and_verify_eq( Array , "[]", "[]" );
    }
    
    /* String
     * ""
     * " chars "
     */
    TEST_F ( JSONFixture, String_Empty )
    {
        parse_and_verify_eq( Value , "\"\"", "\"\"" );
    }
    TEST_F ( JSONFixture, String_Char )
    {
        parse_and_verify_eq( Value , "\"a\"", "\"a\"" );
    }
    TEST_F ( JSONFixture, String_Chars )
    {
        parse_and_verify_eq( Value , "\"abc\"", "\"abc\"" );
    }
    
    /* Bool
     * true
     * false
     */
    TEST_F ( JSONFixture, Bool_True )
    {
        parse_and_verify_eq( Value , "true", "true" );
    }
    TEST_F ( JSONFixture, Bool_False )
    {
        parse_and_verify_eq( Value , "false", "false" );
    }
    
    /* Integer
     * digit
     * digit1-9 digits
     * -digit
     * -digit1-9 digits
     */
    TEST_F ( JSONFixture, Integer_Single )
    {
        parse_and_verify_eq( Value , "0", "0" );
    }
    TEST_F ( JSONFixture, Integer_Multiple )
    {
        parse_and_verify_eq( Value , "12345", "12345" );
    }
    TEST_F ( JSONFixture, Integer_Single_Negative )
    {
        parse_and_verify_eq( Value , "-0", "0" );
        parse_and_verify_eq( Value , "-1", "-1" );
    }
    TEST_F ( JSONFixture, Integer_Multiple_Negative )
    {
        parse_and_verify_eq( Value , "-12345", "-12345" );
    }
    
    /* Floating point
     * int frac
     * int exp
     * int frac exp
     */
    TEST_F ( JSONFixture, Float_Frac )
    {
        parse_and_verify_eq( Value , "0.0", "0" );
        parse_and_verify_eq( Value , "1.2", "1.2" );
    }
    TEST_F ( JSONFixture, Float_Frac_Precision )
    {
        parse_and_verify_eq( Value , "1234.56789", "1234.56789" );
    }
    
    TEST_F ( JSONFixture, Float_eE_nodigit )
    {
        // invalid exp format, but construction should not fail
        // as it is the nature of parsers to consume tokens, not
        // entire strings - should return "0" and consumer
        // only one digit
        parse_and_verify_eq ( Value , "0E", "0", false );
    }
    TEST_F ( JSONFixture, Float_eE_digit )
    {
        parse ( Value , "0e0" );
        parse ( Value , "0E0" );
        
    }
    TEST_F ( JSONFixture, Float_eE_plus_digits )
    {
        parse ( Value , "0e+0" );
        parse ( Value , "0E+0" );
    }
    TEST_F ( JSONFixture, Float_eE_minus_digits )
    {
        parse ( Value , "0e-0" );
        parse ( Value , "0E-0" );
    }

    TEST_F ( JSONFixture, Float_Frac_Exp )
    {
        parse ( Value, "0.0e0" );
    }
    
} // ncbi
