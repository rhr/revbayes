//
//  BiogeographyCladogeneticBirthDeathFunction.hpp
//  revbayes-proj
//
//  Created by Michael Landis on 12/15/18.
//  Copyright © 2018 Michael Landis. All rights reserved.
//

#ifndef BiogeographyCladogeneticBirthDeathFunction__
#define BiogeographyCladogeneticBirthDeathFunction__

#include "AbstractCladogenicStateFunction.h"
#include "CladogeneticSpeciationRateMatrix.h"
#include "RbVector.h"
#include "TypedDagNode.h"
#include "TypedFunction.h"

#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace RevBayesCore {
    
    class BiogeographyCladogeneticBirthDeathFunction : public AbstractCladogenicStateFunction, public TypedFunction<CladogeneticSpeciationRateMatrix> {
        
    public:
        
        BiogeographyCladogeneticBirthDeathFunction( const TypedDagNode< RbVector< double > >* sr, unsigned mrs, TypedDagNode< RbVector< RbVector<double> > >* cm, TypedDagNode< RbVector< double > >* cw, std::string ct="cutset");
        virtual                                                     ~BiogeographyCladogeneticBirthDeathFunction(void);
        
        const static unsigned NUM_CLADO_EVENT_TYPES                 = 3;
        
        const static unsigned SYMPATRY                              = 0;         // A  -> A or AB -> AB|A
        const static unsigned ALLOPATRY                             = 1;         // AB -> A|B
        const static unsigned JUMP_DISPERSAL                        = 2;         // A  -> A|B
        
        // public member functions
        virtual double computeDataAugmentedCladogeneticLnProbability( const std::vector<BranchHistory*>& histories,
                                                                     size_t node_index,
                                                                     size_t left_index,
                                                                     size_t right_index ) const;
        
        BiogeographyCladogeneticBirthDeathFunction*                 clone(void) const;
        std::map< std::vector<unsigned>, double >                   getEventMap(double t=0.0);
        const std::map< std::vector<unsigned>, double >&            getEventMap(double t=0.0) const;
        void                                                        setRateMultipliers(const TypedDagNode< RbVector< double > >* rm);
        void                                                        update(void);
        
    protected:
        
        void                                                        swapParameterInternal(const DagNode *oldP, const DagNode *newP);
        
    private:
        unsigned                                                    bitsToState( const std::vector<unsigned>& b );
        std::string                                                 bitsToString( const std::vector<unsigned>& b );
        std::vector<unsigned>                                       bitAllopatryComplement( const std::vector<unsigned>& mask, const std::vector<unsigned>& base );
        void                                                        bitCombinations(std::vector<std::vector<unsigned> >& comb, std::vector<unsigned> array, int i, std::vector<unsigned> accum);
        void                                                        buildBits(void);
        void                                                        buildBuddingRegions(void);
        void                                                        buildCutsets(void);
        void                                                        buildEventMap(void);
        void                                                        buildEventMapFactors(void);
        void                                                        buildRanges(std::set<unsigned>& range_set, const TypedDagNode< RbVector<RbVector<double> > >* g, bool all=true);
        void                                                        buildRangesRecursively(std::set<unsigned> s, std::set<std::set<unsigned> >& r, size_t k, const TypedDagNode< RbVector<RbVector<double> > >* g, bool all=true);
        double                                                      computeCutsetScore(std::vector<unsigned> idx, unsigned et);
        double                                                      computeModularityScore(std::vector<unsigned> idx, unsigned et);
        size_t                                                      computeNumStates(size_t numAreas, size_t maxRangeSize);
        void                                                        printEventMap(std::map< std::vector< unsigned >, double > x);
        unsigned                                                    sumBits(const std::vector<unsigned>& b);
        void                                                        updateEventMapWeights(void);
        void                                                        updateEventMapCutsetWeights(void);
        void                                                        updateEventMapModularityWeights(void);
        
        // parameters
        const TypedDagNode< RbVector<double> >*                     speciationRates;
        const TypedDagNode< RbVector<double> >*                     hiddenRateMultipliers;
        const TypedDagNode< RbVector<RbVector<double> > >*          connectivityMatrix;
        const TypedDagNode< RbVector<double> >*                     connectivityWeights;
        
        // dimensions
        unsigned                                                    numCharacters;
        unsigned                                                    num_states;
        unsigned                                                    numIntStates;
        unsigned                                                    numRanges;
        unsigned                                                    maxRangeSize;
        
        // model settings
        bool                                                        use_hidden_rate;
        bool                                                        use_cutset_mean;
        std::string                                                 connectivityType;
        
        // range codes
        std::vector<std::vector<unsigned> >                         bits;
        std::map<std::vector<unsigned>, unsigned>                   inverseBits;
        std::vector<std::vector<std::vector<unsigned> > >           bitsByNumOn;
        std::vector<std::vector<unsigned> >                         statesToBitsByNumOn;
        std::vector<std::set<unsigned> >                            statesToBitsetsByNumOn;
        std::map< std::vector<unsigned>, unsigned>                  bitsToStatesByNumOn;
        std::set<unsigned>                                          ranges;
        
        // event maps
        unsigned                                                    numEventTypes;
        std::map< std::vector<unsigned>, double >                   eventMap;
        std::map< std::vector<unsigned>, unsigned>                  eventMapTypes;
        std::map< unsigned, std::vector<unsigned> >                 eventMapCounts;
        std::map< std::vector<unsigned>, double >                   eventMapFactors;
        std::map< std::vector<unsigned>, double >                   eventMapWeights;
        std::map< std::vector<unsigned>, std::vector< std::vector<unsigned> > > eventMapCutsets; // returns the vector of cut edges for a given left/right split
        std::map< std::vector<unsigned>, unsigned >                 eventMapBuddingRegions; // returns the vector of cut edges for a given left/right split
        
        
        // MJL: eventually, deprecate this stuff
        // manages string-based simplex mapping??
        std::vector<std::string>                                    eventTypes;
        std::map<std::string, unsigned>                             eventStringToStateMap;
        
    };
    
}

#endif
