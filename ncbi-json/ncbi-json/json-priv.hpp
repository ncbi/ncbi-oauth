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

#ifndef _hpp_ncbi_oauth_json_priv_
#define _hpp_ncbi_oauth_json_priv_

#include "json.hpp"

namespace ncbi
{
    /* JSONTmpValue
     * a temporary value that is used in the operator [] overloading for assignments
     **********************************************************************************/
    class JSONTmpValue : public JSONValue
    {
    public:
        virtual std :: string toJSON () const;
        
        JSONTmpValue ( JSONValue *parent, int index );
        ~JSONTmpValue ();
        
    private:
        JSONValue *parent;
        int index;
    };
    
    /* JSONNullValue
     **********************************************************************************/
    class JSONNullValue : public JSONValue
    {
    public:
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual std :: string toJSON () const
        { return "null"; }
        virtual std :: string toString () const
        { return toJSON (); }
    };
    
    
    /* JSONTypedValue
     * template types: <bool> <int> <float> <string>
     **********************************************************************************/
    template < class T >
    class JSONTypedValue : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual std :: string toJSON () const;
        virtual std :: string toString () const
        { return toJSON (); }
        
        T getValue ()
        {
            return val;
        }
        
        const T getValue () const
        {
            return val;
        }
        
        // TBD - keep?
        void setValue ( const T & value )
        {
            val = value;
        }
        
        // Constructors
        JSONTypedValue ( const T & value )
        : val ( value )
        {}
        
        JSONTypedValue ( const JSONTypedValue < T > & copy )
        : val ( copy . val )
        {}
        
        JSONTypedValue < T > & operator = ( const JSONTypedValue < T > & orig )
        {
            val = orig . val;
            return *this;
        }
        
        virtual ~JSONTypedValue ()
        {}
        
        
    private:
        
        T val;
    };
} // ncbi

#endif /* _hpp_ncbi_oauth_json_priv_ */
