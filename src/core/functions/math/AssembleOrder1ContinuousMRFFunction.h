#ifndef AssembleOrder1ContinuousMRFFunction_H
#define AssembleOrder1ContinuousMRFFunction_H

#include "RbVector.h"
#include "TypedDagNode.h"
#include "TypedFunction.h"

#include <vector>

namespace RevBayesCore {

    /**
     * @brief Make a MRF on a continuous vector using increments.
     *
     * This function takes theta[1] on the log or non-log scale, and the first-order differences (theta[i] - theta[i-1]).
     * It then computes the entire vector theta, optionally exponentiating the output first.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team
     * @since Version 1.0, 2014-07-04
     *
     */
    class AssembleOrder1ContinuousMRFFunction : public TypedFunction<RbVector<double> > {

    public:
        AssembleOrder1ContinuousMRFFunction(const TypedDagNode< double > *theta1, const TypedDagNode< RbVector<double> > *increments, bool log_theta1);
        virtual                                            ~AssembleOrder1ContinuousMRFFunction(void);                                                    //!< Virtual destructor

        // public member functions
        AssembleOrder1ContinuousMRFFunction*                              clone(void) const;                                                              //!< Create an independent clone
        void                                                update(void);

    protected:
        void                                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);                        //!< Implementation of swaping parameters

    private:

        // members
        const TypedDagNode< RbVector<double> >*             increments;
        const TypedDagNode< double >*                       theta1;

        size_t                                              field_size;
        bool                                                theta_1_is_log;

    };

}

#endif
