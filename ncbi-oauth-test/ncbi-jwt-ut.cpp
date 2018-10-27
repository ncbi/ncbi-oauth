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
   
    TEST_F ( JWTFixture_BasicConstruction, JWT_Hello )
    {
        JWTClaims claims = jwt_fact -> make ();
        claims . addClaimOrDeleteValue ( "hello", JSONValue :: makeString ( "there" ) );
        std :: cout
            << "hello test:\n"
            << "  JSON:\n"
            << "    "
            << claims . toJSON ()
            << '\n'
            ;
        JWT jwt = jwt_fact -> sign ( claims );
        std :: cout
            << "  JWT:\n"
            << "    "
            << jwt
            << '\n'
            ;

        JWTClaims recovered = jwt_fact -> decode ( jwt );
        std :: cout
            << "Recovered claims:\n"
            << "  "
            << recovered . toJSON ()
            << std :: endl
            ;
    }

} // namespace
