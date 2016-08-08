#include "Func_extantTree.h"
#include "ExtantTreeFunction.h"
#include "RlTree.h"
#include "RlDeterministicNode.h"
#include "TypedDagNode.h"

using namespace RevLanguage;

/** default constructor */
Func_extantTree::Func_extantTree( void ) : TypedFunction<Tree>( )
{
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_extantTree* Func_extantTree::clone( void ) const
{
    
    return new Func_extantTree( *this );
}


RevBayesCore::TypedFunction<RevBayesCore::Tree>* Func_extantTree::createFunction( void ) const
{
    
    RevBayesCore::TypedDagNode<RevBayesCore::Tree>* tau = static_cast<const Tree&>( this->args[0].getVariable()->getRevObject() ).getDagNode();
    
    RevBayesCore::ExtantTreeFunction* f = new RevBayesCore::ExtantTreeFunction( tau );
    
    return f;
}


/* Get argument rules */
const ArgumentRules& Func_extantTree::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool          rulesSet = false;
    
    if ( !rulesSet )
    {
        
        argumentRules.push_back( new ArgumentRule( "tree", Tree::getClassTypeSpec(), "The tree variable.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        rulesSet = true;
    }
    
    return argumentRules;
}


const std::string& Func_extantTree::getClassType(void)
{
    
    static std::string revType = "Func_extantTree";
    
    return revType;
}


/* Get class type spec describing type of object */
const TypeSpec& Func_extantTree::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return revTypeSpec;
}



/**
 * Get the primary Rev name for this function.
 */
std::string Func_extantTree::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "fnExtantTree";
    
    return f_name;
}


const TypeSpec& Func_extantTree::getTypeSpec( void ) const
{
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}