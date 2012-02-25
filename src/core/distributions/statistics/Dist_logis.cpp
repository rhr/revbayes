/**
 * @file
 * This file contains the implementation of Dist_logis, which is used to hold
 * parameters and functions related to a Logistic distribution.
 *
 * @brief Implementation of Dist_logis
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-27, version 1.0
 *
 * $Id$
 */

#include "DAGNode.h"
#include "Dist_logis.h"
#include "Move_msimplex.h"
#include "RealPos.h"
#include "RandomNumberGenerator.h"
#include "RbException.h"
#include "DistributionLogistic.h"
#include "Real.h"
#include "Simplex.h"
#include "ValueRule.h"
#include "VectorRealPos.h"
#include "VectorString.h"
#include "Workspace.h"

#include <cmath>
#include <sstream>
#include <vector>

#include "MatrixReal.h"
#include "RbMathMatrix.h"


/** Default constructor for parser use */
Dist_logis::Dist_logis( void ) : DistributionContinuous( getMemberRules() ), location( NULL), scale( NULL ) {

}



/** Clone this object */
Dist_logis* Dist_logis::clone( void ) const {

    return new Dist_logis( *this );
}


/** Get class name of object */
const std::string& Dist_logis::getClassName(void) { 
    
    static std::string rbClassName = "Logistic distribution";
    
	return rbClassName; 
}

/** Get class type spec describing type of object */
const TypeSpec& Dist_logis::getClassTypeSpec(void) { 
    
    static TypeSpec rbClass = TypeSpec( getClassName(), new TypeSpec( DistributionContinuous::getClassTypeSpec() ) );
    
	return rbClass; 
}

/** Get type spec */
const TypeSpec& Dist_logis::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Get member variable rules */
const MemberRules& Dist_logis::getMemberRules( void ) const {

    static MemberRules memberRules = MemberRules();
    static bool        rulesSet = false;

    if ( !rulesSet ){
        memberRules.push_back( new ValueRule( "location",  Real::getClassTypeSpec()    ) );
        memberRules.push_back( new ValueRule( "scale",     RealPos::getClassTypeSpec() ) );
        rulesSet = true;
    }

    return memberRules;
}


/** Get random variable type */
const TypeSpec& Dist_logis::getVariableType( void ) const {

    static TypeSpec varTypeSpec = Real::getClassTypeSpec();
    
    return varTypeSpec;
}

/**
 * This function calculates the probability density
 * for a Logistic-distributed random variable.
 *
 * @brief Logistic probability density
 *
 * @param value Observed value
 * @return      Probability density
 */
double Dist_logis::pdf( const RbLanguageObject& value ) const {
    
    double l = static_cast<const Real&     >( location->getValue() ).getValue();
    double s = static_cast<const RealPos&  >( scale->getValue()    ).getValue();
    double x = static_cast<const Real&     >( value               ).getValue();

	return RbStatistics::Logistic::pdf( l, s, x );

}



/**
 * This function calculates the natural log of the probability
 * density for a logistic-distributed random variable.
 *
 * @brief Natural log of logistic probability density
 *
 * @param value Observed value
 * @return      Natural log of the probability density
 */
double Dist_logis::lnPdf( const RbLanguageObject& value ) const {

	// Get the value and the parameters of the Logistic
    double l = static_cast<const Real&     >( location->getValue() ).getValue();
    double s = static_cast<const RealPos&  >( scale->getValue()    ).getValue();
    double x = static_cast<const Real&     >( value               ).getValue();

	return RbStatistics::Logistic::lnPdf( l, s, x );
}


/**
 * This function calculates the cumulative probability for
 * a Logistic-distributed random variable.
 *
 * @brief Logistic cumulative probability
 *
 * @param q     Quantile
 * @return      Cumulative probability
 *
 */
double Dist_logis::cdf( const RbLanguageObject& value ) {
    
    double l = static_cast<const Real&     >( location->getValue() ).getValue();
    double s = static_cast<const RealPos&  >( scale->getValue()    ).getValue();
    double x = static_cast<const Real&     >( value               ).getValue();

	return RbStatistics::Logistic::cdf( l, s, x );
}



/**
 * This function calculates the quantile for a
 * Logistic-distributed random variable.
 *
 * @brief Quantile of Logistic probability density
 *
 * @param p     Cumulative probability of quantile
 * @return      Quantile
 *
 */
const Real& Dist_logis::quantile( const double p ) {
    
    double l = static_cast<const Real&     >( location->getValue() ).getValue();
    double s = static_cast<const RealPos&  >( scale->getValue()    ).getValue();

	double q = RbStatistics::Logistic::quantile(l, s, p);
	quant.setValue( q );
    
    return quant;
}


/**
 * This function generates a Logistic-distributed
 * random variable.
 *
 * @brief Random draw from Logistic distribution
 *
 * @return      Random draw from Logistic distribution
 */



const RbLanguageObject& Dist_logis::rv(void) {
    
    double l = static_cast<const Real&     >( location->getValue() ).getValue();
    double s = static_cast<const RealPos&  >( scale->getValue()    ).getValue();

    RandomNumberGenerator* rng = GLOBAL_RNG;
    randomVariable.setValue( RbStatistics::Logistic::rv(l, s, *rng) );

	return randomVariable;
}


/** We catch here the setting of the member variables to store our parameters. */
void Dist_logis::setMemberVariable(std::string const &name, Variable *var) {
    
    if ( name == "location" ) {
        location = var;
    }
    else if ( name == "scale" ){
        scale = var;
    }
    else {
        DistributionContinuous::setMemberVariable(name, var);
    }
}

