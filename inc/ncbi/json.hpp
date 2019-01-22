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

#ifndef _hpp_ncbi_json_
#define _hpp_ncbi_json_

#ifndef _hpp_ncbi_jwx_
#include <ncbi/jwx.hpp>
#endif

#include <map>
#include <vector>

/**
 * @file ncbi/json.hpp
 * @brief JSON Object Model - RFC 7159
 *
 * JavaScript Object Notation is a textual
 * approach to encoding general purpose values
 * and structures. A C++ object model is more
 * useful when native C++ constructs are also
 * included, such as numbers and arrays.
 *
 * The object model presented herein is therefor
 * very C++ centric, while at the same time attempts
 * to retain a flavor that is portable to other languages.
 */

namespace ncbi
{
    class JSONValue;
    class JSONArray;
    class JSONObject;
    struct JSONString;
    struct JSONNumber;

    /**
     * @class JSON
     * @brief JavaScript Object Notation Management
     * globally accessible factory functions
     */
    class JSON
    {
    public:

        /**
         * @struct Limits
         * @brief a structure to hold parsing limit constants
         */
        struct Limits
        {
            /**
             * @fn Limits
             * @brief set default limits
             */
            Limits ();
            
            unsigned int json_string_size;    //!< total size of JSON string
            unsigned int recursion_depth;     //!< parser stack depth
            unsigned int numeral_length;      //!< maximum number of characters in number
            unsigned int string_size;         //!< maximum number of bytes in string
            unsigned int array_elem_count;    //!< maximum number of elements in array
            unsigned int object_mbr_count;    //!< maximum number of members in object
        };
        
        /**
         * @fn parse
         * @override parse JSON string using default Limits
         * @param json JSON text as described in RFC-7159
         * @exception MalformedJSON if text does not conform to standard
         * @exception JSONLimitViolation if source text exceeds established limits
         * @return JSONValue representing legal JSON source
         */
        static JSONValue * parse ( const std :: string & json );

        /**
         * @fn parse
         * @override parse JSON string using provided Limits
         * @param json JSON text as described in RFC-7159
         * @exception MalformedJSON if text does not conform to standard
         * @exception JSONLimitViolation if source text exceeds established limits
         * @return JSONValue representing legal JSON source
         */
        static JSONValue * parse ( const Limits & lim, const std :: string & json );

        /**
         * @fn parseObject
         * @override parse JSON string using default Limits
         * @param json JSON text as described in RFC-7159
         * @exception MalformedJSON if text does not conform to standard
         * @exception NotJSONObject if text does not represent a JSON object
         * @exception JSONLimitViolation if source text exceeds established limits
         * @return JSONObject representing legal JSON source
         */
        static JSONObject * parseObject ( const std :: string & json );

        /**
         * @fn parseObject
         * @override parse JSON string using provided Limits
         * @param json JSON text as described in RFC-7159
         * @exception MalformedJSON if text does not conform to standard
         * @exception NotJSONObject if text does not represent a JSON object
         * @exception JSONLimitViolation if source text exceeds established limits
         * @return JSONObject representing legal JSON source
         */
        static JSONObject * parseObject ( const Limits & lim, const std :: string & json );

        /**
         * @fn makeNull
         * @return a JSONValue representing JSON value keyword "null"
         */
        static JSONValue * makeNull ();

        /**
         * @fn makeBoolean
         * @brief creates a value from C++ bool
         * @param val a Boolean initialization value
         * @return a JSONValue representing a JSON value keyword "true" or "false"
         */
        static JSONValue * makeBoolean ( bool val );

        /**
         * @fn makeNumber
         * @brief creates a value from textual numeral
         * @param val a textual numeral initialization value
         * @return a JSONValue representing a numeric JSON value
         */
        static JSONValue * makeNumber ( const std :: string & val );

        /**
         * @fn makeInteger
         * @this is a specialization of a JSON number
         * @param val a long integer initialization value
         * @return a JSONValue representing an Integer JSON value
         * Use this method when starting with a C++ integer
         */
        static JSONValue * makeInteger ( long long int val );

