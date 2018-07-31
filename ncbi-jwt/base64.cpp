//
//  jwt-base64.cpp
//  ncbi-jwt
//
//  Created by User on 7/30/18.
//  Copyright © 2018 NCBI. All rights reserved.
//

#include "base64-priv.hpp"

namespace ncbi
{
    static
    const char encode_table [] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-_";
    
    static
    const char decode_table [] =
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \x00 .. \x0F
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \x10 .. \x1F
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x3e\xff\xff" // \x20 .. \x2F
    "\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\xff\xff\xff\xfe\xff\xff" // \x30 .. \x3F
    "\xff\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e" // \x40 .. \x4F
    "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\xff\xff\xff\xff\x3f" // \x50 .. \x5F
    "\xff\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28" // \x60 .. \x6F
    "\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\xff\xff\xff\xff\xff" // \x70 .. \x7F
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \x80 .. \x8F
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \x90 .. \x9F
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \xA0 .. \xAF
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \xB0 .. \xBF
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \xC0 .. \xCF
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \xD0 .. \xDF
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \xE0 .. \xEF
    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" // \xF0 .. \xFF
    ;
    
    const std :: string encodeBase64URL ( const std :: string &json )
    {
        std :: string encoding;
        
        char buff [ 4096 ];
        size_t i, j, len = json . size ();
        
        uint32_t acc;
        const unsigned char * js = ( const unsigned char * ) json . c_str ();
        
        for ( i = j = 0; i + 2 < len; i += 3, j += 4 )
        {
            acc
            = ( ( uint32_t ) js [ i + 0 ] << 16 )
            | ( ( uint32_t ) js [ i + 1 ] <<  8 )
            | ( ( uint32_t ) js [ i + 2 ] <<  0 )
            ;
            
            if ( j == sizeof buff )
            {
                encoding += std :: string ( buff, j );
                j = 0;
            }
            
            buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
            buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
            buff [ j + 2 ] = encode_table [ ( acc >>  6 ) & 0x3F ];
            buff [ j + 3 ] = encode_table [ ( acc >>  0 ) & 0x3F ];
        }
        
        switch ( len - i )
        {
            case 0:
                break;
                
            case 1:
                
                acc
                = ( ( uint32_t ) js [ i + 0 ] << 16 )
                ;
                
                if ( j == sizeof buff )
                {
                    encoding += std :: string ( buff, j );
                    j = 0;
                }
                
                buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
                buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
                buff [ j + 2 ] = '=';
                buff [ j + 3 ] = '=';
                
#if BASE64_PAD_ENCODING
                j += 4;
#else
                j += 2;
#endif
                
                break;
                
            case 2:
                
                acc
                = ( ( uint32_t ) js [ i + 0 ] << 16 )
                | ( ( uint32_t ) js [ i + 1 ] <<  8 )
                ;
                
                if ( j == sizeof buff )
                {
                    encoding += std :: string ( buff, j );
                    j = 0;
                }
                
                buff [ j + 0 ] = encode_table [ ( acc >> 18 ) & 0x3F ];
                buff [ j + 1 ] = encode_table [ ( acc >> 12 ) & 0x3F ];
                buff [ j + 2 ] = encode_table [ ( acc >>  6 ) & 0x3F ];
                buff [ j + 3 ] = '=';
                
#if BASE64_PAD_ENCODING
                j += 4;
#else
                j += 3;
#endif
                
                break;
                
            default:
                throw "1 - aaaah!";
        }
        
        if ( j != 0 )
            encoding += std :: string ( buff, j );
        
        return encoding;
    }
    
    const std :: string decodeBase64URL ( const std :: string &encoding )
    {
        std :: string json;
        
        char buff [ 4096 ];
        size_t i, j, len = encoding . size ();
        
        uint32_t acc, ac;
        const unsigned char * en = ( const unsigned char * ) encoding . c_str ();
        
        for ( i = j = 0, acc = ac = 0; i < len; ++ i )
        {
            int byte = decode_table [ en [ i ] ];
            if ( byte >= 0 )
            {
                acc <<= 6;
                acc |= byte;
                if ( ++ ac == 4 )
                {
                    buff [ j + 0 ] = ( char ) ( acc >> 16 );
                    buff [ j + 1 ] = ( char ) ( acc >>  8 );
                    buff [ j + 2 ] = ( char ) ( acc >>  0 );
                    ac = 0;
                    
                    j += 3;
                    if ( j + 3 > sizeof buff )
                    {
                        json += std :: string ( buff, j );
                        j = 0;
                    }
                }
            }
            else if ( byte == -2 )
                break;
#if 0
            else
            {
                // have to take a decision about illegal characters
                // white-space and line endings should be okay
                // when okay, just skip them
                // otherwise, consider the string corrupt
            }
#endif
        }
        
        switch ( ac )
        {
            case 0:
                break;
            case 1:
                throw "malformed input - group with 1 base64 encode character";
            case 2:
                
                // fill accumulator with padding
                acc <<= 12;
                
                // check buffer for space
                if ( j >= sizeof buff )
                {
                    json += std :: string ( buff, j );
                    j = 0;
                }
                
                // set additional byte
                buff [ j + 0 ] = ( char ) ( acc >> 16 );
                
                j += 1;
                break;
            case 3:
                
                // fill accumulator with padding
                acc <<= 6;
                
                // check buffer for space
                if ( j + 1 >= sizeof buff )
                {
                    json += std :: string ( buff, j );
                    j = 0;
                }
                
                // set additional bytes
                buff [ j + 0 ] = ( char ) ( acc >> 16 );
                buff [ j + 1 ] = ( char ) ( acc >>  8 );
                
                j += 2;
                break;
            default:
                throw "2 - aaah!";
        }
        
        if ( j != 0 )
            json += std :: string ( buff, j );
        
        return json;
    }
    
} // namespace
