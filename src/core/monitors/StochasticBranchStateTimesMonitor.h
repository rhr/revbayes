//
//  StochasticBranchStateTimesMonitor.h
//  RevBayes_development_branch
//
//  Created by will freyman on 4/25/18.
//  Copyright (c) 2018 will freyman. All rights reserved.
//

#ifndef RevBayes_development_branch_StochasticBranchStateTimesMonitor_h
#define RevBayes_development_branch_StochasticBranchStateTimesMonitor_h

#include "StateDependentSpeciationExtinctionProcess.h"
#include "VariableMonitor.h"
#include "TypedDagNode.h"
#include "StochasticNode.h"

#include <fstream>
#include <string>
#include <vector>
#include <typeinfo>


namespace RevBayesCore {
    
    /**
     * Monitor to print out the time spent in each state during an SSE stochastically mapped character history.
     */
    class StochasticBranchStateTimesMonitor : public VariableMonitor {
        
    public:
        
        // Constructors and Destructors
        StochasticBranchStateTimesMonitor(StochasticNode<Tree>* ch, unsigned long g, const std::string &fname, const std::string &del);
        StochasticBranchStateTimesMonitor(const StochasticBranchStateTimesMonitor &m);
        virtual ~StochasticBranchStateTimesMonitor(void);
        
        StochasticBranchStateTimesMonitor*                  clone(void) const;                                                  //!< Clone the object
        
        // Monitor functions
        void                                                monitorVariables(unsigned long gen);                                 //!< Monitor at generation gen
        void                                                printFileHeader(void);                                              //!< Print header
        
        // getters and setters
        void                                                swapNode(DagNode *oldN, DagNode *newN);
        
    private:
        
        // members
        TypedDagNode<Tree>*                                 tree;
        StochasticNode<Tree>*                               cdbdp;                                                              //!< The character dependent birth death process we are monitoring
    };
    
}

#endif