        /**
         * @fn makeDouble
         * @this is a specialization of a JSON number
         * @param val an IEEE-754 long double initialization value
         * @return a JSONValue representing a floating point JSON value
         * Use this method ONLY when starting with a C++ double
         * Its use is not recommended due to loss of information
         * when converting to textual numeral.
         */
        static JSONValue * makeDouble ( long double val, unsigned int precision );

        /**
         * @fn makeString
         * @brief creates a value from std:: string
         * @param val a textual initialization value
         * @return a JSONValue representing a JSON string value
         */
        static JSONValue * makeString ( const std :: string & val );

        /**
         * @fn makeArray
         * @return an empty JSONArray
         */
        static JSONArray * makeArray ();

        /**
         * @fn makeObject
         * @return an empty JSONObject
         */
        static JSONObject * makeObject ();

    private:

        static JSONValue * parse ( const Limits & lim, const std :: string & json,
            size_t & pos, unsigned int depth );

        static JSONValue * parseNull ( const std :: string & json, size_t & pos );
        static JSONValue * parseBoolean ( const std :: string & json, size_t & pos );
        static JSONValue * parseNumber ( const Limits & lim,
            const std :: string & json, size_t & pos );
        static JSONValue * parseString ( const Limits & lim,
            const std :: string & json, size_t & pos );
        static JSONArray * parseArray ( const Limits & lim, const std :: string & json,
            size_t & pos, unsigned int depth );
        static JSONObject * parseObject ( const Limits & lim, const std :: string & json,
            size_t & pos, unsigned int depth );

        static JSONValue * makeParsedNumber ( const std :: string & val );
        static JSONValue * makeParsedString ( const std :: string & val );
        

        static Limits default_limits;
        
        // for testing
        static JSONValue * test_parse ( const std :: string & json, bool consume_all );
        friend class JSONFixture_WhiteBox;
    };


    /**
     * @class JSONValue
     * @brief an abstraction representing a value within a JSON entity
     * actual types are { null, Boolean, Integer, numeric-string, string, array, object }
     * predicate messages are defined to help determine the actual type
     */
    class JSONValue
    {
    public:

        /*=================================================*
         *                TYPE PREDICATES                  *
         *=================================================*/

        /**
         * @fn isNull
         * @return Boolean true if value is 'null'
         */
        virtual bool isNull () const;

        /**
         * @fn isBoolean
         * @return Boolean true if value is 'true' or 'false'
         */
        virtual bool isBoolean () const;

        /**
         * @fn isNumber
         * @return Boolean true if value is a number according to JSON spec
         */
        virtual bool isNumber () const;

        /**
         * @fn isInteger
         * @return Boolean true if value is specifically an Integer
         * an Integer is an integral number that can be represented
         * in a C++ long long int type.
         */
        virtual bool isInteger () const;

        /**
         * @fn isString
         * @return Boolean true if value is textual according to JSON spec
         */
        virtual bool isString () const;

        /**
         * @fn isArray
         * @return Boolean true if the value is an array
         * an array may contain other values
         */
        virtual bool isArray () const;

        /**
         * @fn isObject
         * @return Boolean true if the value is an object
         * an object may contain other values
         */
        virtual bool isObject () const;



        /*=================================================*
         *                 VALUE SETTERS                   *
         *=================================================*/

        /**
         * @fn setNull
         * @brief change type if necessary to null type with value 'null'
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual JSONValue & setNull ();

        /**
         * @fn setBoolean
         * @brief change type if necessary to Boolean and set value
         * @param val C++ bool true or false
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual JSONValue & setBoolean ( bool val );

        /**
         * @fn setNumber
         * @brief change type if necessary to Number and set value
         * @param val std::string numeric value representation
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual JSONValue & setNumber ( const std :: string & val );

        /**
         * @fn setInteger
         * @brief change type if necessary to Integer and set value
         * @param val C++ long long int
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual JSONValue & setInteger ( long long int val );

        /**
         * @fn setDouble
         * @brief change type if necessary to Number and set textual value
         * @param val C++ IEEE-754 long double to be converted to text
         * @return self reference as a convenience to aid in C++ expressions
         * It is recommended to use setNumber() instead due to loss of precision
         */
        virtual JSONValue & setDouble ( long double val, unsigned int precision );

