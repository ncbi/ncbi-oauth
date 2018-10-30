//
//  ncbi-jwt-ut.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/30/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "gtest/gtest.h"
#include <ncbi/jws.hpp>
#include <ncbi/jwt.hpp>

#include <iostream>

namespace ncbi
{
    /* JWT
     *
     **********************************************************************************/
    class JWTFixture_BasicConstruction : public :: testing :: Test
    {
    public:
        void printClaims ( JWTClaims claims, bool recovered = false )
        {
            if ( ! recovered )
            {
                std :: cout
                << "---- JSON Claims ----\n"
                << "    "
                << claims . toJSON ()
                << "\n---- JSON Claims ----\n"
                << std :: endl
                ;
            }
            else
            {
                std :: cout
                << "---- Decoded Claims ----\n"
                << "    "
                << claims . toJSON ()
                << "\n---- Decoded Claims ----"
                << std :: endl
                ;
            }
        }
        
        void printJWT ( JWT jwt )
        {
            std :: cout
            << "---- JWT ----\n"
            << "    "
            << jwt
            << "\n---- JWT ----\n"
            << std :: endl
            ;
        }
        
        void printJWTTransitionStack ( JWTClaims json, JWT jwt, JWTClaims decoded )
        {
            std :: cout << "{" << std :: endl;
            printClaims ( json );
            printJWT ( jwt );
            printClaims ( decoded, true );
            std :: cout << "}\n" << std :: endl;
        }

        void SetUp ()
        {
            jws_fact = new JWSFactory ( "ncbi", "HS384", "blarky2", "key-id-1234", "blarky2" );
            jwt_fact = new JWTFactory ( * jws_fact );
            ASSERT_NO_THROW ( jwt_fact -> setIssuer ( "ncbi" ) );
            ASSERT_NO_THROW ( jwt_fact -> setSubject ( "john doe" ) );
            ASSERT_NO_THROW ( jwt_fact -> setDuration ( 15 ) );
            //ASSERT_NO_THROW ( jwt_fact -> setNotBefore ( 60 ) );
        }
        
        void TearDown ()
        {
            delete jwt_fact;
            delete jws_fact;
        }
        
    protected:

        JWSFactory * jws_fact;
        JWTFactory * jwt_fact;
    };
    
   
    TEST_F ( JWTFixture_BasicConstruction, JWT_Example )
    {
        JWTClaims claims = jwt_fact -> make ();
        claims . addClaimOrDeleteValue ( "example", JSONValue :: makeString ( "hello there" ) );
        
        JWT jwt = jwt_fact -> sign ( claims );

        JWTClaims decoded = jwt_fact -> decode ( jwt );
        
        printJWTTransitionStack ( claims, jwt, decoded );
    }

} // namespace
