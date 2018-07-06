//
//  main.cpp
//  ncbi-oauth-test
//
//  Created by User on 7/5/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "gtest/gtest.h"

#include <iostream>

int main(int argc, const char * argv[])
{
    testing :: InitGoogleTest ( & argc, ( char ** ) argv );
    return RUN_ALL_TESTS ();
}
