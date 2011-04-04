/**
 * @file
 * This file contains the declaration of DistributionReal, which specifies the
 * interface for distributions on real-valued variables.
 *
 * @brief Declaration of DistributionReal
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-12-29 23:23:09 +0100 (Tis, 29 Dec 2009) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-27, version 1.0
 * @interface DistributionReal
 * @package distributions
 *
 * $Id: DistributionReal.h 211 2009-12-29 22:23:09Z ronquist $
 */

#ifndef DistributionReal_H
#define DistributionReal_H

#include "Distribution.h"
#include "Real.h"

#include <set>
#include <string>

class ArgumentRule;
class DAGNode;
class Move;
class RandomNumberGenerator;
class StochasticNode;
class VectorString;

class DistributionReal: public Distribution {

    public:
	    virtual                    ~DistributionReal(void) {}                                                  //!< Destructor

        // Basic utility functions
        virtual const VectorString& getClass(void) const;                                                      //!< Get class vector   

        // Member variable rules and random variable type
        virtual const MemberRules&  getMemberRules(void) const = 0;                                            //!< Get member rules
        virtual const std::string&  getVariableType(void) const;                                               //!< Get random variable type

        // Member method inits
        const MethodTable&			getMethodInits(void) const;                                                //!< Get method inits
        
        // Real-valued distribution functions
        virtual double              cdf(const double q) = 0;                                                   //!< Cumulative probability
        virtual Move*               getDefaultMove(StochasticNode* node) = 0;                                  //!< Get default move
        virtual const Real*         getMax(void);                                                              //!< Get max value
        virtual const Real*         getMin(void);                                                              //!< Get min value
        virtual double              lnLikelihoodRatio(const RbObject* value) = 0;                              //!< Ln prob ratio of A | B when only B is touched
        virtual double              lnPdf(const RbObject* value) = 0;                                          //!< Ln probability density
        virtual double              lnPriorRatio(const RbObject* newVal, const RbObject* oldVal) = 0;          //!< Ln prob ratio of A | B when only A is touched
        virtual double              lnProbabilityRatio(const RbObject* newVal, const RbObject* oldVal) = 0;    //!< Ln prob ratio of A | B when both A and B are touched
        virtual double              pdf(const RbObject* value) = 0;                                            //!< Probability density
        virtual double              quantile(const double p) = 0;                                              //!< Quantile
        virtual Real*               rv(void) = 0;                                                              //!< Generate a random draw

    protected:
                                    DistributionReal(const MemberRules& memberRules);                          //!< Constructor

        // Member method call
        DAGNode*                    executeOperation(const std::string& name, std::vector<VariableSlot>& args);    //!< Execute method
};

#endif

