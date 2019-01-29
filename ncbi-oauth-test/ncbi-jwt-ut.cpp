//
//  ncbi-jwt-ut.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/30/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "gtest/gtest.h"
#include <ncbi/jws.hpp>
#include <ncbi/jwk.hpp>
#include <ncbi/jwt.hpp>

#include <iostream>

namespace ncbi
{
    // call this function with a reasonable but fixed value
    // before generating any JWT so that the result is predictable
    //
    // setting the value to 0 will revert to a real timestamp
    void jwt_setStaticCurrentTime ( long long cur_time );

    /* JWT
     *
     **********************************************************************************/
    class JWTFixture_BasicConstruction : public :: testing :: Test
    {
    public:

        void printClaims ( const JWTClaimSet & claims, bool recovered = false )
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
        
        void printJWT ( const JWT & jwt )
        {
            std :: cout
            << "---- JWT ----\n"
            << "    "
            << jwt
            << "\n---- JWT ----\n"
            << std :: endl
            ;
        }
        
        void printJWTTransitionStack ( const JWTClaimSet & json, const JWT & jwt, const JWTClaimSet & decoded )
        {
            std :: cout << "{" << std :: endl;
            printClaims ( json );
            printJWT ( jwt );
            printClaims ( decoded, true );
            std :: cout << "}\n" << std :: endl;
        }

        void SetUp ()
        {
            // fix the current time to a known value
            jwt_setStaticCurrentTime ( 1540664164 );

            const char jwk_json [] =
                "{"
                    "\"kty\":\"oct\","
                    "\"kid\":\"12345\","
                    "\"alg\":\"HS384\","
                    "\"k\":\"wonder-key-id\","
                "}";

            key = JWKMgr :: parseJWK ( jwk_json );

            const char jwk_set_json [] =
                "{"
                    "\"keys\":"
                    "["
                        "{"
                            "\"kty\":\"oct\","
                            "\"kid\":\"12345\","
                            "\"alg\":\"HS384\","
                            "\"k\":\"wonder-key-id\","
                        "}"
                    "]"
                "}";

            key_set = JWKMgr :: parseJWKSet ( jwk_set_json );
        }
        
        void TearDown ()
        {
            key = nullptr;
            key_set = nullptr;

            // restore timestamp behavior
            jwt_setStaticCurrentTime ( 0 );
        }
        
    protected:

        JWKRef key;
        JWKSetRef key_set;

    };
    
   
    TEST_F ( JWTFixture_BasicConstruction, JWT_Example )
    {
        JWTClaimSetRef claims = JWTMgr :: makeClaimSet ();
        claims -> addClaim ( "example", JSON :: makeString ( "hello there" ) );
        
        JWT jwt = JWTMgr :: sign ( * key, * claims );

        JWTClaimSetRef decoded = JWTMgr :: decode ( * key_set, jwt );
        
        printJWTTransitionStack ( * claims, jwt, * decoded );
    }

} // namespace
