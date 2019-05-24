//
//  Func_biogeographyCladoEventsBD.cpp
//
//  Created by Michael Landis on 12/16/18.
//

#include "BiogeographyCladogeneticBirthDeathFunction.h"
#include "CladogeneticSpeciationRateMatrix.h"
#include "Func_biogeographyCladoEventsBD.h"
#include "ModelVector.h"
#include "OptionRule.h"
#include "RealPos.h"
#include "RlString.h"

using namespace RevLanguage;

/** default constructor */
Func_biogeographyCladoEventsBD::Func_biogeographyCladoEventsBD( void ) : TypedFunction<CladogeneticSpeciationRateMatrix>( ) {
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_biogeographyCladoEventsBD* Func_biogeographyCladoEventsBD::clone( void ) const {
    
    return new Func_biogeographyCladoEventsBD( *this );
}


RevBayesCore::TypedFunction< RevBayesCore::CladogeneticSpeciationRateMatrix >* Func_biogeographyCladoEventsBD::createFunction( void ) const
{
    
    RevBayesCore::TypedDagNode<RevBayesCore::RbVector<double> >* sr = static_cast<const ModelVector<RealPos> &>( this->args[0].getVariable()->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<RevBayesCore::RbVector<double> >* rm = NULL;
    if (this->args[1].getVariable()->getRevObject() != RevNullObject::getInstance())
    {
        RevBayesCore::TypedDagNode<RevBayesCore::RbVector<double> >* rm = static_cast<const ModelVector<RealPos> &>( this->args[1].getVariable()->getRevObject() ).getDagNode();
        if (rm->getValue().size() > 1)
        {
            throw RbException("Only one hidden rate category currently implemented.");
        }
        
    }
    
    
//    std::string c_type = static_cast<const RlString &>( this->args[3].getVariable()->getRevObject() ).getValue();
    RevBayesCore::TypedDagNode<RevBayesCore::RbVector<RevBayesCore::RbVector<double> > >* cm = static_cast<const ModelVector<ModelVector<RealPos> > &>( this->args[2].getVariable()->getRevObject() ).getDagNode();
    
    RevBayesCore::TypedDagNode<RevBayesCore::RbVector<double> >* cw = static_cast<const ModelVector<RealPos> &>( this->args[3].getVariable()->getRevObject() ).getDagNode();
//    std::string c_type = static_cast<const RlString &>( this->args[4].getVariable()->getRevObject() ).getValue();
    int mrs = (int)static_cast<const Natural &>( this->args[4].getVariable()->getRevObject() ).getValue();
    
    
    RevBayesCore::BiogeographyCladogeneticBirthDeathFunction* f = new RevBayesCore::BiogeographyCladogeneticBirthDeathFunction( sr, mrs, cm, cw );
    f->setRateMultipliers(rm);
    
    return f;
}


/* Get argument rules */
const ArgumentRules& Func_biogeographyCladoEventsBD::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool          rules_set = false;
    
    if ( !rules_set )
    {
        
        argumentRules.push_back( new ArgumentRule( "speciation_rates", ModelVector<RealPos>::getClassTypeSpec() , "The speciation rates for different cladogenetic event types.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        argumentRules.push_back( new ArgumentRule( "rate_multipliers", ModelVector<RealPos>::getClassTypeSpec() , "The rate multipliers for hidden rate classes.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY, NULL ) );
        argumentRules.push_back( new ArgumentRule( "connectivity_matrix", ModelVector<ModelVector<RealPos> >::getClassTypeSpec(), "The connectivity matrix.", ArgumentRule::BY_VALUE, ArgumentRule::CONSTANT, NULL ) );
        argumentRules.push_back( new ArgumentRule( "connectivity_weights", ModelVector<RealPos>::getClassTypeSpec() , "The speciation rates for different cladogenetic event types.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY, NULL ) );
        
//        std::vector<std::string> options;
//        options.push_back( "none" );
//        options.push_back( "cutset" );
////        argumentRules.push_back( new OptionRule( "connectivity_type", new RlString("modularity"), options, "How modularity of cladogenetic outcomes." ) );
        argumentRules.push_back( new ArgumentRule( "max_range_size", Natural::getClassTypeSpec(), "The maximum range size.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        
        rules_set = true;
    }
    
    return argumentRules;
}


const std::string& Func_biogeographyCladoEventsBD::getClassType(void)
{
    
    static std::string rev_type = "Func_biogeographyCladoEventsBD";
    
    return rev_type;
}

/* Get class type spec describing type of object */
const TypeSpec& Func_biogeographyCladoEventsBD::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_biogeographyCladoEventsBD::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "fnBiogeographyCladoEventsBD";
    
    return f_name;
}


const TypeSpec& Func_biogeographyCladoEventsBD::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}
