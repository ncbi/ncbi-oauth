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

#include <ncbi/json.hpp>

namespace ncbi
{
    /* JSONTmpValue
     * a temporary value that is used in the operator [] overloading for assignments
     **********************************************************************************/
    class JSONTmpValue : public JSONValue
    {
    public:
        // behave as keyword null
        virtual std :: string toJSON () const;

        // we are an array element
        JSONTmpValue ( JSONArray *parent, int index );
        // we are an object member
        JSONTmpValue ( JSONObject *parent, const std :: string & mbr );
        virtual ~JSONTmpValue () {}
        
    private:
        virtual JSONValue & getValueByIndex ( int idx );
        virtual JSONValue & getValueByName ( const std :: string & mbr );
        virtual JSONValue & setBooleanValue ( bool val );
        virtual JSONValue & setIntegerValue ( long long int val );
        virtual JSONValue & setRealValue ( long double val );
        virtual JSONValue & setStringValue ( const std :: string & val );
        virtual JSONValue & setToNull ();
        
        JSONValue & replaceSelf ( JSONValue * val );
        
        JSONValue *parent;
        std :: string mbr;
        int index;
    };
    
    /* JSONNullValue
     **********************************************************************************/
    class JSONNullValue : public JSONValue
    {
    public:
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual std :: string toJSON () const;
        virtual JSONValue & setToNull ();
    };
    
    class JSONBoolValue : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual JSONValue & setBooleanValue ( bool val );
        virtual bool toBool () const;
        virtual std :: string toJSON () const;
        
        
        JSONBoolValue ( bool value );
        JSONBoolValue ( const JSONBoolValue & copy );
        JSONBoolValue & operator = ( const JSONBoolValue & orig );
        virtual ~JSONBoolValue () {}
        
    private:
        bool value;

    };
    
    class JSONIntegerValue : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual JSONValue & setIntegerValue ( long long int val );
        virtual long long toInt () const;
        virtual long double toReal () const;
        virtual std :: string toJSON () const;
        
        JSONIntegerValue ( long long int value );
        JSONIntegerValue ( const JSONIntegerValue & copy );
        JSONIntegerValue & operator = ( const JSONIntegerValue & orig );
        virtual ~JSONIntegerValue () {}
        
    private:
        long long int value;

    };
    
    class JSONRealValue : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual JSONValue & setRealValue ( long double val );
        virtual long long toInt () const;
        virtual long double toReal () const;
        virtual std :: string toJSON () const;
        
        JSONRealValue ( long double value );
        JSONRealValue ( const std :: string & value );
        JSONRealValue ( const JSONRealValue & copy );
        JSONRealValue & operator = ( const JSONRealValue & orig );
        virtual ~JSONRealValue () {}
        
    private:
        std :: string value;

    };
    
    class JSONStringValue : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
        
        virtual bool toBool () const;
        virtual long long toInt () const;
        virtual long double toReal () const;
        virtual std :: string toString () const;
        virtual std :: string toJSON () const;
        virtual JSONValue & setStringValue ( const std :: string & val );
        
        JSONStringValue ( const std :: string & value );
        JSONStringValue ( const JSONStringValue & copy );
        JSONStringValue & operator = ( const JSONStringValue & orig );
        virtual ~JSONStringValue () {}
        
    private:
        std :: string value;
    };

} // ncbi

#endif /* _hpp_ncbi_oauth_json_priv_ */
