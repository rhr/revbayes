#include "RbException.h"
#include "RbUtil.h"
#include "RbOptions.h"
#include "SyntaxWorkspaceVariableAssignment.h"
#include "Workspace.h"

#include <iostream>
#include <list>
#include <sstream>

using namespace RevLanguage;

/** Basic constructor from lef-hand side and right-hand side expressions */
SyntaxWorkspaceVariableAssignment::SyntaxWorkspaceVariableAssignment( SyntaxElement* lhsExpr, SyntaxElement* rhsExpr ) : SyntaxAssignment(lhsExpr,rhsExpr)
{
    
}


/** Destructor deletes operands */
SyntaxWorkspaceVariableAssignment::~SyntaxWorkspaceVariableAssignment( void )
{
    
}


/** Type-safe clone of the syntax element */
SyntaxWorkspaceVariableAssignment* SyntaxWorkspaceVariableAssignment::clone () const
{
    return new SyntaxWorkspaceVariableAssignment( *this );
}


/**
 * Get semantic value. The semantic value of a WorkspaceVariable assignment is identical to
 * that of a constant assignment, the only difference being that the WorkspaceVariable
 * variable flag of the resulting variable will be set to true.
 *
 * Note that the return variable is not set to a WorkspaceVariable variable, in case it is
 * used in further assignments of other types.
 */
void SyntaxWorkspaceVariableAssignment::assign(RevPtr<Variable> &lhs, RevPtr<Variable> &rhs)
{
#ifdef DEBUG_PARSER
    printf( "Evaluating WorkspaceVariable assignment\n" );
#endif
    
    // Get a reference to the Rev object value
    const RevObject& value = rhs->getRevObject();
    
    // TODO: This needs to be cleaned up because it is not used properly anymore! (Sebastian)
    // Perform type conversion if needed, otherwise just clone the value object
    RevObject* newValue;
    if ( !value.getTypeSpec().isDerivedOf( lhs->getRevObjectTypeSpec() ) )
    {
        // We are not of a derived type (or the same type) so we need to cast
        if (value.isConvertibleTo( lhs->getRevObjectTypeSpec(), true ) )
        {
            newValue = value.convertTo( lhs->getRevObjectTypeSpec() );
        }
        else
        {
            std::ostringstream msg;
            msg << "Cannot assign variable '" << lhs->getName() << "' with value of type '" << value.getTypeSpec().getType() << "'" << std::endl;
            msg << " because the variable requires type '" << lhs->getRevObjectTypeSpec().getType() << "'" << std::endl;
            throw RbException( msg );
        }
    }
    else
    {
        newValue = value.clone();
        newValue->makeConstantValue();  // We cannot trust evaluateContent to return a constant variable
    }
    
    // Fill the slot with newValue. The variable itself will be
    // passed on as the semantic value of the statement and can
    // be used in further assignments.
    lhs->setRevObject( newValue );
    lhs->setWorkspaceVariableState( true );
    
#ifdef DEBUG_PARSER
    env.printValue(std::cerr);
#endif
    
}


