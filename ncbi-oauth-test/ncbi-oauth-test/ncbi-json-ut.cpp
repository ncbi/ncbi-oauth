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
    
    TEST ( JSONObject, Empty )
    {
        JSONObject *obj = JSONObject::parse ( "{}" );
        ASSERT_TRUE ( obj != nullptr );
        EXPECT_STREQ ( obj -> toString() . c_str(), "{}" );
        delete obj;
    }
    
    /* Array
     * []
     * [ elements ]
     */
    TEST ( JSONArray, Empty )
    {
        JSONArray *array = JSONArray::parse ( "[]" );
        ASSERT_TRUE ( array != nullptr );
        EXPECT_STREQ ( array -> toString() . c_str(), "[]" );
        delete array;
    }
    
    /* String
     * ""
     * " chars "
     */
    TEST ( JSONValue, String_Empty )
    {
        // Empty
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "\"\"", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "\"\"" );
        delete val;
    }
    
    TEST ( JSONValue, String_Char )
    {
        // Single character
        size_t pos = 0;
        JSONValue * val = JSONValue::parse ( "\"a\"", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "\"a\"" );
        delete val;
    }
    
    TEST ( JSONValue, String_Chars )
    {
        // String of characters
        size_t pos = 0;
        JSONValue * val = JSONValue::parse ( "\"abc\"", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "\"abc\"" );
        delete val;
    }
    
    /* Bool
     * true
     * false
     */
    TEST ( JSONValue, Bool_True )
    {
        // true
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "true", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "true" );
        delete val;
    }
    
    TEST ( JSONValue, Bool_False )
    {
        // false
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "false", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "false" );
        delete val;
    }

    /* Integer
     * digit
     * digit1-9 digits
     * -digit
     * -digit1-9 digits
     */
    TEST ( JSONValue, Integer_Single )
    {
        // single digit
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "0", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "0" );
        delete val;
    }
    
    TEST ( JSONValue, Integer_Multiple )
    {
        // multiple digits starting with 1
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "12345", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "12345" );
        delete val;
    }
    
    TEST ( JSONValue, Integer_Single_Negative )
    {
        // negative single digit
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "-0", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "0" );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "-1", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "-1" );
        delete val;
    }
    
    TEST ( JSONValue, Integer_Multiple_Negative )
    {
        // negative multiple digits starting with 1
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "-12345", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "-12345" );
        delete val;
    }
    
    /* Floating point
     * int frac
     * int exp
     * int frac exp
     */
    TEST ( JSONValue, Float_Frac )
    {
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "0.0", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "0" );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "1.2", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "1.2" );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "1234.5678", pos );
        ASSERT_TRUE ( val != nullptr );
        EXPECT_STREQ ( val -> toString() . c_str(), "1234.5678" );
        delete val;
    }
    
    TEST ( JSONValue, Float_Exp )
    {
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "0e0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "0e+0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "0e-0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "0E", pos ); // this WILL pass
        ASSERT_TRUE ( val != nullptr );       // should have returned an integer
        EXPECT_EQ ( pos, 1 );                 // but should only consume 1 digit
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "0E+0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
        
        pos = 0;
        val = JSONValue::parse ( "0E-0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
    }
    
    TEST ( JSONValue, Float_Frac_Exp )
    {
        size_t pos = 0;
        JSONValue *val = JSONValue::parse ( "0.0e0", pos );
        ASSERT_TRUE ( val != nullptr );
        delete val;
    }
    
} // ncbi
