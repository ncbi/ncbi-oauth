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

#ifndef _hpp_ncbi_oauth_json_
#define _hpp_ncbi_oauth_json_

#include <map>
#include <string>
#include <vector>
#include <stdexcept>


namespace ncbi
{
    class JSONFixture_JSONConstruction;
    class JSONTmpValue;
    
    /* JSONParseException
     **********************************************************************************/
    class JSONException : public std :: logic_error
    {
    public:
        explicit JSONException ( const char * function, unsigned int line, const char * message )
        : std :: logic_error ( message )
        , msg ( function )
        {
            msg += ":" + std :: to_string ( line ) + " - " + message;
            fl_msg = msg . c_str ();
        }
        
        virtual ~JSONException () throw ()
        { }
        
        virtual const char * what () const throw ()
        {
            return fl_msg;
        }
        
    private:
        std :: string msg;
        const char * fl_msg;
    };
    
    /* JSONValue interface
     **********************************************************************************/
    class JSONValue
    {
    public:
        JSONValue & operator [] ( int idx ) { return getValueByIndex ( idx ); }
        JSONValue & operator [] ( const char * mbr ) { return getValueByName ( mbr ); }
        JSONValue & operator [] ( const std :: string & mbr ) { return getValueByName ( mbr ); }

        JSONValue & operator = ( bool val ) { return setBooleanValue ( val ); }
        JSONValue & operator = ( int val ) { return setIntegerValue ( val ); }
        JSONValue & operator = ( long long int val ) { return setIntegerValue ( val ); }
        JSONValue & operator = ( double val ) { return setRealValue ( val ); }
        JSONValue & operator = ( long double val ) { return setRealValue ( val ); }
        JSONValue & operator = ( const char * val ) { return setPointerValue ( val ); }
        JSONValue & operator = ( const std :: string & val ) { return setStringValue ( val ); }
        
        const JSONValue & operator [] ( int idx ) const { return getConstValueByIndex ( idx ); }
        const JSONValue & operator [] ( const char * mbr ) const { return getConstValueByName ( mbr ); }
        const JSONValue & operator [] ( const std :: string & mbr ) const { return getConstValueByName ( mbr ); }

        virtual bool toBool () const;
        virtual long long toInt () const;
        virtual long double toReal () const;
        virtual std :: string toString () const;
        virtual std :: string toJSON () const = 0;
        
        operator bool () const { return toBool (); }
        operator long long () const { return toInt (); }
        operator long double () const { return toReal (); }
        operator std :: string () const { return toString (); }
        
        virtual ~JSONValue () {}
        
    protected:
        virtual JSONValue & getValueByIndex ( int idx );
        virtual JSONValue & getValueByName ( const std :: string & mbr );
        
        virtual JSONValue & setBooleanValue ( bool val );
        virtual JSONValue & setIntegerValue ( long long int val );
        virtual JSONValue & setRealValue ( long double val );
        virtual JSONValue & setStringValue ( const std :: string & val );
        virtual JSONValue & setToNull ();
        
        virtual const JSONValue & getConstValueByIndex ( int idx ) const;
        virtual const JSONValue & getConstValueByName ( const std :: string & mbr ) const;

        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
      
    private:
        JSONValue & setPointerValue ( const char * val );
        
        friend class JSONFixture_OperatorAssingments;
        friend class JSONFixture_JSONConstruction;
        friend class JSONTmpValue;
    };
        
    /* JSONArray 
     * array of JSONValues
     **********************************************************************************/
    class JSONArray : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONArray * make ( const std :: string & json );

        virtual std :: string toJSON () const;
        
        void append ( JSONValue * elem );
        
        JSONArray & operator = ( const JSONArray & array );
        
        JSONArray ( const JSONArray & a )
        {
            * this = a;
        }
        
        virtual ~JSONArray ();
        
    private:
        static JSONArray * parse ( const std :: string & json, size_t & offset );
        
        virtual JSONValue & getValueByIndex ( int idx );
        virtual const JSONValue & getConstValueByIndex ( int idx ) const;

        void clear ();

        JSONArray () {}
        
        std :: vector < JSONValue * > seq;
        
        friend class JSONValue;
        friend class JSONTmpValue;
        
    };
    
    /* JSONObject
     * map of key <string> / value <JSONValue*> pairs
     **********************************************************************************/
    class JSONObject : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONObject * make ( const std :: string & json );
        
        virtual std :: string toJSON () const;
        
        // retrieve a named value
        // TBD - these might be confusing with respect to the private accessors...
        JSONValue * getValue ( const std :: string & name );
        const JSONValue * getValue ( const std :: string & name ) const;
        
        // set an entry
        // if "is_final" is true, then this name cannot be modified after the fact
        void addMember ( const std :: string & name, JSONValue * value, bool is_final = false );
        
        // remove a named value
        // returns true if found and removed
        // throws an exception if found and final
        // returns false otherwise
        bool removeMember ( const std :: string & name );
        
        // return names/keys
        std :: vector < std :: string > getNames () const;
        
        JSONObject () {}
        virtual ~JSONObject ()
        {}
        
    private:
        static JSONObject * parse ( const std :: string & json, size_t & offset );
        
        virtual JSONValue & getValueByName ( const std :: string & mbr );
        virtual const JSONValue & getConstValueByName ( const std :: string & mbr ) const;
        

        // string is the key, pair ( is_final, val ) is the associated value
        // "val" can be NULL
        // if "is_final" is true, then entry cannot be deleted or overwritten
        std :: map < std :: string, std :: pair < bool, JSONValue * > > members;
        
        friend class JSONValue;
        friend class JSONTmpValue;
    };
    
    

}
#endif /* _hpp_ncbi_oauth_json_ */
