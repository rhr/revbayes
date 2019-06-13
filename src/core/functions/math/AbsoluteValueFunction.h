#ifndef AbsoluteValueFunction_H
#define AbsoluteValueFunction_H

#include "ContinuousFunction.h"
#include "TypedDagNode.h"

namespace RevBayesCore {
    
    
    /**
     * @brief Absolute value of a real number.
     *
     * The absolute value computes the value without sign.
     * Hence, the return value y = x if x >= 0 and y = -x if x < 0.
     *
     */
    class AbsoluteValueFunction : public ContinuousFunction {
        
    public:
        AbsoluteValueFunction(const TypedDagNode<double> *a);
        
        AbsoluteValueFunction*              clone(void) const;                                                  //!< Create a clon.
        void                                update(void);                                                       //!< Recompute the value
        
    protected:
        void                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);    //!< Implementation of swapping parameters
        
    private:
        const TypedDagNode<double>*         a;
    };
}

#endif