        /**
         * @fn setString
         * @brief change type if necessary to String and set value
         * @param val std::string value representation
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual JSONValue & setString ( const std :: string & val );



        /*=================================================*
         *                 VALUE GETTERS                   *
         *=================================================*/

        /**
         * @fn toBoolean
         * @exception JSONIncompatibleType if value is not Boolean
         * @return C++ bool value of true or false
         */
        virtual bool toBoolean () const;

        /**
         * @fn toNumber
         * @exception JSONIncompatibleType if value is not numeric
         * @return C++ std::string representing numeral
         */
        virtual std :: string toNumber () const;

        /**
         * @fn toInteger
         * @exception JSONIncompatibleType if value is not representable as long long int
         * @return C++ long long int
         */
        virtual long long int toInteger () const;

        /**
         * @fn toString
         * @return C++ std::string with textual representation of value
         * differs from toJSON() in that actual string values will be
         * returned as is, in full UTF-8 UNICODE, with no escapes or quotes.
         */
        virtual std :: string toString () const;

        /**
         * @fn toJSON
         * @return C++ std::string with JSON representation of value
         * differs from toString() in that actual string values will
         * be quoted and escaped.
         */
        virtual std :: string toJSON () const = 0;

        /**
         * @fn readableJSON
         * @return C++ std::string with human-formatted JSON representation of value
         * differs from toJSON() in that spacing, indentation and line endings are inserted.
         */
        virtual std :: string readableJSON ( unsigned int indent = 0 ) const;



        /*=================================================*
         *                   TYPE CASTS                    *
         *=================================================*/

        /**
         * @fn toArray
         * @overload attempts to downcast from JSONValue to JSONArray
         * @exception JSONBadCast if value is not an array
         * @return JSONArray reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        virtual JSONArray & toArray ();

        /**
         * @fn toArray
         * @overload attempts to downcast from const JSONValue to const JSONArray
         * @exception JSONBadCast if value is not an array
         * @return JSONArray reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        virtual const JSONArray & toArray () const;

        /**
         * @fn toObject
         * @overload attempts to downcast from JSONValue to JSONObject
         * @exception JSONBadCast if value is not an object
         * @return JSONObject reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        virtual JSONObject & toObject ();

        /**
         * @fn toObject
         * @overload attempts to downcast from const JSONValue to const JSONObject
         * @exception JSONBadCast if value is not an object
         * @return JSONObject reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        virtual const JSONObject & toObject () const;

        /**
         * @fn clone
         * @return creates a deep copy of value
         */
        virtual JSONValue * clone () const;

        /**
         * @fn invalidate
         * @brief overwrite potentially sensitive contents in memory
         */
        virtual void invalidate () = 0;
        
        /**
         * @fn !JSONValue
         * @brief disposes of dynmically allocated content in derived classes
         */
        virtual ~JSONValue ();

    protected:
        
        JSONValue ();
    };
        

    /**
     * @class JSONArray
     * @brief an array of zero or more JSONValues
     * extends JSONValue to define some behavior
     * and to provide access via container interface.
     */
    class JSONArray : public JSONValue
    {
    public:

        /*=================================================*
         *                   OVERRIDES                     *
         *=================================================*/

        // type predicate
        virtual bool isArray () const override
        { return true; }

        // getters
        virtual std :: string toString () const override;
        virtual std :: string toJSON () const override;
        virtual std :: string readableJSON ( unsigned int indent = 0 ) const override;

        // casts
        virtual JSONArray & toArray () override
        { return * this; }
        virtual const JSONArray & toArray () const override
        { return * this; }

        // clone and invalidate
        virtual JSONValue * clone () const override;
        virtual void invalidate () override;


        /*=================================================*
         *             CONTAINER INTERFACE                 *
         *=================================================*/

        /**
         * @fn isEmpty
         * @return Boolean true if array has no elements
         */
        bool isEmpty () const;

        /**
         * @fn count
         * @return Natural number with the number of contained elements
         */
        unsigned long int count () const;

        /**
         * @fn exists
         * @brief answers whether the indicated element exists
         * @param idx signed array index
         * @return Boolean true if element exists
         */
        bool exists ( long int idx ) const;

