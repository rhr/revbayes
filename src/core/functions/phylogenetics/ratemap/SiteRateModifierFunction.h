//
//  SiteRateModifierFunction.h
//  revbayes-branch-proj
//
//  Created by Michael Landis on 2/23/17.
//  Copyright © 2017 Michael Landis. All rights reserved.
//

#ifndef SiteRateModifierFunction_h
#define SiteRateModifierFunction_h

#include "CharacterHistoryRateModifier.h"
#include "TypedDagNode.h"
#include "TypedFunction.h"

#include <iostream>
#include <vector>

namespace RevBayesCore {
    
    class SiteRateModifierFunction : public TypedFunction<CharacterHistoryRateModifier> {
        
    public:
        SiteRateModifierFunction(const TypedDagNode<RbVector<RbVector<double> > >* rm, const TypedDagNode<RbVector<RbVector<long> > >* ec, const TypedDagNode<RbVector<long> >* sc);
        SiteRateModifierFunction(const SiteRateModifierFunction& m);
        virtual ~SiteRateModifierFunction(void);                                                                                                  //!< Virtual destructor
        
        // public member functions
        SiteRateModifierFunction*                                           clone(void) const;                                                          //!< Create an independent clone
        void                                                                update(void);
        
    protected:
        void                                                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);            //!< Implementation of swaping parameters
        
    private:
        
        // members
        const TypedDagNode<RbVector<RbVector<double> > >*                   rate_multipliers;
        const TypedDagNode<RbVector<RbVector<long> > >*                     event_classes;
        const TypedDagNode<RbVector<long> >*                                site_classes;
        
    };
}

#endif /* SiteRateModifierFunction_h */
