/*==============================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#ifndef _hpp_ncbi_jwx_
#define _hpp_ncbi_jwx_

#include <string>
#include <stdexcept>
#include <cstdarg>

/**
 * @file ncbi/jwx.hpp
 * @brief JSON Web Token Exceptions
 *
 *  A common exception family for this library's
 *  exception types.
 */

namespace ncbi
{
    /**
     * @class JWX
     * @brief base class for errors
     *
     *  Would be helpful to have explicit subclasses
     *  provide type compatibility with other exceptions
     *  within the std-namespace, however emphasis was given
     *  to being able to catch all library exceptions in a
     *  single clause. This means they all share a common base
     *  class, competing with the std::exception hierarchy.
     */
    class JWX : public std :: exception
    {
    public:

        /**
         * @fn what
         * @return const char * to exception report
         * may also include file and line number
         */
        virtual const char * what () const noexcept override;

        /**
         * @fn message
         * @return const char * with message
         * similar to what() but will never include file and line number
         */
        virtual const char * message () const noexcept;
        
        /**
         * @fn JWX
         * @overload vararg constructor
         * @param func_name the value of the __func__ macro
         * @param line the value of the __LINE__ macro
         * @param fmt a printf-compatible format string for dynamic message generation
         * note that the g++ format attribute is counting hidden "this" as param #1
         */
        explicit JWX ( const char * func_name, unsigned int line, const char * fmt, ... )
            __attribute__ ( ( format ( printf, 4, 5 ) ) );

        /**
         * @fn JWX
         * @overload std::string constructor
         * @param func_name the value of the __func__ macro
         * @param line the value of the __LINE__ macro
         * @param msg a pre-formatted message string
         */
        explicit JWX ( const char * func_name, unsigned int line, const std :: string & msg );

        /**
         * @fn ~JWX
         * @brief destroys message storage strings
         */
        virtual ~ JWX ();

    protected:

        /**
         * @fn JWX
         * @overload default constructor
         */
        JWX ();
        
        /**
         * @fn msgPrintf
         * @brief allows construction from a va_list
         * @param func_name the value of the __func__ macro
         * @param line the value of the __LINE__ macro
         * @param fmt a printf-compatible format string for dynamic message generation
         * @param args a va_list of format parameters
         */
        void msgPrintf ( const char * func_name, unsigned int line, const char * fmt, va_list args );
        
    private:

        void createWhatValue ( const char * func_name, unsigned int line );
        
        std :: string msg;                //!< contains static or dynamically generated message
        std :: string what_msg;           //!< contains full message for what() method
        const char * prepared_what_value; //!< pre-dereferenced return for what()
        const char * prepared_msg_value;  //!< pre-dereferenced return for message()
    };


#define DECLARE_JWX_MSG_EXCEPTION( class_name, super_class )                          \
    class class_name : public super_class                                             \
    {                                                                                 \
    public:                                                                           \
                                                                                      \
        explicit class_name ( const char * func_name, unsigned int line, const char * fmt, ... ) \
            __attribute__ ( ( format ( printf, 4, 5 ) ) )                             \
        {                                                                             \
            va_list args;                                                             \
            va_start ( args, fmt );                                                   \
            super_class :: msgPrintf ( func_name, line, fmt, args );                  \
            va_end ( args );                                                          \
        }                                                                             \
                                                                                      \
        explicit class_name ( const char * func_name, unsigned int line, const std :: string & msg ) \
            : super_class ( func_name, line, msg )                                    \
        {                                                                             \
        }                                                                             \
                                                                                      \
    protected:                                                                        \
                                                                                      \
        class_name () {}                                                              \
    }
}

#endif // _hpp_ncbi_jwx_
