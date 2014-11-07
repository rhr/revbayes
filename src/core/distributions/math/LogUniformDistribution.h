/**
 * @file
 * This file contains the declaration of the 1/x distributed random variable class. Note,
 * to obtain a proper distribution we truncate this distribution (x in (0,a]).
 * This class is derived from the stochastic node and each instance will represent a random variable
 * from a normal distribution in the model graph.
 *
 * @brief Declaration of the stochastic DAG node base class.
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date:$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2012-06-17, version 1.0
 * @interface TypedDagNode
 *
 * $Id:$
 */



#ifndef LogUniformDistribution_H
#define LogUniformDistribution_H

#include "ContinuousDistribution.h"
#include "TypedDagNode.h"

namespace RevBayesCore {
    
    class LogUniformDistribution : public ContinuousDistribution {
        
    public:
        LogUniformDistribution(const TypedDagNode<double> *min, const TypedDagNode<double> *max);
        virtual                                            ~LogUniformDistribution(void);                                                 //!< Virtual destructor
        
        // public member functions
        double                                              cdf(void) const;                                                                  //!< Cummulative density function
        LogUniformDistribution*                             clone(void) const;                                                          //!< Create an independent clone
        double                                              computeLnProbability(void);
        double                                              getMax(void) const;
        double                                              getMin(void) const;
        double                                              quantile(double p) const;                                                       //!< Qu
        void                                                redrawValue(void);

    protected:
        // Parameter management functions
        void                                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);            //!< Swap a parameter
        
    private:
        
        // members
        const TypedDagNode<double>*                         min;
        const TypedDagNode<double>*                         max;
        
    };
    
}

#endif