//
//  ncbi-jwt-ut.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/30/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "gtest/gtest.h"
#include "../../inc/jwt.hpp"

#include <iostream>

namespace ncbi
{
    /* JWT
     *
     **********************************************************************************/
    class JWTFixture_BasicConstruction : public :: testing :: Test
    {
    public:
        void SetUp ()
        {
            jwt = JWT :: make ();
        }
        
        void TearDown ()
        {
        }
        
        JWTFixture_BasicConstruction ()
        {
        }
        
    protected:
        JWT jwt;
    };
    
    TEST_F ( JWTFixture_BasicConstruction, JWT_Parse_Member )
    {
        jwt . payload [ "ncbi" ] = "specifics";
        
        std :: cout << jwt . encode () << std :: endl;
    }

} // namespace
