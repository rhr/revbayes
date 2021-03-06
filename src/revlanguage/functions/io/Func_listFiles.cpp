#include "ArgumentRule.h"
#include "ConstantNode.h"
#include "Ellipsis.h"
#include "Func_listFiles.h"
#include "ModelVector.h"
#include "NclReader.h"
#include "RbException.h"
#include "RbFileManager.h"
#include "RevNullObject.h"
#include "RlBoolean.h"
#include "RlString.h"
#include "RlTimeTree.h"
#include "RlUtils.h"
#include "StringUtilities.h"
#include "Tree.h"
#include "RlUserInterface.h"

#include <map>
#include <set>
#include <sstream>


using namespace RevLanguage;

/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Func_listFiles* Func_listFiles::clone( void ) const
{
    
    return new Func_listFiles( *this );
}


/** Execute function */
RevPtr<RevVariable> Func_listFiles::execute( void )
{
    
    // get the information from the arguments for reading the file
    size_t arg_index = 0;
    const std::string& pn   = static_cast<const RlString&>( args[arg_index++].getVariable()->getRevObject() ).getValue();
    //bool all_files          = static_cast<const RlBoolean&>( args[arg_index++].getVariable()->getRevObject() ).getValue();
    bool recursive          = static_cast<const RlBoolean&>( args[arg_index++].getVariable()->getRevObject() ).getValue();
    //bool incl_dirs          = static_cast<const RlBoolean&>( args[arg_index++].getVariable()->getRevObject() ).getValue();
    
    // check that the file/path name has been correctly specified
    RevBayesCore::RbFileManager my_file_manager( pn );
    if ( my_file_manager.testDirectory() == false )
    {
        std::string errorStr = "";
        my_file_manager.formatError( errorStr );
        throw RbException("Could not find path with name \"" + pn + "\"");
    }
    
    // set up a vector of strings containing the name or names of the files to be read
    RevBayesCore::RbVector<std::string> file_names;
    if ( my_file_manager.isDirectory() )
    {
        my_file_manager.setStringWithNamesOfFilesInDirectory( file_names, recursive );
    }
    else
    {
        file_names.push_back( my_file_manager.getFullFileName() );
    }
    
    return new RevVariable( new ModelVector<RlString>( file_names ) );
}


/** Get argument rules */
const ArgumentRules& Func_listFiles::getArgumentRules( void ) const
{
    
    static ArgumentRules argument_rules = ArgumentRules();
    static bool rules_set = false;
    
    if (!rules_set)
    {
        
        argument_rules.push_back( new ArgumentRule( "path", RlString::getClassTypeSpec(), "The path in which to list files.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlString(".") ) );
        argument_rules.push_back( new ArgumentRule( "allFiles", RlBoolean::getClassTypeSpec(), "Should special files be returned.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );
        argument_rules.push_back( new ArgumentRule( "recursive", RlBoolean::getClassTypeSpec(), "Should we search directories recursively.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );
        argument_rules.push_back( new ArgumentRule( "includeDirs", RlBoolean::getClassTypeSpec(), "Should we re return directory names in recursive search.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );
        rules_set = true;

    }
    
    return argument_rules;
}


/** Get Rev type of object */
const std::string& Func_listFiles::getClassType(void)
{
    
    static std::string revType = "Func_listFiles";
    
    return revType;
}


/** Get class type spec describing type of object */
const TypeSpec& Func_listFiles::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_listFiles::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "listFiles";
    
    return f_name;
}


/** Get type spec */
const TypeSpec& Func_listFiles::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get return type */
const TypeSpec& Func_listFiles::getReturnType( void ) const
{
    
    static TypeSpec return_typeSpec = ModelVector<RlString>::getClassTypeSpec();
    return return_typeSpec;
}




