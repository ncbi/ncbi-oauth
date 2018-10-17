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

#include <ncbi/jwt.hpp>

namespace ncbi
{
    const char * JWTException :: what () const
        throw ()
    {
        return fl_msg;
    }
        
    JWTException :: JWTException ( const char * function, unsigned int line, const char * message )
        : std :: logic_error ( message )
        , msg ( function )
    {
        msg += ":" + std :: to_string ( line ) + " - " + message;
        fl_msg = msg . c_str ();
    }
    
    JWTException :: ~JWTException ()
        throw ()
    {
        fl_msg = "<DELETED>";
    }
}
