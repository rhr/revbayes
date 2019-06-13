#ifndef FloorFunction_H
#define FloorFunction_H

#include "TypedFunction.h"
#include "TypedDagNode.h"

namespace RevBayesCore {
    
    /**
     * @brief Round down to the next integer.
     *
     * The floor function rounds a real number down to the next integer number.
     *
     */
    class FloorFunction : public TypedFunction<long> {
        
    public:
        FloorFunction(const TypedDagNode<double> *a);
        
        FloorFunction*                      clone(void) const;                                                  //!< Create a clon.
        void                                update(void);                                                       //!< Recompute the value
        
    protected:
        void                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);    //!< Implementation of swaping parameters
        
    private:
        const TypedDagNode<double>*         a;
    };
}

#endif
