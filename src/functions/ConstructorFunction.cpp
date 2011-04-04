/**
 * @file
 * This file contains the implementation of ConstructorFunction, which is used
 * for functions that construct member objects.
 *
 * @brief Implementation of ConstructorFunction
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes core team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-09-17, version 1.0
 *
 * $Id$
 */

#include "ArgumentRule.h"
#include "ConstructorFunction.h"
#include "DAGNode.h"
#include "MemberNode.h"
#include "MemberObject.h"
#include "RbNames.h"
#include "TypeSpec.h"
#include "VectorString.h"

#include <sstream>


/** Constructor */
ConstructorFunction::ConstructorFunction(MemberObject* obj) 
    : RbFunction(), templateObject(obj) {
}


/** Clone the object */
ConstructorFunction* ConstructorFunction::clone(void) const {

    return new ConstructorFunction(*this);
}


/** Execute operation: we reset our template object here */
DAGNode* ConstructorFunction::executeOperation(const std::vector<VariableSlot>& args) {

    const ArgumentRules& argRules = getArgumentRules();

    MemberObject* copy = templateObject->clone();

    ArgumentRules::const_iterator i;
    std::vector<VariableSlot>::const_iterator j;
    for (i=argRules.begin(), j=args.begin(); i!=argRules.end(); i++, j++) {
        if (copy->getMemberTypeSpec((*i)->getArgLabel()).isReference())
            copy->setVariable((*i)->getArgLabel(), (*j).getReference());
        else
            copy->setValue((*i)->getArgLabel(), (*j).getValue()->clone());
    }
 
    return new MemberNode(copy);
}


/** Get argument rules */
const ArgumentRules& ConstructorFunction::getArgumentRules(void) const {

    return templateObject->getMemberRules();
}


/** Get class vector describing type of object */
const VectorString& ConstructorFunction::getClass(void) const { 

    static VectorString rbClass = VectorString(ConstructorFunction_name) + RbFunction::getClass();
	return rbClass; 
}


/** Get return type */
const TypeSpec ConstructorFunction::getReturnType(void) const {

    return templateObject->getTypeSpec();
}

