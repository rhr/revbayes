/**
 * @file
 * This file contains the implementation of Func_ls, which is
 * the function used to list the content of the workspace.
 *
 * @brief Implementation of Func_ls
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @interface RbFunction
 * @package functions
 * @since Version 1.0, 2009-09-03
 *
 * $Id$
 */

#include "Boolean.h"
#include "DAGNode.h"
#include "Func_ls.h"
#include "Model.h"
#include "Parser.h"
#include "RbException.h"
#include "RbNames.h"
#include "RbString.h"
#include "TypeSpec.h"
#include "ValueRule.h"
#include "VectorString.h"
#include "Workspace.h"

#include <fstream>


/** Clone object */
Func_ls* Func_ls::clone( void ) const {

    return new Func_ls( *this );
}


/** Execute function */
DAGNode* Func_ls::executeFunction( void ) {

    /* Open file */
    const Boolean *printAll = static_cast<const Boolean*>( args[0].getValue() );
    
    std::cout << "User workspace:" << std::endl;
    std::cout << "===============" << std::endl;
    std::cout << std::endl;

    Workspace::userWorkspace().printValue( std::cout );
    std::cout << std::endl;
    
    if ( *printAll == true ) {
        
        std::cout << "Global workspace:" << std::endl;
        std::cout << "===============" << std::endl;
        std::cout << std::endl;

        Workspace::globalWorkspace().printValue( std::cout );
        std::cout << std::endl;
    }

    return NULL;
}


/** Get argument rules */
const ArgumentRules& Func_ls::getArgumentRules( void ) const {

    static ArgumentRules argumentRules;
    static bool          rulesSet = false;

    if ( !rulesSet ) {

        argumentRules.push_back( new ValueRule( "all", new Boolean( false ) ) );
        rulesSet = true;
    }

    return argumentRules;
}


/** Get class vector describing type of object */
const VectorString& Func_ls::getClass( void ) const {

    static VectorString rbClass = VectorString( Func_ls_name ) + RbFunction::getClass();
    return rbClass;
}


/** Get return type */
const TypeSpec Func_ls::getReturnType( void ) const {

    return TypeSpec( RbVoid_name );
}

