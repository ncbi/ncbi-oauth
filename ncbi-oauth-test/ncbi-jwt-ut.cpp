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
            // fix the current time to a known value
            jwt_setStaticCurrentTime ( 1540664164 );

            // make a symmetric key
            HMAC_JWKey * key = HMAC_JWKey :: make ( "key-id-1234" );
            try
            {
                key -> setValue ( "blarky2" );

                jws_fact = new JWSFactory ( "ncbi", "HS384", key );
                jwt_fact = new JWTFactory ( * jws_fact );
                jwt_fact -> setIssuer ( "ncbi" );
                jwt_fact -> setDuration ( 15 );
            }
            catch ( ... )
            {
                key -> release ();
                throw;
            }
            key -> release ();
        }
        
        void TearDown ()
        {
            delete jwt_fact;
            delete jws_fact;

            // restore timestamp behavior
            jwt_setStaticCurrentTime ( 0 );
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
