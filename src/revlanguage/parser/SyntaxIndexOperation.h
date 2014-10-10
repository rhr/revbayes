#ifndef SyntaxIndexOperation_H
#define SyntaxIndexOperation_H

#include "SyntaxElement.h"

#include <iostream>
#include <list>

namespace RevLanguage {

    class SyntaxFunctionCall;
    class VariableSlot;

    /**
     * This is the class used to hold variables in the syntax tree.
     *
     * We store the identifier, the index vector and the base variable
     * here so that we can wrap these things into a DAG node expression
     * if needed.
     *
     */

    class SyntaxIndexOperation : public SyntaxElement {

    public:
        SyntaxIndexOperation(SyntaxElement* var, SyntaxElement* indx);                                                              //!< Standard constructor
        SyntaxIndexOperation(const SyntaxIndexOperation& x);                                                                        //!< Copy constructor

        virtual                            ~SyntaxIndexOperation(void);                                                             //!< Destructor deletes variable, identifier and index

        // Assignment operator
        SyntaxIndexOperation&               operator=(const SyntaxIndexOperation& x);                                               //!< Assignment operator

        // Basic utility functions
        SyntaxIndexOperation*               clone(void) const;                                                                      //!< Clone object
        void                                printValue(std::ostream& o) const;                                                      //!< Print info about object

        // Regular functions
        RevPtr<Variable>                    evaluateLHSContent(Environment& env, const std::string& varType);                       //!< Get semantic lhs value
        RevPtr<Variable>                    evaluateContent(Environment& env, bool dynamic=false);                                  //!< Get semantic value
        
    protected:
        SyntaxElement*                      index;                                                                                  //!< Vector of int indices to variable element
        SyntaxElement*                      baseVariable;                                                                           //!< Base variable (pointing to a composite node)
    };

}

#endif

