/**
 * @file
 * This file contains the implementation of UserFunction, which is
 * used to hold user-defined functions.
 *
 * @brief Implementation of UserFunction
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-11-21, version 1.0
 *
 * $Id$
 */

#include "ValueRule.h"
#include "Frame.h"
#include "RbNames.h"
#include "RbString.h"
#include "VectorString.h"
#include "SyntaxElement.h"
#include "TypeSpec.h"
#include "UserFunction.h"

#include <list>
#include <sstream>

/** Basic constructor */
UserFunction::UserFunction( const ArgumentRules&        argRules,
                            const TypeSpec&             retType,
                            std::list<SyntaxElement*>*  stmts,
                            Frame*                      defineEnv)
    : RbFunction(), argumentRules(argRules), returnType(retType), code(stmts), defineEnvironment(defineEnv) {
}


/** Copy constructor */
UserFunction::UserFunction(const UserFunction &x)
    : RbFunction(x), argumentRules(x.argumentRules), returnType(x.returnType) {

    defineEnvironment   = x.defineEnvironment->clone();
    for (std::list<SyntaxElement*>::const_iterator i=x.code->begin(); i!=x.code->end(); i++)
        code->push_back((*i)->clone());
}


/** Destructor */
UserFunction::~UserFunction() {

    for (std::list<SyntaxElement*>::iterator i=code->begin(); i!=code->end(); i++)
        delete (*i);
    delete code;

    // defineEnvironment->destroyEnclosure();   //TODO: or something like that
    delete defineEnvironment;
}


/** Brief info on the function */
std::string UserFunction::briefInfo(void) const {

    std::ostringstream o;
    o << "UserFunction: ";
    printValue(o);

    return o.str();
}


/** Clone function */
UserFunction* UserFunction::clone(void) const {

    return new UserFunction(*this);
}


/** Execute function */
DAGNode* UserFunction::executeOperation(const std::vector<VariableSlot>& args) {

    std::cerr << "I am a user-defined function." << std::endl;
    std::cerr << "I know who I am but I do not know how to execute myself." << std::endl;

    //TODO: Execute code: call getvalue, maintain a call stack and a frame stack, watch for return signal

    return NULL;
}


/** Get argument rules */
const ArgumentRules& UserFunction::getArgumentRules() const {

    return argumentRules;
}


/** Get class vector describing type of object */
const VectorString& UserFunction::getClass() const {

    static VectorString rbClass = VectorString(UserFunction_name) + RbFunction::getClass();
    return rbClass;
}


/** Get return type */
const TypeSpec UserFunction::getReturnType() const {

    return returnType;
}


/** Complete info about object */
std::string UserFunction::richInfo() const {

    std::ostringstream o;
    o << "User-defined function:" << std::endl;
    o << "formals     = " << argumentRules.size() << " formal arguments" << std::endl;
    o << "return type = " << returnType << std::endl;
    o << "code        = " << code->size() << " lines of code" << std::endl;
    o << "definition environment:" << std::endl;
    defineEnvironment->printValue(o);

    return o.str();
}

