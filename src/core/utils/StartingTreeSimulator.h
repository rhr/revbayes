#ifndef StartingTreeSimulator_H
#define StartingTreeSimulator_H

#include "Tree.h"

#include <string>
#include <vector>

namespace RevBayesCore {
    
    /**
     * This class represents the writer object of character data objects into files in Fasta format.
     *
     * This class currently has only one functionality,
     * to write character data objects into a file in Fasta format.
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2013-04-15, version 1.0
     */
    class StartingTreeSimulator {
        
    public:
        
        enum SIM_CONDITION { TIME, SURVIVAL, ROOT };
        
        StartingTreeSimulator();
        
        Tree*                                   simulateTree( const std::vector<Taxon> &taxa, const std::vector<Clade> &constraints ) const;
        
    private:
        
        void                                    simulateClade( std::set<TopologyNode*> &nodes) const;
        
    };
    
}


#endif
