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
        void SetUp ()
        {
            pos = 0;
            obj = nullptr;
        }
        
        void TearDown ()
        {
            delete obj;
        }
        
        void parse ( const std :: string &json, bool consume_all = true )
        {
            pos = 0;
            obj = JSONValue::parse ( json, pos );
            ASSERT_TRUE ( obj != nullptr );
            
            if ( consume_all )
                ASSERT_TRUE ( pos == json . size () );
            else
                ASSERT_TRUE ( pos < json . size () );
        }
        
        void parse_and_verify_eq ( const std :: string &json, const std :: string &expected, bool consume_all = true )
        {
            parse ( json, consume_all );
            EXPECT_STREQ ( obj -> toString() . c_str(), expected . c_str () );
        }
    
    protected:
        size_t pos;
        JSONValue *obj;
    };
    
    /* JSONException
     */
    TEST ( JSONException, JSONObject )
    {
        EXPECT_ANY_THROW ( JSONObject :: parse ( "{" ) );
        EXPECT_ANY_THROW ( JSONObject :: parse ( "}" ) );
    }
    
    TEST ( JSONException, JSONArray )
    {
        EXPECT_ANY_THROW ( JSONObject :: parse ( "[" ) );
        EXPECT_ANY_THROW ( JSONObject :: parse ( "]" ) );
    }
    
    TEST ( JSONException, JSONValue_String )
    {
        size_t pos = 0;
        EXPECT_ANY_THROW ( JSONValue::parse ( "\"", pos ) );
        pos = 0;
        EXPECT_ANY_THROW ( JSONValue::parse ( "\"\\", pos ) );
        pos = 0;
        EXPECT_ANY_THROW ( JSONValue::parse ( "\"\\y", pos ) );
        pos = 0;
        EXPECT_ANY_THROW ( JSONValue::parse ( "\"\\u", pos ) );
        pos = 0;
        JSONValue *val = JSONValue::parse ( "\"\\u1fc0", pos );
        delete val;
        //EXPECT_ANY_THROW ( JSONValue::parse ( "\"\\u1fc0", pos ) );
    }
    
    /* Object
     * {}
     * { members }
     */
    TEST_F ( JSONFixture, JSONObject_Empty )
    {
        parse_and_verify_eq( "{}", "{}" );
    }
    
    /* Array
     * []
     * [ elements ]
     */
    TEST_F ( JSONFixture, JSONArray_Empty )
    {
        parse_and_verify_eq( "[]", "[]" );
    }
    
    /* String
     * ""
     * " chars "
     */
    TEST_F ( JSONFixture, String_Empty )
    {
        parse_and_verify_eq( "\"\"", "\"\"" );
    }
    TEST_F ( JSONFixture, String_Char )
    {
        parse_and_verify_eq( "\"a\"", "\"a\"" );
    }
    TEST_F ( JSONFixture, String_Chars )
    {
        parse_and_verify_eq( "\"abc\"", "\"abc\"" );
    }
    
    /* Bool
     * true
     * false
     */
    TEST_F ( JSONFixture, Bool_True )
    {
        parse_and_verify_eq( "true", "true" );
    }
    TEST_F ( JSONFixture, Bool_False )
    {
        parse_and_verify_eq( "false", "false" );
    }
    
    /* Integer
     * digit
     * digit1-9 digits
     * -digit
     * -digit1-9 digits
     */
    TEST_F ( JSONFixture, Integer_Single )
    {
        parse_and_verify_eq( "0", "0" );
    }
    TEST_F ( JSONFixture, Integer_Multiple )
    {
        parse_and_verify_eq( "12345", "12345" );
    }
    TEST_F ( JSONFixture, Integer_Single_Negative )
    {
        parse_and_verify_eq( "-0", "0" );
        parse_and_verify_eq( "-1", "-1" );
    }
    TEST_F ( JSONFixture, Integer_Multiple_Negative )
    {
        parse_and_verify_eq( "-12345", "-12345" );
    }
    
    /* Floating point
     * int frac
     * int exp
     * int frac exp
     */
    TEST_F ( JSONFixture, Float_Frac )
    {
        parse_and_verify_eq( "0.0", "0" );
        parse_and_verify_eq( "1.2", "1.2" );
    }
    TEST_F ( JSONFixture, Float_Frac_Precision )
    {
        parse_and_verify_eq( "1234.56789", "1234.56789" );
    }
    
    TEST_F ( JSONFixture, Float_eE_nodigit )
    {
        // invalid exp format, but  construction should not fail.
        // as it is the nature of parsers to consume tokens
        // disregarding context - should return "0" and consumer
        // only one digit
        parse_and_verify_eq ( "0E", "0", false );
    }
    TEST_F ( JSONFixture, Float_eE_digit )
    {
        parse ( "0e0" );
        parse ( "0E0" );
        
    }
    TEST_F ( JSONFixture, Float_eE_plus_digits )
    {
        parse ( "0e+0" );
        parse ( "0E+0" );
    }
    TEST_F ( JSONFixture, Float_eE_minus_digits )
    {
        parse ( "0e-0" );
        parse ( "0E-0" );
    }

    TEST ( JSONValue, Float_Frac_Exp )
    {
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "0.0e0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
    }
    
} // ncbi