        /**
         * @fn appendValue
         * @brief add a new element to end of array
         * @exception JSONNullValue if elem == nullptr
         */
        void appendValue ( JSONValue * elem );

        /**
         * @fn setValue
         * @brief set entry to a new value
         * @param idx signed array index
         * @param elem a JSONValue to store within array
         * @exception JSONIndexOutOfBounds if idx < 0
         * @exception JSONNullValue if elem == nullptr
         * if idx >= count (), the array will be extended
         * and missing elements with index < idx will be
         * filled with null values.
         */
        void setValue ( long int idx, JSONValue * elem );

        /**
         * @fn getValue
         * @overload returns non-const JSONValue at idx
         * @param idx signed array index
         * @exception JSONIndexOutOfBounds if idx < 0
         * @return non-const JSONValue reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        JSONValue & getValue ( long int idx );

        /**
         * @fn getValue
         * @overload returns const JSONValue at idx
         * @param idx signed array index
         * @exception JSONIndexOutOfBounds if idx < 0
         * @return const JSONValue reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        const JSONValue & getValue ( long int idx ) const;

        /**
         * @fn removeValue
         * @brief remove and return an entry if valid
         * @param idx signed array index
         * @return nullptr if element does not exist or valid JSONValue pointer
         * replaces valid internal entries with null element
         * deletes trailing null elements making them undefined
         */
        JSONValue * removeValue ( long int idx );
        
        /**
         * @fn lock
         * @brief lock the array against future modification
         */
        void lock ();

        /**
         * @fn operator =
         * @brief assignment operator
         * @param array source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         * will delete any current contents
         * clones contents of source array.
         */
        JSONArray & operator = ( const JSONArray & array );

        /**
         * @fn JSONArray
         * @override copy constructor
         * @param array source of contents to clone
         * clones contents of source array.
         */
        JSONArray ( const JSONArray & a );

        /**
         * @fn ~JSONArray
         * @brief deletes any contents and destroys internal structures
         */        
        virtual ~JSONArray () override;
        
    private:
        
        // used to empty out the array before copy
        void clear ();

        JSONArray ();

        std :: vector < JSONValue * > array;
        bool locked;
        
        friend class JSON;
    };
    

    /**
     * @class JSONObject
     * @brief a map of zero or more ( name, value ) pairs
     * extends JSONValue to define some behavior
     * and to provide access via container interface.
     */
    class JSONObject : public JSONValue
    {
    public:
        
        /*=================================================*
         *                   OVERRIDES                     *
         *=================================================*/

        // type predicate
        virtual bool isObject () const override
        { return true; }

        // getters
        virtual std :: string toString () const override;
        virtual std :: string toJSON () const override;
        virtual std :: string readableJSON ( unsigned int indent = 0 ) const override;

        // casts
        virtual JSONObject & toObject () override
        { return * this; }
        virtual const JSONObject & toObject () const override
        { return * this; }

        // clone and invalidate
        virtual JSONValue * clone () const override;
        virtual void invalidate () override;


        /*=================================================*
         *             CONTAINER INTERFACE                 *
         *=================================================*/

        /**
         * @fn isEmpty
         * @return Boolean true if object has no ( name, value ) pairs
         */
        bool isEmpty () const;

        /**
         * @fn count
         * @return Natural number with the number of member pairs
         */
        unsigned long int count () const;

        /**
         * @fn exists
         * @brief answers whether the indicated mapping exists
         * @param name std::string with the member name
         * @return Boolean true if pair exists
         */
        bool exists ( const std :: string & name ) const;
        
        /**
         * @fn getNames
         * @return std::vector<std::string> of member names
         */
        std :: vector < std :: string > getNames () const;

        /**
         * @fn addNameValuePair
         * @brief add a new ( name, value ) pair
         * @param name std::string with unique member name
         * @param val a non-null JSONValue pointer
         * @exception JSONUniqueConstraintViolation if name exists
         * @exception JSONNullValue if val == nullptr
         */
        void addNameValuePair ( const std :: string & name, JSONValue * val );

