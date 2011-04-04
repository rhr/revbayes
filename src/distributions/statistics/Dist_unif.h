/**
 * @file
 * This file contains the declaration of Dist_unif, which is used to hold
 * parameters and functions related to a uniform distribution.
 *
 * @brief Declaration of Dist_unif
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-12-14 12:43:32 +0100 (Mån, 14 Dec 2009) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-27, version 1.0
 *
 * $Id: MemberObject.h 194 2009-12-14 11:43:32Z ronquist $
 */

#ifndef Dist_unif_H
#define Dist_unif_H

#include "DistributionReal.h"

#include <ostream>
#include <string>

class Real;
class VectorString;

class Dist_unif: public DistributionReal {

    public:
                                    Dist_unif(void);                                                    //!< Parser constructor
                                    Dist_unif(double min, double max);                                  //!< Internal constructor

        // Basic utility functions
        Dist_unif*                  clone(void) const;                                                  //!< Clone object
        const VectorString&         getClass(void) const;                                               //!< Get class vector

        // Member variable setup
        const MemberRules&          getMemberRules(void) const;                                         //!< Get member variable rules

        // Real-valued distribution functions
        double                      cdf(const double q);                                                //!< Cumulative density
        Move*                       getDefaultMove(StochasticNode* node);                               //!< Get default move
        const Real*                 getMax(void);                                                       //!< Get max value
        const Real*                 getMin(void);                                                       //!< Get min value
        double                      lnLikelihoodRatio(const RbObject* value);                           //!< Ln prob ratio of A | B when only B is touched
        double                      lnPdf(const RbObject* value);                                       //!< Ln probability density
        double                      lnPriorRatio(const RbObject* newVal, const RbObject* oldVal);       //!< Ln prob ratio of A | B when only A is touched
        double                      lnProbabilityRatio(const RbObject* newVal, const RbObject* oldVal); //!< Ln prob ratio of A | B when both A and B are touched
        double                      pdf(const RbObject* value);                                         //!< Probability density
        double                      quantile(const double p);                                           //!< Quantile
        Real*                       rv(void);                                                           //!< Generate random variable
};

#endif

