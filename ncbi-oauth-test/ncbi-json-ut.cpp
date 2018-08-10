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
    /* JSON Construction - Whitebox
     *
     **********************************************************************************/
    class JSONFixture_WhiteBox : public :: testing :: Test
    {
    public:
        enum JSONType { Object, Array, Value };
        
        void SetUp ()
        {
            jObj = nullptr;
        }
        
        void TearDown ()
        {
            delete jObj;
        }
        
        void make_throw ( JSONType type, const std :: string &json, bool consume_all = true )
        {
            switch ( type )
            {
                case Object:
                    EXPECT_ANY_THROW ( JSONObject :: make ( json ) );
                    break;
                case Array:
                    EXPECT_ANY_THROW ( JSONArray :: test_parse ( json ) );
                    break;
                case Value:
                    EXPECT_ANY_THROW ( JSONValue :: test_parse ( json, consume_all ) );
                    break;
            }
        }
        
        void make ( JSONType type, const std :: string &json, bool consume_all = true )
        {
            switch ( type )
            {
                case Object:
                {
                    jObj = JSONObject :: make ( json );
                    break;
                }
                case Array:
                {
                    jObj = JSONArray :: test_parse ( json );
                    break;
                }
                case Value:
                {
                    JSONValue *val = JSONValue :: test_parse ( json, consume_all );
                    ASSERT_TRUE ( val != nullptr );
                    
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
        JSONValue *jObj;
    };

    /* Object
     * {}
     * { members }
     */
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Empty )
    {
        make_and_verify_eq ( Object , "{}", "{}" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_EmptyArray )
    {
        make_and_verify_eq ( Object , "{\"\":[]}", "{\"\":[]}" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_String_Member )
    {
        make_and_verify_eq ( Object , "{\"name\":\"value\"}", "{\"name\":\"value\"}" );
    }
    
    // JSONObject Exceptions
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Throw_Empty )
    {
        make_throw ( Object, "" );  // Empty JSON object
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Throw_ExpecttRightBrace )
    {
        make_throw ( Object, "{" ); // Expected '}'
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Throw_ExpectLeftBrace )
    {
        make_throw ( Object, "}" ); // Expected '{'
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Throw_ExpectColon )
    {
        make_throw ( Object, "{\"name\"\"value\"" ); // Expected ':'
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Throw_ExpectRightBrace2 )
    {
        make_throw ( Object, "{\"name\":\"value\"" ); // Expected '}'
    }
    TEST_F ( JSONFixture_WhiteBox, JSONObject_Throw_TrailingBytes )
    {
        make_throw ( Object, "{\"name\":\"value\"}trailing" ); // Expected '}'
    }
    
    /* Array
     * []
     * [ elements ]
     */
    TEST_F ( JSONFixture_WhiteBox, JSONArray_Empty )
    {
        make_and_verify_eq ( Array , "[]", "[]" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONArray_String_Elems )
    {
        make_and_verify_eq( Array , "[\"name\",\"value\"]", "[\"name\",\"value\"]" );
    }
    
    // JSONArray Exceptions
    TEST_F ( JSONFixture_WhiteBox, JSONArray_Throw_Empty )
    {
        make_throw ( Array, "" );  // Empty JSON array
    }
    TEST_F ( JSONFixture_WhiteBox, JSONArray_Throw_ExpectLeftBracket )
    {
        make_throw ( Array, "]" ); // Expected '['
    }
    TEST_F ( JSONFixture_WhiteBox, JSONArray_Throw_ExpectRightBracket )
    {
        make_throw ( Array, "[" ); // Expected ']'
    }
    TEST_F ( JSONFixture_WhiteBox, JSONArray_Throw_ExpectRightBracket2 )
    {
        make_throw ( Array, "[\"name\",\"name\"" ); // Expected ']'
    }
    TEST_F ( JSONFixture_WhiteBox, JSONArray_Throw_TrailingBytes )
    {
        make_throw ( Array, "[\"name\",\"name\"]trailing" ); // Expected ']'
    }
    
    /* Bool
     * true
     * false
     */
    TEST_F ( JSONFixture_WhiteBox, Bool_True )
    {
        make_and_verify_eq( Value , "true", "true" );
    }
    TEST_F ( JSONFixture_WhiteBox, Bool_False )
    {
        make_and_verify_eq( Value , "false", "false" );
    }
    
    /* Integer
     * digit
     * digit1-9 digits
     * -digit
     * -digit1-9 digits
     */
    TEST_F ( JSONFixture_WhiteBox, Integer_Single )
    {
        make_and_verify_eq ( Value , "0", "0" );
    }
    TEST_F ( JSONFixture_WhiteBox, Integer_Multiple )
    {
        make_and_verify_eq( Value , "12345", "12345" );
    }
    TEST_F ( JSONFixture_WhiteBox, Integer_Single_Negative )
    {
        make_and_verify_eq( Value , "-0", "0" );
        make_and_verify_eq( Value , "-1", "-1" );
    }
    TEST_F ( JSONFixture_WhiteBox, Integer_Multiple_Negative )
    {
        make_and_verify_eq( Value , "-12345", "-12345" );
    }
    
    /* Floating point
     * int frac
     * int exp
     * int frac exp
     */
    TEST_F ( JSONFixture_WhiteBox, Float_Frac )
    {
        make_and_verify_eq( Value , "0.0", "0.0" );
        make_and_verify_eq( Value , "1.2", "1.2" );
    }
    TEST_F ( JSONFixture_WhiteBox, Float_Frac_Precision )
    {
        make_and_verify_eq( Value , "1234.56789", "1234.56789" );
    }
    
    TEST_F ( JSONFixture_WhiteBox, Float_eE_nodigit )
    {
        // invalid exp format, but construction should not fail
        // as it is the nature of parsers to consume tokens, not
        // entire strings - should return "0" and consumed
        // only one digit
        make_and_verify_eq ( Value , "0E", "0", false );
    }
    TEST_F ( JSONFixture_WhiteBox, Float_eE_digit )
    {
        make ( Value , "0e0" );
        make ( Value , "0E0" );
        
    }
    TEST_F ( JSONFixture_WhiteBox, Float_eE_plus_digits )
    {
        make ( Value , "0e+0" );
        make ( Value , "0E+0" );
    }
    TEST_F ( JSONFixture_WhiteBox, Float_eE_minus_digits )
    {
        make ( Value , "0e-0" );
        make ( Value , "0E-0" );
    }

    TEST_F ( JSONFixture_WhiteBox, Float_Frac_Exp )
    {
        make ( Value, "0.0e0" );
    }
    
    /* String
     * ""
     * " chars "
     */
    TEST_F ( JSONFixture_WhiteBox, String_Empty )
    {
        make_and_verify_eq( Value , "\"\"", "\"\"" );
    }
    TEST_F ( JSONFixture_WhiteBox, String_Char )
    {
        make_and_verify_eq( Value , "\"a\"", "\"a\"" );
    }
    TEST_F ( JSONFixture_WhiteBox, String_Chars )
    {
        make_and_verify_eq( Value , "\"abc\"", "\"abc\"" );
    }
    
    // JSONValue Exceptions
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Null_Throw_InvJSONFmt )
    {
        make_throw ( Value, "a" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Null_Throw_InvNullFmt_Missing )
    {
        make_throw ( Value, "n" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Null_Throw_InvNullFmt_Bad )
    {
        make_throw ( Value, "nulll" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvBeginFormat )
    {
        make_throw ( Value, "\"" ); // Invalid begin of string format
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvEscChar_Missing )
    {
        make_throw ( Value, "\"\\" ); // Invalid escape character
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvEscChar_Bad )
    {
        make_throw ( Value, "\"\\y" ); // Invalid escape character
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvUEscSeq_Missing )
    {
        make_throw ( Value, "\"\\u" ); // Invalid \u escape sequence
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvUEscSeq_Short )
    {
        make_throw ( Value, "\"\\uabc" ); // Invalid \u escape sequence
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvUEscSeq_Bad )
    {
        make_throw ( Value, "\"\\uabcz" ); // Invalid \u escape sequence
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_InvEndFormat )
    {
        make_throw ( Value, "\"\\u0061" ); // Invalid end of string format
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_String_Throw_Trailing )
    {
        make_throw ( Value, "\"validtext\"trailing" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Bool_Throw_True_Missing )
    {
        make_throw ( Value, "t" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Bool_Throw_True_Bad )
    {
        make_throw ( Value, "truee" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Bool_Throw_False_Missing )
    {
        make_throw ( Value, "f" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Bool_Throw_False_Bad )
    {
        make_throw ( Value, "falsee" );
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Integer_Throw_Negative_Missing )
    {
        make_throw ( Value, "-" ); // Expected digit
    }
    TEST_F ( JSONFixture_WhiteBox, JSONValue_Integer_Throw_Negative_Bad )
    {
        make_throw ( Value, "-a" ); // Expected digit
    }
    
    /* JSON Construction - Blackbox
     *
     **********************************************************************************/
    class JSONFixture_BlackBox : public :: testing :: Test
    {
    public:
        void SetUp ()
        {
            jObj = nullptr;
        }
        
        void TearDown ()
        {
            delete jObj;
        }
        
        void make_and_verify_eq ( const std :: string &json, const std :: string &expected )
        {
            jObj = JSONObject :: make ( json );
            EXPECT_STREQ ( jObj -> toJSON() . c_str(), expected . c_str () );
        }
        
    protected:
        size_t pos;
        JSONObject *jObj;
    };
    
    
    // Construction via JSON source
    TEST_F ( JSONFixture_BlackBox, JSONObject_Empty )
    {
        make_and_verify_eq ( "{}", "{}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Nested_Obj1 )
    {
        make_and_verify_eq ( "{\"nested-obj\":{}}", "{\"nested-obj\":{}}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Nested_Obj2 )
    {
        make_and_verify_eq ( "{\"nested-obj\":{\"nested-array\":[]}}",
                             "{\"nested-obj\":{\"nested-array\":[]}}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Nested_Obj3 )
    {
        make_and_verify_eq ( "{\"nested-obj\":{\"nested-obj\":{}}}",
                             "{\"nested-obj\":{\"nested-obj\":{}}}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_EmptyArray )
    {
        make_and_verify_eq ( "{\"array\":[]}", "{\"array\":[]}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Bool_Member1 )
    {
        make_and_verify_eq ( "{\"name\":true}", "{\"name\":true}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Bool_Member2 )
    {
        make_and_verify_eq ( "{\"name\":false}", "{\"name\":false}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Number_Member1 )
    {
        make_and_verify_eq ( "{\"name\":0}", "{\"name\":0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Number_Member2 )
    {
        make_and_verify_eq ( "{\"name\":1234567890}", "{\"name\":1234567890}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_NegNumber_Member1 )
    {
        make_and_verify_eq ( "{\"name\":-1}", "{\"name\":-1}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_NegNumber_Member2 )
    {
        make_and_verify_eq ( "{\"name\":-1234567890}", "{\"name\":-1234567890}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_Frac1 )
    {
        make_and_verify_eq( "{\"name\":0.0}", "{\"name\":0.0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_Frac2 )
    {
        make_and_verify_eq( "{\"name\":123.456789}", "{\"name\":123.456789}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_eE_digit1 )
    {
        make_and_verify_eq( "{\"name\":0e0}", "{\"name\":0e0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_eE_digit2 )
    {
        make_and_verify_eq( "{\"name\":0E0}", "{\"name\":0E0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_eE_plus_digit )
    {
        make_and_verify_eq( "{\"name\":0e+0}", "{\"name\":0e+0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_eE_minus_digit )
    {
        make_and_verify_eq( "{\"name\":0E-0}", "{\"name\":0E-0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Float_Frac_Exp )
    {
        make_and_verify_eq( "{\"name\":0.0E0}", "{\"name\":0.0E0}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_String_Member )
    {
        make_and_verify_eq ( "{\"name\":\"value\"}", "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_BlackBox, JSONObject_Member_Array )
    {
        make_and_verify_eq ( "{\"\":[true,false]}", "{\"\":[true,false]}" );
    }
    
    /* JSON Construction - Method
     *
     **********************************************************************************/
    class JSONFixture_BlackBox_Method : public :: testing :: Test
    {
    public:
        void SetUp ()
        {
            jObj = nullptr;
        }
        
        void TearDown ()
        {
            delete jObj;
        }
        
        void make_empty ()
        {
            jObj = JSONObject :: make ();
            ASSERT_TRUE ( jObj != nullptr );
        }

    protected:
        size_t pos;
        JSONObject *jObj;
    };
    
    // JSONObject
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_isObject )
    {
        make_empty();
        ASSERT_TRUE ( jObj -> isObject () );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_clone )
    {
        make_empty();
        JSONValue *obj = jObj -> clone ();
        ASSERT_TRUE ( obj -> isObject () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj -> toJSON() . c_str() );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_isEmpty )
    {
        make_empty();
        ASSERT_TRUE( jObj -> isEmpty() );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_exists )
    {
        make_empty();
        ASSERT_FALSE( jObj -> exists ( "name" ) );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_count )
    {
        make_empty();
        ASSERT_EQ ( jObj -> count (), 0 );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_getNames )
    {
        make_empty();
        ASSERT_TRUE ( jObj -> getNames () . empty () );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Null )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeNull () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":null}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Bool)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeBool ( true ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":true}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Integer)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeInteger( 123 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Double)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeDouble( 123.456789, 10 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123.456789}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Number)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeNumber( "123.456789" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123.456789}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_String )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Object )
    {
        make_empty();
        jObj -> setValue ( "obj", JSONObject :: make () );
        jObj -> getValue ( "obj" ) . toObject ()
        . setValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> getValue ( "obj" ) . toObject ()
        . setValue ( "number", JSONValue :: makeInteger ( 2 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"obj\":{\"name\":\"value\",\"number\":2}}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setValue_Array )
    {
        make_empty();
        jObj -> setValue ( "array", JSONArray :: make () );
        jObj -> getValue ( "array" ) . toArray ()
        . appendValue ( JSONValue :: makeString ( "first" ) );
        jObj -> getValue ( "array" ) . toArray ()
        . appendValue ( JSONValue :: makeInteger ( 2 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"array\":[\"first\",2]}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_setFinalValue )
    {
        make_empty();
        jObj -> setFinalValue ( "name", JSONValue :: makeString ( "value" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_getValue )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        JSONValue &val = jObj -> getValue ( "name" );
        EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_const_getValue )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        const JSONObject *cObj = jObj;
        const JSONValue &val =  cObj -> getValue ( "name" );
        EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_removeValue )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> removeValue ( "name" );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_removeFinalValue )
    {
        make_empty();
        jObj -> setFinalValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> removeValue ( "name" );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_operator_equals )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        JSONObject obj = *jObj;
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
    }
    
    TEST_F ( JSONFixture_BlackBox_Method, JSONObject_copy_constructor )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        JSONObject obj ( *jObj );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
    }
    
} // ncbi
