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
#pragma warning "see if theres a better way to get access to JSONFixture"
    class JSONFixture;
    
    /* JSONParseException
     **********************************************************************************/
    class JSONException : public std :: logic_error
    {
    public:
        explicit JSONException ( const char * function, unsigned int line, const char * message )
        : logic_error ( message )
        , msg ( function )
        {
            msg += ":" + std :: to_string ( line );
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
        //typedef std :: string :: size_type size_t;
        
    public:
        // produces a string suitable for JSON output
        virtual std :: string toString () const = 0;
        
        JSONValue () {}
        virtual ~JSONValue () {}

    protected:
        // Parse/Factory constructor.
        static JSONValue * parse ( const std :: string & json, size_t & offset );
      
        friend class JSONFixture;
    };
        
    /* JSONArray 
     * array of JSONValues
     **********************************************************************************/
    class JSONArray : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONArray * parse ( const std :: string & json );
        
        virtual std :: string toString () const;
        
        void append ( JSONValue * elem );
        
        JSONArray & operator = ( const JSONArray & array );
        
        JSONArray ( const JSONArray & a )
        {
            * this = a;
        }
        
        JSONArray () {}
        virtual ~JSONArray ()
        {
            clear ();
        }
        
    private:
        static JSONArray * parse ( const std :: string & json, size_t & offset );
        void clear ();
        
        std :: vector < JSONValue * > seq;
        
        friend class JSONValue;
        
    };
    
    /* JSONObject
     * map of key <string> / value <JSONValue*> pairs
     **********************************************************************************/
    class JSONObject : public JSONValue
    {
    public:
        // Parse/Factory constructor.
        static JSONObject * parse ( const std :: string & json );
        
        // required behavior
        virtual std :: string toString () const;
        
        // retrieve a named value
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

        // string is the key, pair ( is_final, val ) is the associated value
        // "val" can be NULL
        // if "is_final" is true, then entry cannot be deleted or overwritten
        std :: map < std :: string, std :: pair < bool, JSONValue * > > members;
        mutable std :: atomic < bool > writable; // this should probably be a mutex
        
        friend class JSONValue;
    };
    
    

}
#endif /* _hpp_ncbi_oauth_json_ */
