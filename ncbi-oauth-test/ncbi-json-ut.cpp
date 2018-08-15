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
    
    /* JSONValue Construction - Method
     *
     **********************************************************************************/
    class JSONFixture_JSONValue_Interface : public :: testing :: Test
    {
    public:
        enum Type { jvt_null, jvt_bool, jvt_int, jvt_double,
            jvt_num, jvt_string, jvt_array, jvt_obj };
        
        void SetUp ()
        {
            jObj = nullptr;
        }
        
        void TearDown ()
        {
            delete jObj;
        }
        
        void assert_is_of_type ( const Type type )
        {
            switch ( type )
            {
            case jvt_null:
                ASSERT_TRUE ( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
            case jvt_bool:
                ASSERT_TRUE ( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
            case jvt_int:
                ASSERT_TRUE ( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
            case jvt_double:
                ASSERT_TRUE ( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
            case jvt_num:
                ASSERT_TRUE ( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
            case jvt_string:
                ASSERT_TRUE ( jObj -> isString () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
            case jvt_array:
                ASSERT_TRUE ( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isNull () );
                ASSERT_FALSE( jObj -> isObject () );
                break;
        case jvt_obj:
                ASSERT_TRUE ( jObj -> isObject () );
                ASSERT_FALSE( jObj -> isBool () );
                ASSERT_FALSE( jObj -> isInteger () );
                ASSERT_FALSE( jObj -> isNumber () );
                ASSERT_FALSE( jObj -> isString () );
                ASSERT_FALSE( jObj -> isArray () );
                ASSERT_FALSE( jObj -> isNull () );
                break;
            }
        }
        
        void set_types ( const Type type )
        {
            switch ( type )
            {
                case jvt_null:
                    ASSERT_TRUE ( ( jObj -> setBool ( true ) ) . isBool () );
                    ASSERT_TRUE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                    ASSERT_TRUE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setString ( "string" ) ) . isString () );
                    ASSERT_TRUE ( ( jObj -> setNull () ) . isNull () );
                    break;
                case jvt_bool:
                    ASSERT_TRUE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                    ASSERT_TRUE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setString ( "string" ) ) . isString () );
                    ASSERT_TRUE ( ( jObj -> setNull () ) . isNull () );
                    ASSERT_TRUE ( ( jObj -> setBool ( true ) ) . isBool () );
                    break;
                case jvt_int:
                    ASSERT_TRUE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setString ( "string" ) ) . isString () );
                    ASSERT_TRUE ( ( jObj -> setNull () ) . isNull () );
                    ASSERT_TRUE ( ( jObj -> setBool ( true ) ) . isBool () );
                    ASSERT_TRUE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                    break;
                case jvt_double:
                    ASSERT_TRUE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setString ( "string" ) ) . isString () );
                    ASSERT_TRUE ( ( jObj -> setNull () ) . isNull () );
                    ASSERT_TRUE ( ( jObj -> setBool ( true ) ) . isBool () );
                    ASSERT_TRUE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                    ASSERT_TRUE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                    break;
                case jvt_num:
                    ASSERT_TRUE ( ( jObj -> setString ( "string" ) ) . isString () );
                    ASSERT_TRUE ( ( jObj -> setNull () ) . isNull () );
                    ASSERT_TRUE ( ( jObj -> setBool ( true ) ) . isBool () );
                    ASSERT_TRUE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                    ASSERT_TRUE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                    break;
                case jvt_string:
                    ASSERT_TRUE ( ( jObj -> setNull () ) . isNull () );
                    ASSERT_TRUE ( ( jObj -> setBool ( true ) ) . isBool () );
                    ASSERT_TRUE ( ( jObj -> setInteger ( 123 ) ) . isInteger () );
                    ASSERT_TRUE ( ( jObj -> setDouble ( 123.456789, 10 ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setNumber ( "123.456789" ) ) . isNumber () );
                    ASSERT_TRUE ( ( jObj -> setString ( "string" ) ) . isString () );
                    break;
                case jvt_array:
                    EXPECT_ANY_THROW ( jObj -> setNull () );
                    EXPECT_ANY_THROW ( jObj -> setBool ( true ) );
                    EXPECT_ANY_THROW ( jObj -> setInteger ( 123 ) );
                    EXPECT_ANY_THROW ( jObj -> setDouble ( 123.456789, 10 ) );
                    EXPECT_ANY_THROW ( jObj -> setNumber ( "123.456789" ) );
                    EXPECT_ANY_THROW ( jObj -> setString ( "string" ) );
                    break;
                case jvt_obj:
                    EXPECT_ANY_THROW ( jObj -> setNull () );
                    EXPECT_ANY_THROW ( jObj -> setBool ( true ) );
                    EXPECT_ANY_THROW ( jObj -> setInteger ( 123 ) );
                    EXPECT_ANY_THROW ( jObj -> setDouble ( 123.456789, 10 ) );
                    EXPECT_ANY_THROW ( jObj -> setNumber ( "123.456789" ) );
                    EXPECT_ANY_THROW ( jObj -> setString ( "string" ) );
                    break;
            }
        }
        
        void to_types ( const Type type, const void *cmp = nullptr )
        {
            switch ( type )
            {
                case jvt_null:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_ANY_THROW  ( jObj -> toNumber() );
                    EXPECT_STREQ  ( jObj -> toString () . c_str (), "null" );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_bool:
                    ASSERT_TRUE  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_ANY_THROW  ( jObj -> toNumber() );
                    EXPECT_STREQ  ( jObj -> toString () . c_str (), ( const char * ) cmp );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_int:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_EQ  ( jObj -> toInteger (), * ( long long int * ) cmp  );
                    EXPECT_STREQ  ( jObj -> toNumber () . c_str (),
                                   std :: to_string ( * ( long long int * ) cmp ) . c_str () );
                    EXPECT_STREQ  ( jObj -> toString () . c_str (),
                                   std :: to_string ( * ( long long int * ) cmp ) . c_str () );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_double:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_STREQ  ( jObj -> toNumber () . c_str (),
                                   std :: to_string ( * ( long double * ) cmp ) . c_str () );
                    EXPECT_STREQ  ( jObj -> toString () . c_str (),
                                   std :: to_string ( * ( long double * ) cmp ) . c_str () );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_num:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_STREQ  ( jObj -> toNumber () . c_str (), ( const char * ) cmp );
                    EXPECT_STREQ  ( jObj -> toString () . c_str (), ( const char * ) cmp );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_string:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_ANY_THROW  ( jObj -> toNumber () );
                    EXPECT_STREQ  ( jObj -> toString () . c_str (), ( const char * ) cmp );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_array:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_ANY_THROW  ( jObj -> toNumber() );
                    EXPECT_ANY_THROW  ( jObj -> toString () );
                    EXPECT_NO_THROW  ( jObj -> toArray () );
                    EXPECT_ANY_THROW  ( jObj -> toObject () );
                    break;
                case jvt_obj:
                    EXPECT_ANY_THROW  ( jObj -> toBool () );
                    EXPECT_ANY_THROW  ( jObj -> toInteger () );
                    EXPECT_ANY_THROW  ( jObj -> toNumber() );
                    EXPECT_ANY_THROW  ( jObj -> toString () );
                    EXPECT_ANY_THROW  ( jObj -> toArray () );
                    EXPECT_NO_THROW  ( jObj -> toObject () );
                    break;
            }
        }

    protected:
        size_t pos;
        JSONValue *jObj;
    };
    
    // JSONObject
    TEST_F ( JSONFixture_JSONValue_Interface, t_null )
    {
        Type type = jvt_null;
        jObj = JSONValue :: makeNull ();
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type );
        JSONValue *clone = jObj -> clone ();
        ASSERT_TRUE( clone -> isNull () );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_bool )
    {
        Type type = jvt_bool;
        const char *val = "true";
        jObj = JSONValue :: makeBool ( true );
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type, val );
        JSONValue *clone = jObj -> clone ();
        ASSERT_TRUE( clone -> isBool () );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_integer )
    {
        Type type = jvt_int;
        long long int val = 123;
        jObj = JSONValue :: makeInteger ( val );
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type, &val );
        JSONValue *clone = jObj -> clone ();
        ASSERT_TRUE( clone -> isInteger () );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_double )
    {
        Type type = jvt_double;
        long double val = 123.456789;
        jObj = JSONValue :: makeDouble ( val, 10 );
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type, &val );
        JSONValue *clone = jObj -> clone ();
        ASSERT_TRUE( clone -> isNumber () );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_number )
    {
        Type type = jvt_num;
        const char * val = "123.456789";
        jObj = JSONValue :: makeNumber ( std :: string ( val ) );
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type, val );
        JSONValue *clone = jObj -> clone ();
        ASSERT_TRUE( clone -> isNumber () );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_string )
    {
        Type type = jvt_string;
        const char * val = "string";
        jObj = JSONValue :: makeString ( val );
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type, val );
        JSONValue *clone = jObj -> clone ();
        ASSERT_TRUE( clone -> isString () );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_array )
    {
        Type type = jvt_array;
        jObj = JSONArray :: make ();
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type );
    }
    TEST_F ( JSONFixture_JSONValue_Interface, t_obj )
    {
        Type type = jvt_obj;
        jObj = JSONObject :: make ();
        ASSERT_TRUE ( jObj != nullptr );
        assert_is_of_type ( type );
        set_types ( type );
        to_types ( type );
    }
    /* JSONObject Construction - Method
     *
     **********************************************************************************/
    class JSONFixture_JSONObject_Interface : public :: testing :: Test
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
    TEST_F ( JSONFixture_JSONObject_Interface, isObject )
    {
        make_empty();
        ASSERT_TRUE ( jObj -> isObject () );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, clone )
    {
        make_empty();
        JSONValue *obj = jObj -> clone ();
        ASSERT_TRUE ( obj -> isObject () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj -> toJSON() . c_str() );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, isEmpty )
    {
        make_empty();
        ASSERT_TRUE( jObj -> isEmpty() );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, exists )
    {
        make_empty();
        ASSERT_FALSE( jObj -> exists ( "name" ) );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, count )
    {
        make_empty();
        ASSERT_EQ ( jObj -> count (), 0 );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, getNames )
    {
        make_empty();
        ASSERT_TRUE ( jObj -> getNames () . empty () );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Null )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeNull () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":null}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Bool)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeBool ( true ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":true}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Integer)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeInteger( 123 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Double)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeDouble( 123.456789, 10 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123.456789}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Number)
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeNumber( "123.456789" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":123.456789}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_String )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Object )
    {
        make_empty();
        jObj -> setValue ( "obj", JSONObject :: make () );
        jObj -> getValue ( "obj" ) . toObject ()
        . setValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> getValue ( "obj" ) . toObject ()
        . setValue ( "number", JSONValue :: makeInteger ( 2 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"obj\":{\"name\":\"value\",\"number\":2}}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setValue_Array )
    {
        make_empty();
        jObj -> setValue ( "array", JSONArray :: make () );
        jObj -> getValue ( "array" ) . toArray ()
        . appendValue ( JSONValue :: makeString ( "first" ) );
        jObj -> getValue ( "array" ) . toArray ()
        . appendValue ( JSONValue :: makeInteger ( 2 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"array\":[\"first\",2]}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, setFinalValue )
    {
        make_empty();
        jObj -> setFinalValue ( "name", JSONValue :: makeString ( "value" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, getValue )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        JSONValue &val = jObj -> getValue ( "name" );
        EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, getConstValue )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        const JSONObject *cObj = jObj;
        const JSONValue &val =  cObj -> getValue ( "name" );
        EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, removeValue )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> removeValue ( "name" );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, removeFinalValue )
    {
        make_empty();
        jObj -> setFinalValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> removeValue ( "name" );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "{\"name\":\"value\"}" );
    }
    TEST_F ( JSONFixture_JSONObject_Interface, operator_equals )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        JSONObject obj = *jObj;
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
    }
    
    TEST_F ( JSONFixture_JSONObject_Interface, copy_constructor )
    {
        make_empty();
        jObj -> setValue ( "name", JSONValue :: makeString ( "value" ) );
        JSONObject obj ( *jObj );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
    }
    
    /* JSONArray Construction - Method
     *
     **********************************************************************************/
    class JSONFixture_JSONArray_Interface : public :: testing :: Test
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
            jObj = JSONArray :: make ();
            ASSERT_TRUE ( jObj != nullptr );
        }
        
    protected:
        size_t pos;
        JSONArray *jObj;
    };
    
    // JSONObject
    TEST_F ( JSONFixture_JSONArray_Interface, isArray )
    {
        make_empty();
        ASSERT_TRUE ( jObj -> isArray () );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, clone )
    {
        make_empty();
        JSONValue *obj = jObj -> clone ();
        ASSERT_TRUE ( obj -> isArray () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj -> toJSON() . c_str() );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, isEmpty )
    {
        make_empty();
        ASSERT_TRUE( jObj -> isEmpty() );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, exists )
    {
        make_empty();
        ASSERT_FALSE( jObj -> exists ( 3 ) );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, count )
    {
        make_empty();
        ASSERT_EQ ( jObj -> count (), 0 );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, appendValue )
    {
        make_empty();
        jObj -> appendValue ( JSONValue :: makeNull () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[null]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Null )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeNull () );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[null]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Bool)
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeBool ( true ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[true]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Integer)
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeInteger( 123 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[123]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Double)
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeDouble( 123.456789, 10 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[123.456789]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Number)
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeNumber( "123.456789" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[123.456789]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_String )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[\"value\"]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Object )
    {
        make_empty();
        jObj -> setValue ( 0, JSONObject :: make () );
        jObj -> getValue ( 0 ) . toObject ()
        . setValue ( "name", JSONValue :: makeString ( "value" ) );
        jObj -> getValue ( 0 ) . toObject ()
        . setValue ( "number", JSONValue :: makeInteger ( 2 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[{\"name\":\"value\",\"number\":2}]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, setValue_Array )
    {
        make_empty();
        jObj -> setValue ( 0, JSONArray :: make () );
        jObj -> getValue ( 0 ) . toArray ()
        . appendValue ( JSONValue :: makeString ( "first" ) );
        jObj -> getValue ( 0 ) . toArray ()
        . appendValue ( JSONValue :: makeInteger ( 2 ) );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[[\"first\",2]]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, getValue )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
        JSONValue &val = jObj -> getValue ( 0 );
        EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, getConstValue )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
        const JSONArray *cObj = jObj;
        const JSONValue &val =  cObj -> getValue ( 0 );
        EXPECT_STREQ ( val . toJSON() . c_str(), "\"value\"" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, removeValue )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
        jObj -> removeValue ( 0 );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), "[]" );
    }
    TEST_F ( JSONFixture_JSONArray_Interface, operator_equals )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
        JSONArray obj = *jObj;
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
    }
    
    TEST_F ( JSONFixture_JSONArray_Interface, copy_constructor )
    {
        make_empty();
        jObj -> setValue ( 0, JSONValue :: makeString ( "value" ) );
        JSONArray obj ( *jObj );
        EXPECT_STREQ ( jObj -> toJSON() . c_str(), obj . toJSON() . c_str () );
    }

    /* Fuzzing
     *
     **********************************************************************************/
    class JSONFixture_Fuzzing : public :: testing :: Test
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
        
        void run_crash_file ( const std :: string & name)
        {
            std :: string path = "crash-files/" + name;
            FILE *file = fopen ( path . c_str (), "rb" );
            if ( file != nullptr )
            {
                try
                {
                    fseek ( file, 0, SEEK_END );
                    long fSize = ftell ( file );
                    rewind ( file );
                
                    char *buff = new char [ fSize ];
                    try
                    {
                        size_t num_read = fread ( buff, 1, fSize, file );
                        if ( num_read == fSize )
                        {
                            EXPECT_ANY_THROW ( delete JSONObject :: make ( std :: string ( buff, num_read ) ) );
                        }
                    }
                    catch ( ... )
                    {
                        delete [] buff;
                        throw;
                    }
                    
                    delete [] buff;
                }
                catch ( ... )
                {
                    fclose ( file );
                    throw;
                }
                fclose ( file );
            }
        }
        
    protected:
        size_t pos;
        JSONObject *jObj;
    };
    
    TEST_F ( JSONFixture_Fuzzing, test1 )
    {
        run_crash_file ( "oom-019001c7b22ae7889a8cf8e09def61728fc8cbdd" );
    }
    
    TEST ( JSONFuzzing, test2 ) // fuzz crash but test doesnt
    {
    }
    
    TEST ( JSONFuzzing, test3 )
    {
    }
    
    TEST ( JSONFuzzing, test4 ) // fuzz crash but test doesnt
    {
    }
    
} // ncbi


