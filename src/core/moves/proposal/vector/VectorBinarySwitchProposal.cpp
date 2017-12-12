#include "VectorBinarySwitchProposal.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbException.h"
#include "TypedDagNode.h"

#include <cmath>
#include <iostream>

using namespace RevBayesCore;

/**
 * Constructor
 *
 * Here we simply allocate and initialize the Proposal object.
 */
VectorBinarySwitchProposal::VectorBinarySwitchProposal( std::vector< StochasticNode<long> *> n, const std::vector<long> &i, double sp) : Proposal(),
variables( n ),
indices( i ),
length( indices.size() ),
switch_probability( sp )
{
    // tell the base class to add the node
    for (std::vector< StochasticNode<long> *>::const_iterator it = variables.begin(); it != variables.end(); it++)
    {
        addNode( *it );
    }
    
    // if the length is 0 then we use all elements (no indices were provided)
    if ( length == 0 )
    {
        length = variables.size();
    }
    
    
}


/**
 * The cleanProposal function may be called to clean up memory allocations after AbstractMove
 * decides whether to accept, reject, etc. the proposed value.
 *
 */
void VectorBinarySwitchProposal::cleanProposal( void )
{
    ; //variable->clearTouchedElementIndices();
}

/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the proposal.
 */
VectorBinarySwitchProposal* VectorBinarySwitchProposal::clone( void ) const
{
    
    return new VectorBinarySwitchProposal( *this );
}


/**
 * Get Proposals' name of object
 *
 * \return The Proposals' name.
 */
const std::string& VectorBinarySwitchProposal::getProposalName( void ) const
{
    static std::string name = "VectorBinarySwitch";
    
    return name;
}


/**
 * Perform the proposal.
 *
 * A scaling Proposal draws a random uniform number u ~ unif (-0.5,0.5)
 * and Slides the current vale by a scaling factor
 * sf = exp( lambda * u )
 * where lambda is the tuning parameter of the Proposal to influence the size of the proposals.
 *
 * \return The hastings ratio.
 */
double VectorBinarySwitchProposal::doProposal( void )
{
    
    // Get random number generator
    RandomNumberGenerator* rng = GLOBAL_RNG;
    
    // how many elements did we specifically want to update?
    size_t n_indices = indices.size();
    
    // clear update set
    update_set.clear();
    
    // get values to flip
//    RbVector<long> &val = variables->getValue();
 
    // choose one value in the vector to flip
    double u = (size_t)(rng->uniform01() * length);
//    long v = variables[u]->getValue() == 0 ? 1 : 0;
    variables[u]->setValue( new long(variables[u]->getValue() == 0 ? 1 : 0) ); //(variables[u]->getValue() == 0 ? 1 : 0) );
    update_set.insert(u);
    
    // flip all remaining values in indices according to switch_probability
    for (size_t i = 0; i < length; i++) {
        size_t index = ( n_indices == 0 ? i : indices[i]);
        if (rng->uniform01() < switch_probability) {
            variables[index]->setValue( new long(variables[index]->getValue() == 0 ? 1 : 0) );
//            val[index] = ( val[index]==0 ? 1 : 0 );
            update_set.insert( index );
        }
    }
    
    return 0.0;
}


/**
 * Prepare the proposal, e.g., pick the element that we want to change.
 * Here we do not need to do any preparation.
 */
void VectorBinarySwitchProposal::prepareProposal( void )
{
    
}


/**
 * Print the summary of the Proposal.
 *
 * The summary just contains the current value of the tuning parameter.
 * It is printed to the stream that it passed in.
 *
 * \param[in]     o     The stream to which we print the summary.
 */
void VectorBinarySwitchProposal::printParameterSummary(std::ostream &o) const
{
    
    o << "switch_probability = " << switch_probability;
    
}


/**
 * Reject the Proposal.
 *
 * Since the Proposal stores the previous value and it is the only place
 * where complex undo operations are known/implement, we need to revert
 * the value of the variable/DAG-node to its original value.
 */
void VectorBinarySwitchProposal::undoProposal( void )
{
    
    // flip all remaining values in indices according to switch_probability
    for (std::set<size_t>::iterator it = update_set.begin(); it != update_set.end(); it++) {
        size_t index = *it;
        variables[index]->setValue( new long(variables[index]->getValue() == 0 ? 1 : 0) );
    }
    
    // clear elements in update_set
    update_set.clear();
    
}


/**
 * Swap the current variable for a new one.
 *
 * \param[in]     oldN     The old variable that needs to be replaced.
 * \param[in]     newN     The new RevVariable.
 */
void VectorBinarySwitchProposal::swapNodeInternal(DagNode *oldN, DagNode *newN)
{
    for (size_t i = 0; i < variables.size(); ++i)
    {
        if ( variables[i] == oldN )
        {
            variables[i] = static_cast<StochasticNode<long> *>(newN);
        }
    }
}


/**
 * Tune the Proposal to accept the desired acceptance ratio.
 *
 * The acceptance ratio for this Proposal should be around 0.44.
 * If it is too large, then we increase the proposal size,
 * and if it is too small, then we decrease the proposal size.
 */
void VectorBinarySwitchProposal::tune( double rate )
{
    
    if ( rate > 0.44 )
    {
        switch_probability *= (1.0 + ((rate-0.44)/0.56) );
    }
    else
    {
        switch_probability /= (2.0 - rate/0.44 );
    }
    
    if (switch_probability > 1.0) {
        switch_probability = 1.0;
    } else if (switch_probability < 1 / length) {
        switch_probability = 1 / length;
    }
    
}

