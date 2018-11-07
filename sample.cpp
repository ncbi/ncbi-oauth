#include <ncbi/json.hpp>
#include <ncbi/jwt.hpp>
#include <ncbi/jws.hpp>
#include <ncbi/jwk.hpp>

#include <iostream>

using namespace ncbi;

static void do_other_stuff ( JWTFactory & jwt_fact );
static void do_even_more_stuff ( const JWTFactory & jwt_fact );

static const char sample_pem [] = "";
static const char sample_pem_pwd [] = "";

int main ( int argc, char * argv [], char * envp [] )
{
    try
    {
        /* STARTING POINT

           For this example, the starting point is the signing key.
           A key is either randomly created, or is pre-existing.
           A server will have pre-existing keys and will need to
           obtain a JWK representing its signing authority.

           Here we will use a string of PEM text representing a
           private RSA key. Of course, this is an absurd way to manage
           keys, as there is no protection of the key contents.

           As soon as possible, we'll provide a service for
           storing/retrieving keys, both public and private. The
           native format will be JWK, NOT PEM or DER. Notice that
           JWK stores "use", "alg" and importantly "kid". In this
           example, since we're creating a key manually from PEM,
           we will have to manually provide these properties.

        */
        const char signing_key_use [] = "sig";
        const char signing_key_alg [] = "RS256";
        const char signing_key_kid [] = "key-id-1234";
        const JWK * signing_key = JWK :: parsePEM ( sample_pem, sample_pem_pwd,
            signing_key_use, signing_key_alg, signing_key_kid );

        /* NEXT STEP

           When creating simple JWTs that only bear a signature
           (i.e. not encrypted), first prepare a signature factory
           object. This is an object that is used for the
           actual signing and signature verification process.

           A JWSFactory is created with the signing key, and a name
           representing the authority for logging purposes. Since a
           JWK is not (yet) required to carry the algorithm, so we
           are supplying it again. This is likely to change.
          
           NB - the intention of this object is that it will live for as long
           as it is needed. It is intended to be declared in "main()" or high
           up in the call chain and to live for the duration of the process.

         */
        const char signing_authority [] = "NCBI";
        JWSFactory jws_fact ( signing_authority, signing_key_alg, signing_key );

        // can (and should) dump our key reference now
        signing_key -> release ();
        
        /* CREATE JWT FACTORY

           now that we have JWS, which is the ability to sign and verify signatures,
           we need the factory object for JWT. It is created with the JWS factory
           so that it acquires the ability to sign/verify JWTs.
          
           NB - the "jws_fact" is only captured as a REFERENCE within the "jwt_fact"
           so the jws_fact must outlive the jwt_fact. Don't let it go away first!
          
           NB - in the future, we will add another constructor that allows association
           of a JWE factory for encryption.
          
           NB - the lifetime of "jwt_fact" is intended to be for the entire process,
           just as with the "jws_fact".

        */
        JWTFactory jwt_fact ( jws_fact );
        
        // the "jwt_fact" is now essentially standalone
        // and can be passed to other functions.
        do_other_stuff ( jwt_fact );
    }
    catch ( JSONException & x )
    {
        std :: cerr
            << "JSON exception: '"
            << x . what ()
            << "'\n"
            ;
        return 1;
    }
    catch ( JWTException & x )
    {
        std :: cerr
            << "JWT exception: '"
            << x . what ()
            << "'\n"
            ;
        return 2;
    }
    catch ( ... )
    {
        // don't propagate exceptions
        std :: cerr
            << "unknown exception\n"
            ;
        return 3;
    }

    return 0;
}

static void do_other_stuff ( JWTFactory & jwt_fact )
{
    // the factory can be pre-populated with some reserved attributes,
    //  e.g. the issuer
    // see <ncbi/hwt.hpp> for more registered claims
    jwt_fact . setIssuer ( "https://ncbi.nlm.nih.gov" );

    // duration MUST be set as of right now
    // it is a number that must be > 0
    // and is used to create an expiration timeout
    jwt_fact . setDuration ( 15 );

    // here is an interesting one - the "skew" is an attribute
    // that is used when verifying JWTs, and it attempts to account for
    // potential clock skew between the issuing host and the verifying host
    // it must be >= 0. The constructor sets this to 0.
    jwt_fact . setDefaultSkew ( 5 );

    // when configuration is complete and you will never need
    // to update it again, you can lock the factory against change
    jwt_fact . lock ();

    // so now, the JWT factory should be set from configuration
    do_even_more_stuff ( jwt_fact );
}

