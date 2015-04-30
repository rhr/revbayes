//
//  Move_LevyJumpSum.h
//  revbayes-proj
//
//  Created by Michael Landis on 4/4/15.
//  Copyright (c) 2015 Michael Landis. All rights reserved.
//

#ifndef __revbayes_proj__Move_LevyJumpSum__
#define __revbayes_proj__Move_LevyJumpSum__

#include "SimpleMove.h"
#include "RlMove.h"
#include "TypedDagNode.h"

#include <ostream>
#include <string>

namespace RevLanguage {
    
    class Move_LevyJumpSum : public Move {
        
    public:
        
        Move_LevyJumpSum(void);                                                                                      //!< Default constructor (0.0)
        
        // Basic utility functions
        virtual Move_LevyJumpSum*                  clone(void) const;                                                      //!< Clone object
        void                                        constructInternalObject(void);                                          //!< We construct the a new internal SlidingMove.
        static const std::string&                   getClassType(void);                                                     //!< Get Rev type
        static const TypeSpec&                      getClassTypeSpec(void);                                                 //!< Get class type spec
        const MemberRules&                          getParameterRules(void) const;                                             //!< Get member rules (const)
        virtual const TypeSpec&                     getTypeSpec(void) const;                                                //!< Get language type of the object
        virtual void                                printValue(std::ostream& o) const;                                      //!< Print value (for user)
        
    protected:
        
        void                                        setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);     //!< Set member variable
        
        RevPtr<const RevVariable>                      slideUp;
        RevPtr<const RevVariable>                      slideDown;
        RevPtr<const RevVariable>                      slideFactor;
        RevPtr<const RevVariable>                      weight;
        RevPtr<const RevVariable>                      tune;
        
    };
    
}

#endif /* defined(__revbayes_proj__Move_LevyJumpSum__) */