        /**
         * @fn addFinalNameValuePair
         * @brief add a new ( name, value ) pair and make element immutable
         * @param name std::string with unique member name
         * @param val a non-null JSONValue pointer
         * @exception JSONUniqueConstraintViolation if name exists
         * @exception JSONNullValue if val == nullptr
         */
        void addFinalNameValuePair ( const std :: string & name, JSONValue * val );
        
        /**
         * @fn setValue
         * @brief set value of an existing pair or add a new pair
         * @param name std::string with unique member name
         * @param val a non-null JSONValue pointer
         * @exception JSONPermViolation if member exists and is final
         * @exception JSONNullValue if val == nullptr
         */
        void setValue ( const std :: string & name, JSONValue * val );

        /**
         * @fn setValueOrDelete
         * @brief calls setValue() and deletes val upon exceptions
         * @param name std::string with unique member name
         * @param val a non-null JSONValue pointer
         * @exception JSONPermViolation if member exists and is final
         * @exception JSONNullValue if val == nullptr
         * In C++, the temptation is to create JSONValue objects inline
         * to the setValue expression. When using setValue() directly,
         * this can lead to orphaned objects. Use this method to
         * indicate that the value object should be deleted upon error.
         */
        void setValueOrDelete ( const std :: string & name, JSONValue * val );

        /**
         * @fn setFinalValue
         * @brief set value of an existing pair or add a new pair and mark member as final
         * @param name std::string with unique member name
         * @param val a non-null JSONValue pointer
         * @exception JSONPermViolation if member exists and is final
         * @exception JSONNullValue if val == nullptr
         */
        void setFinalValue ( const std :: string & name, JSONValue * val );

        /**
         * @fn setFinalValueOrDelete
         * @brief calls setFinalValue() and deletes val upon exceptions
         * @param name std::string with unique member name
         * @param val a non-null JSONValue pointer
         * @exception JSONPermViolation if member exists and is final
         * @exception JSONNullValue if val == nullptr
         * In C++, the temptation is to create JSONValue objects inline
         * to the setFinalValue expression. When using setFinalValue()
         * directly, this can lead to orphaned objects. Use this method to
         * indicate that the value object should be deleted upon error.
         */
        void setFinalValueOrDelete ( const std :: string & name, JSONValue * val );

        /**
         * @fn getValue
         * @override non-const accessor
         * @param name std::string with the member name
         * @exception JSONNoSuchName if name is not a member of object
         * @return JSONValue reference to existing value
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        JSONValue & getValue ( const std :: string & name );

        /**
         * @fn getValue
         * @override const accessor
         * @param name std::string with the member name
         * @exception JSONNoSuchName if name is not a member of object
         * @return const JSONValue reference to existing value
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        const JSONValue & getValue ( const std :: string & name ) const;
        
        /**
         * @fn removeValue
         * @brief remove and delete named value
         * @param name std:: string with member name
         * @exception JSONPermViolation if member exists and is final
         * ignored if member is not found
         */
        void removeValue ( const std :: string & name );
        
        /**
         * @fn lock
         * @brief lock the object against future modification
         */
        void lock ();

        /**
         * @fn operator =
         * @brief assignment operator
         * @param obj source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         * will delete any current contents
         * clones contents of source object.
         */
        JSONObject & operator = ( const JSONObject & obj );

        /**
         * @fn JSONObject
         * @override copy constructor
         * @param obj source of contents to clone
         * clones contents of source object.
         */
        JSONObject ( const JSONObject & obj );

        /**
         * @fn ~JSONObject
         * @brief deletes any contents and destroys internal structures
         */        
        virtual ~JSONObject () override;

        
    private:

        void clear ();
        
        JSONObject ();

        std :: map < std :: string, std :: pair < bool, JSONValue * > > members;
        bool locked;
        
        friend class JSON;
    };


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( JSONException, JWX );
    DECLARE_JWX_MSG_EXCEPTION ( MalformedJSON, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( NotJSONObject, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONLimitViolation, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONIncompatibleType, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONBadCast, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONNullValue, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONIndexOutOfBounds, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONUniqueConstraintViolation, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONPermViolation, JSONException );
    DECLARE_JWX_MSG_EXCEPTION ( JSONNoSuchName, JSONException );

}
#endif /* _hpp_ncbi_json_ */