static void do_even_more_stuff ( const JWTFactory & jwt_fact )
{
    /* THIS is the point at which you start your operation.
       Now, you can create and verify and use JWT objects. */

    // you start with a "claims" object
    JWTClaims claims1 = jwt_fact . make ();

    // the claims can be populated with registered claims individually
    // but each object has already been populated with those claims
    // stored on the factory, and these cannot be reset.
    claims1 . setSubject ( "https://some.subject.com" );

    // there is a "not-before" property that allows you to create
    // a JWT in advance of when it can be used. Not sure how easy
    // it is to synchronize this, but it essentially moves the lower
    // bound of the time window for the JWT's validity
    claims1 . setNotBefore ( 2 );

    /* NB - the current interface gives this as a delta from time
       of creation. The use case may require an absolute time. */

    // add more claims
    claims1 . addClaim ( "hello", JSONValue :: makeString ( "there!" ) );

    /* NB - the simple "addClaim()" is an interface to the fundamental
       operation of adding a claim, but it is difficult to use in a way
       that doesn't leak memory if there are exceptions thrown.

       The usage shown above is an example of WHAT NOT TO DO, because
       if you tried to set a reserved claim, e.g. "iat", you'd get
       an exception thrown and the allocation of the JSONValue* would
       be orphaned/leaked.

       For convenience and good programming practice, you should use
       the version given in the next statement. It will catch exceptions
       and delete your inline allocation for you. */
    claims1 . addClaimOrDeleteValue ( "how", JSONValue :: makeString ( "are you?" ) );

    /* JSONValue
       You will have noticed the use of the class JSONValue for creating
       claims values. Look at <ncbi/json.hpp> for more details. The types
       of values you can create are

       null, bool, integer, floating-point real, numeric string,
       and simple string.

       NB - It's best to use "makeDouble()" ONLY when your starting point
       is a double ( or long double ). If you have it in text form, it will
       be preserved in text form via "makeNumber()". */
    claims1 . addClaimOrDeleteValue ( "a bool", JSONValue :: makeBool ( true ) );
    claims1 . addClaimOrDeleteValue ( "a machine-word integer", JSONValue :: makeInteger ( -1234567 ) );
    claims1 . addClaimOrDeleteValue ( "any numeric number", JSONValue :: makeNumber ( "123.4567" ) );

    /* If you are adding an array, you FIRST set the claim value to
       "JSONArray :: make ()" and then you can use "appendValue()" on the
       array to insert your items. */
    claims1 . addClaimOrDeleteValue ( "array1", JSONArray :: make () );
    claims1 . getClaim ( "array1" ) . toArray () . appendValue ( JSONValue :: makeBool ( false ) );

    // another, perhaps preferred way would be to capture the array value first
    JSONArray * array2 = JSONArray :: make ();
    claims1 . addClaim ( "array2", array2 );
    array2 -> appendValue ( JSONValue :: makeNumber ( "13" ) );

    /* You can similarly add a JSON object */
    JSONObject * obj1 = JSONObject :: make ();
    claims1 . addClaim ( "object1", obj1 );
    obj1 -> setValue ( "member", JSONValue :: makeString ( "value" ) );

    /*

      AFTER YOUR CLAIM IS BUILT,
      you can now convert it to a signed JWT.
      A JWT is just a std::string, but has been typedef'd
      to conform to the descriptions given in the RFCs.

    */
    JWT jwt = jwt_fact . sign ( claims1 );

    std :: cout
        << "Look at my shiny new JWT:\n"
        << "  "
        << jwt
        << '\n'
        ;

    /* This JWT is a compact serialization using JWS.
       We can look at the parts that are separated by '.'
     */
    std :: vector < std :: string > parts;
    size_t start = 0;
    for ( size_t dot = jwt . find ( ".", 0 );
          dot != std :: string :: npos;
          dot = jwt . find ( ".", start = dot + 1 ) )
    {
        parts . push_back ( jwt . substr ( start, dot - start ) );
    }
    parts . push_back ( jwt . substr ( start ) );

    std :: cout
        << "Break it up into sections:\n"
        << "  "
        << parts [ 0 ]
        << '\n'
        ;
    for ( size_t part = 1; part < parts . size (); ++ part )
    {
        std :: cout
            << "  .\n"
            << "  "
            << parts [ part ]
            << '\n'
            ;
    }

    /* You should now be able to paste the JWT into the little
       page at https://jwt.io to show its contents and validate
       the signature. Notice that the timestamp of creation is
       included in the final claims, so you'll get a different
       signature every time. */

    // decode the JWT
    JWTClaims claims2 = jwt_fact . decode ( jwt );

    // NB - if you are single-stepping through this, there's a
    // chance that "decode()" will fail since we only set a 15
    // second duration for the object.

    /* If you arrived HERE, then the "jwt" passed validation */
    std :: cout
        << "And here's what was recovered from JWT:\n"
        << claims2 . readableJSON ( 1 )
        << '\n'
        ;
}
