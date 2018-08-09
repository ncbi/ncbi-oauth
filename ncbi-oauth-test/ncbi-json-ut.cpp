//
//  ncbi-json-ut.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/5/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "gtest/gtest.h"
#include <ncbi/json.hpp>

#include <iostream>

namespace ncbi
{
    /* JSON Construction
     *
     **********************************************************************************/
    class JSONFixture_JSONConstruction : public :: testing :: Test
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
        
        void make_throw ( JSONType type, const std :: string &json )
        {
            pos = 0;
            
            switch ( type )
            {
                case Object:
                    EXPECT_ANY_THROW ( JSONObject :: make ( json ) );
                    break;
                case Array:
                    EXPECT_ANY_THROW ( JSONArray :: make () );
                    break;
                case Value:
                    EXPECT_ANY_THROW ( JSONValue :: parse ( json, pos ) );
                    break;
            }
        }
        
        void make ( JSONType type, const std :: string &json, bool consume_all = true )
        {
            pos = 0;
            
            switch ( type )
            {
                case Object:
                {
                    JSONObject *obj = JSONObject :: make ( json );
                    ASSERT_TRUE ( obj != nullptr );
                    jObj = obj;
                    break;
                }
                case Array:
                {
                    JSONArray *array = JSONArray :: make ( json );
                    ASSERT_TRUE ( array != nullptr );
                    jObj = array;
                    break;
                }
                case Value:
                {
                    JSONValue *val = JSONValue :: parse ( json, pos );
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
        
        void make_and_verify_eq ( JSONType type, const std :: string &json, const std :: string &expected,
                                  bool consume_all = true )
        {
            make ( type, json, consume_all );
            EXPECT_STREQ ( jObj -> toJSON() . c_str(), expected . c_str () );
        }
    
    protected:
        size_t pos;
        JSONValue *jObj;
    };

    /* Object
     * {}
     * { members }
     */
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_Empty )
    {
        make_and_verify_eq ( Object , "{}", "{}" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_EmptyArray )
    {
        make_and_verify_eq ( Object , "{\"\":[]}", "{\"\":[]}" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_String_Member )
    {
        make_and_verify_eq ( Object , "{\"name\":\"value\"}", "{\"name\":\"value\"}" );
    }
    
    /* Array
     * []
     * [ elements ]
     */
    TEST_F ( JSONFixture_JSONConstruction, JSONArray_Empty )
    {
        make_and_verify_eq ( Array , "[]", "[]" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONArray_String_Elems )
    {
        make_and_verify_eq( Array , "[\"name\",\"value\"]", "[\"name\",\"value\"]" );
    }
  
    /* String
     * ""
     * " chars "
     */
    TEST_F ( JSONFixture_JSONConstruction, String_Empty )
    {
        make_and_verify_eq( Value , "\"\"", "\"\"" );
    }
    TEST_F ( JSONFixture_JSONConstruction, String_Char )
    {
        make_and_verify_eq( Value , "\"a\"", "\"a\"" );
    }
    TEST_F ( JSONFixture_JSONConstruction, String_Chars )
    {
        make_and_verify_eq( Value , "\"abc\"", "\"abc\"" );
    }
    
    /* Bool
     * true
     * false
     */
    TEST_F ( JSONFixture_JSONConstruction, Bool_True )
    {
        make_and_verify_eq( Value , "true", "true" );
    }
    TEST_F ( JSONFixture_JSONConstruction, Bool_False )
    {
        make_and_verify_eq( Value , "false", "false" );
    }
    
    /* Integer
     * digit
     * digit1-9 digits
     * -digit
     * -digit1-9 digits
     */
    TEST_F ( JSONFixture_JSONConstruction, Integer_Single )
    {
        make_and_verify_eq ( Value , "0", "0" );
    }
    TEST_F ( JSONFixture_JSONConstruction, Integer_Multiple )
    {
        make_and_verify_eq( Value , "12345", "12345" );
    }
    TEST_F ( JSONFixture_JSONConstruction, Integer_Single_Negative )
    {
        make_and_verify_eq( Value , "-0", "0" );
        make_and_verify_eq( Value , "-1", "-1" );
    }
    TEST_F ( JSONFixture_JSONConstruction, Integer_Multiple_Negative )
    {
        make_and_verify_eq( Value , "-12345", "-12345" );
    }
    
    /* Floating point
     * int frac
     * int exp
     * int frac exp
     */
    TEST_F ( JSONFixture_JSONConstruction, Float_Frac )
    {
        make_and_verify_eq( Value , "0.0", "0.0" );
        make_and_verify_eq( Value , "1.2", "1.2" );
    }
    TEST_F ( JSONFixture_JSONConstruction, Float_Frac_Precision )
    {
        make_and_verify_eq( Value , "1234.56789", "1234.56789" );
    }
    
    TEST_F ( JSONFixture_JSONConstruction, Float_eE_nodigit )
    {
        // invalid exp format, but construction should not fail
        // as it is the nature of parsers to consume tokens, not
        // entire strings - should return "0" and consumed
        // only one digit
        make_and_verify_eq ( Value , "0E", "0", false );
    }
    TEST_F ( JSONFixture_JSONConstruction, Float_eE_digit )
    {
        make ( Value , "0e0" );
        make ( Value , "0E0" );
        
    }
    TEST_F ( JSONFixture_JSONConstruction, Float_eE_plus_digits )
    {
        make ( Value , "0e+0" );
        make ( Value , "0E+0" );
    }
    TEST_F ( JSONFixture_JSONConstruction, Float_eE_minus_digits )
    {
        make ( Value , "0e-0" );
        make ( Value , "0E-0" );
    }

    TEST_F ( JSONFixture_JSONConstruction, Float_Frac_Exp )
    {
        make ( Value, "0.0e0" );
    }
    
    /* Exceptions
     */
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_Throw_Empty )
    {
        make_throw ( Object, "" );  // Empty JSON object
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_Throw_ExpecttRightBrace )
    {
        make_throw ( Object, "{" ); // Expected '}'
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_Throw_ExpectLeftBrace )
    {
        make_throw ( Object, "}" ); // Expected '{'
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_Throw_ExpectColon )
    {
        make_throw ( Object, "{\"name\"\"value\"" ); // Expected ':'
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONObject_Throw_ExpectRightBrace2 )
    {
        make_throw ( Object, "{\"name\":\"value\"" ); // Expected '}'
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONArray_Throw_Empty )
    {
        make_throw ( Array, "" );  // Empty JSON array
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONArray_Throw_ExpectRightBracket )
    {
        make_throw ( Array, "[" ); // Expected ']'
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONArray_Throw_ExpectRightBracket2 )
    {
        make_throw ( Array, "[\"name\",\"name\"" ); // Expected ']'
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONArray_Throw_ExpectLeftBracket )
    {
        make_throw ( Array, "]" ); // Expected '['
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_InvJSONFmt )
    {
        make_throw ( Value, "a" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_InvNullFmt_Missing )
    {
        make_throw ( Value, "n" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_InvNullFmt_Bad )
    {
        make_throw ( Value, "nulll" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvBeginFormat )
    {
        make_throw ( Value, "\"" ); // Invalid begin of string format
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvEscChar_Missing )
    {
        make_throw ( Value, "\"\\" ); // Invalid escape character
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvEscChar_Bad )
    {
        make_throw ( Value, "\"\\y" ); // Invalid escape character
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvUEscSeq_Missing )
    {
        make_throw ( Value, "\"\\u" ); // Invalid \u escape sequence
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvUEscSeq_Short )
    {
        make_throw ( Value, "\"\\uabc" ); // Invalid \u escape sequence
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvUEscSeq_Bad )
    {
        make_throw ( Value, "\"\\uabcz" ); // Invalid \u escape sequence
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_String_Throw_InvEndFormat )
    {
        make_throw ( Value, "\"\\u0061" ); // Invalid end of string format
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_Bool_Throw_True_Missing )
    {
        make_throw ( Value, "t" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_Bool_Throw_True_Bad )
    {
        make_throw ( Value, "truee" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_Bool_Throw_False_Missing )
    {
        make_throw ( Value, "f" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_Bool_Throw_False_Bad )
    {
        make_throw ( Value, "falsee" );
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_Integer_Throw_Negative_Missing )
    {
        make_throw ( Value, "-" ); // Expected digit
    }
    TEST_F ( JSONFixture_JSONConstruction, JSONValue_Integer_Throw_Negative_Bad )
    {
        make_throw ( Value, "-a" ); // Expected digit
    }
} // ncbi
