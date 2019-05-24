#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "ConstantDemographicFunction.h"
#include "ModelVector.h"
#include "RbException.h"
#include "RealPos.h"
#include "RevObject.h"
#include "RlString.h"
#include "RlConstantDemographicFunction.h"
#include "TypeSpec.h"


using namespace RevLanguage;

ConstantDemographicFunction::ConstantDemographicFunction(void) : DemographicFunction()
{
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
ConstantDemographicFunction* ConstantDemographicFunction::clone(void) const
{
    
	return new ConstantDemographicFunction(*this);
}


void ConstantDemographicFunction::constructInternalObject( void )
{
    // we free the memory first
    delete value;
    
    // now allocate space for a new ConstantDemographicFunction object
    RevBayesCore::TypedDagNode<double> *th = static_cast<const RealPos &>( theta->getRevObject() ).getDagNode();

    value = new RevBayesCore::ConstantDemographicFunction( th );
}


/** Get Rev type of object */
const std::string& ConstantDemographicFunction::getClassType(void)
{
    
    static std::string rev_type = "ConstantDemographicFunction";
    
	return rev_type; 
}

/** Get class type spec describing type of object */
const TypeSpec& ConstantDemographicFunction::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( DemographicFunction::getClassTypeSpec() ) );
    
	return rev_type_spec; 
}


/**
 * Get the Rev name for the constructor function.
 *
 * \return Rev name of constructor function.
 */
std::string ConstantDemographicFunction::getDemographicFunctionName( void ) const
{
    // create a constructor function name variable that is the same for all instance of this class
    std::string c_name = "Constant";
    
    return c_name;
}


/** Return member rules (no members) */
const MemberRules& ConstantDemographicFunction::getParameterRules(void) const
{
    
    static MemberRules member_rules;
    static bool rules_set = false;
    
    if ( rules_set == false )
    {
        
        member_rules.push_back( new ArgumentRule("theta"  , RealPos::getClassTypeSpec(), "The population size.", ArgumentRule::BY_REFERENCE, ArgumentRule::ANY ) );
        
        rules_set = true;
    }
    
    return member_rules;
}

/** Get type spec */
const TypeSpec& ConstantDemographicFunction::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get type spec */
void ConstantDemographicFunction::printValue(std::ostream &o) const
{
    
    o << "ConstantDemographicFunction";
}


/** Set a member variable */
void ConstantDemographicFunction::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    
    if ( name == "theta" )
    {
        theta = var;
    }
    else
    {
        DemographicFunction::setConstParameter(name, var);
    }
}
