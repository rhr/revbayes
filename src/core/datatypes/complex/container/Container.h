/**
 * @file
 * This file contains the declaration of Container, a container type
 * that acts as a base class for all constant Containers.
 *
 * @brief Declaration of Container
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-12-04, version 1.0
 *
 * $Id$
 */

#ifndef Container_H
#define Container_H

#include "ConstantMemberObject.h"

#include <iostream>
#include <vector>


const std::string Container_name = "Container";

class Container : public ConstantMemberObject {
    
public:
    virtual                            ~Container(void);                                                    //!< Virtual destructor 
    
    // Basic utility functions you have to override
    virtual Container*                  clone(void) const = 0;                                              //!< Clone object
    virtual const VectorString&         getClass(void) const;                                               //!< Get class
    virtual void                        printValue(std::ostream& o) const = 0;                              //!< Print value for user
    
    // Container functions you have to override
    virtual void                        pop_back(void) = 0;                                                 //!< Drop element at back
    virtual void                        pop_front(void) = 0;                                                //!< Drop element from front
    virtual void                        push_back(RbObject* x) = 0;                                         //!< Append element to end
    virtual void                        push_front(RbObject* x) = 0;                                        //!< Add element in front
    virtual void                        resize(size_t n) = 0;                                               //!< Resize to new Container of length n
    virtual size_t                      size(void) const = 0;                                               //!< get the number of elements in the Container
    
    virtual void                        clear(void) = 0;                                                    //!< Clear
    virtual const RbObject&             getElement(size_t index) const = 0;                                 //!< Get element
    virtual RbObject&                   getElement(size_t index) = 0;                                       //!< Get element (non-const to return non-const element)
    virtual void                        setElement(const size_t index, RbObject* elem) = 0;                 //!< Set element with type conversion
    virtual void                        sort(void) = 0;                                                     //!< sort the Container
    virtual void                        unique(void) = 0;                                                   //!< removes consecutive duplicates
    
    // Member method inits
    virtual const MethodTable*          getMethods(void) const;                                             //!< Get methods

    // Container functions you do not have to override
    const TypeSpec&                     getElementType(void) const { return elementType; }                  //!< Get element type
    bool                                empty(void) const { return size() == 0; }                           //!< Test whether the container is empty
    bool                                supportsIndex(void) const { return true; }                          //!< Does object support index operator?
    
    
    
protected:
    Container(const TypeSpec &elemType);                                                                    //!< Set type spec of container from type of elements
    Container(const TypeSpec &elemType, const MemberRules* memberRules);                              //!< Set type spec of container from type of elements
    Container(const Container &v);                                                                          //!< Copy Constructor
    
    Container&                          operator=(const Container& x);                                          //!< Assignment operator

    virtual RbLanguageObject*           executeOperationSimple(const std::string& name, Environment* args);//!< Execute method

    // Member variables
    const TypeSpec                      elementType;                                                            //!< Type of elements
};

#endif

