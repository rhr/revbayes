#include "Clade.h"
#include "AbstractRootedTreeDistribution.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbException.h"
#include "RbMathCombinatorialFunctions.h"
#include "StochasticNode.h"
#include "Taxon.h"
#include "TopologyNode.h"
#include "TreeUtilities.h"

#include <algorithm>
#include <cmath>

using namespace RevBayesCore;


/**
 * Constructor.
 *
 * The constructor connects the parameters of the birth-death process (DAG structure)
 * and initializes the probability density by computing the combinatorial constant of the tree structure.
 *
 * \param    ra       The start time of the process.
 * \param    tn       Taxon names used during initialization.
 * \param    uo       True = condition on the origin time, False = condition on the root age.
 */
AbstractRootedTreeDistribution::AbstractRootedTreeDistribution(const TypedDagNode<double> *ra, const std::vector<Taxon> &tn, bool uo ) : TypedDistribution<Tree>( new Tree() ),
    process_age( ra ),
    taxa( tn ),
    use_origin( uo )
{
    // add the parameters to our set (in the base class)
    // in that way other class can easily access the set of our parameters
    // this will also ensure that the parameters are not getting deleted before we do
    addParameter( process_age );

    std::set<std::string> found;
    for (size_t i = 0; i < taxa.size(); i++)
    {
        if (found.find(taxa[i].getName()) == found.end())
        {
            found.insert(taxa[i].getName());
        }
        else
        {
            std::stringstream ss;
            ss << "Duplicate taxon name '" << taxa[i].getName() << "' encountered when building tree distribution";
            throw(RbException(ss.str()));
        }
    }
}


AbstractRootedTreeDistribution::~AbstractRootedTreeDistribution(void)
{
    
}


void AbstractRootedTreeDistribution::buildRandomBinaryTree(std::vector<TopologyNode*> &tips)
{
    
    if (tips.size() < taxa.size() )
    {
        // Get the rng
        RandomNumberGenerator* rng = GLOBAL_RNG;
        
        // randomly draw one node from the list of tips
        size_t index = static_cast<size_t>( floor(rng->uniform01()*tips.size()) );
        
        // get the node from the list
        TopologyNode* parent = tips.at(index);
        
        // remove the randomly drawn node from the list
        tips.erase(tips.begin()+long(index));
        
        // add a left child
        TopologyNode* leftChild = new TopologyNode(0);
        parent->addChild(leftChild);
        leftChild->setParent(parent);
        tips.push_back(leftChild);
        
        // add a right child
        TopologyNode* rightChild = new TopologyNode(0);
        parent->addChild(rightChild);
        rightChild->setParent(parent);
        tips.push_back(rightChild);
        
        // recursive call to this function
        buildRandomBinaryTree(tips);
    }
    
}



double AbstractRootedTreeDistribution::computeLnProbability( void )
{
    
    // proceed as long as derived classes validate a non-zero likeilhood
    if ( isLnProbabilityNonZero() == false )
    {
        return RbConstants::Double::neginf;
    }
    
    // check that the ages are in correct chronological order
    // i.e., no child is older than its parent
    const std::vector<TopologyNode*>& nodes = value->getNodes();
    for (std::vector<TopologyNode*>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
    {

        const TopologyNode &the_node = *(*it);
        if ( the_node.isRoot() == false )
        {
            if( the_node.isTip() )
            {
                if ( the_node.isSampledAncestor() == true )
                {
                    if( the_node.getAge() - the_node.getParent().getAge() != 0 )
                    {
                        return RbConstants::Double::neginf;
                    }
                    else if ( the_node.isFossil() == false )
                    {
                        return RbConstants::Double::neginf;
                    }
                    else if ( the_node.getBranchLength() != 0 )
                    {
                        return RbConstants::Double::neginf;
                    }

                }
            }
            else if( the_node.getAge() - the_node.getParent().getAge() > 0 )
            {
                return RbConstants::Double::neginf;
            }
            
        }
        else if ( the_node.getAge() > getOriginAge() )
        {
            return RbConstants::Double::neginf;
        }
        
        if ( the_node.getBranchLength() < 0 )
        {
            return RbConstants::Double::neginf;
        }

    }
    
    // present time
    double ra = value->getRoot().getAge();
    
    if ( ra > getOriginAge() || ra != getRootAge() )
    {
        return RbConstants::Double::neginf;
    }
        
    const std::vector<TopologyNode*> &c = value->getRoot().getChildren();
    
    for (std::vector<TopologyNode*>::const_iterator it = c.begin(); it != c.end(); ++it)
    {
        if ( ra < (*it)->getAge() )
        {
            return RbConstants::Double::neginf;
        }
    }
    
    // variable declarations and initialization
    double lnProbTimes = 0;

    // multiply the probability of a descendant of the initial species
    lnProbTimes += computeLnProbabilityDivergenceTimes();
    
    return lnProbTimes + lnProbTreeShape();
}


bool AbstractRootedTreeDistribution::isLnProbabilityNonZero(void)
{
    return true;
}


/**
 * Compute the diversity of the tree at time t.
 *
 * \param[in]    t      time at which we want to know the diversity.
 *
 * \return The diversity (number of species in the reconstructed tree).
 */
int AbstractRootedTreeDistribution::diversity(double t) const
{
    recomputeDivergenceTimesSinceOrigin();
    
    for (size_t i = 0; i < divergence_times.size(); ++i)
    {
        if ( divergence_times[i] > t )
        {
            return int( i + 2 );
        }
    }
    
    int rv = int(divergence_times.size() + 2);
    
    return rv;
}


void AbstractRootedTreeDistribution::getAffected(RbOrderedSet<DagNode *> &affected, RevBayesCore::DagNode *affecter)
{
    
    if ( affecter == process_age && dag_node != NULL )
    {
        dag_node->initiateGetAffectedNodes( affected );
    }
    
}


/**
 * Get the age of the internal nodes meassured since the time of the most recent tip.
 * We get the ages from the nodes and simply subtract these from the age of the most recent tip.
 *
 * \return     A vector of ages. The caller needs to deallocate this vector.
 */
std::vector<double> AbstractRootedTreeDistribution::getAgesOfInternalNodesFromMostRecentSample( void ) const
{
    
    double minTipAge = 0.0;
    const std::vector<TopologyNode*> &nodes = value->getNodes();
    
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        
        const TopologyNode& n = *(nodes[i]);
        if ( n.isTip() == true )
        {
            double tipAge = value->getNode( i ).getAge();
            if ( tipAge < minTipAge)
            {
                minTipAge = tipAge;
            }
            
        }
        
    }
    
    // retrieved the speciation times
    std::vector<double> ages;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        
        const TopologyNode& n = *(nodes[i]);
        if ( n.isInternal() == true )
        {
            double t = n.getAge() - minTipAge;
            ages.push_back(t);
        }
        
    }
    // sort the vector of times in ascending order
    std::sort(ages.begin(), ages.end());
    
    return ages;
}


/**
 * Get the age of all tip nodes meassured since the time of the most recent tip.
 * We get the ages from the nodes and simply subtruct these from the age of the most recent tip.
 *
 * \return     A vector of ages. The caller needs to deallocate this vector.
 */
std::vector<double> AbstractRootedTreeDistribution::getAgesOfTipsFromMostRecentSample( void ) const
{
    
    double minTipAge = 0.0;
    const std::vector<TopologyNode*> &nodes = value->getNodes();
    
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        
        const TopologyNode& n = *(nodes[i]);
        if ( n.isTip() == true )
        {
            double tipAge = value->getNode( i ).getAge();
            if ( tipAge < minTipAge)
            {
                minTipAge = tipAge;
            }
            
        }
        
    }
    
    // retrieved the speciation times
    std::vector<double> ages;
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        
        const TopologyNode& n = *(nodes[i]);
        if ( n.isTip() == true && n.isSampledAncestor() == false )
        {
            double t = n.getAge() - minTipAge;
            ages.push_back(t);
            
        }
        
    }
    
    // sort the vector of times in ascending order
    std::sort(ages.begin(), ages.end());
    
    return ages;
}


size_t AbstractRootedTreeDistribution::getNumberOfTaxa( void ) const
{

    return taxa.size();
}


double AbstractRootedTreeDistribution::getRootAge( void ) const
{
    
    if ( use_origin == true )
    {
        if ( value->getNumberOfNodes() > 0 )
        {
            return value->getRoot().getAge();
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return getOriginAge();
    }
    
}


double AbstractRootedTreeDistribution::getOriginAge( void ) const
{

	return process_age->getValue();
}


const std::vector<Taxon>& AbstractRootedTreeDistribution::getTaxa( void ) const
{
    
    return taxa;
}


void AbstractRootedTreeDistribution::keepSpecialization(DagNode *affecter)
{
    
    if ( affecter == process_age && dag_node != NULL)
    {
        dag_node->keepAffected();
    }
    
}


double AbstractRootedTreeDistribution::lnProbTreeShape(void) const
{
    // the number of ranked non-oriented labeled trees is
    // n! (n-1)! / 2^(n-1)

    size_t num_taxa = value->getNumberOfTips();

    return (num_taxa - 1) * RbConstants::LN2 - 2.0 * RbMath::lnFactorial((int)num_taxa) + log(num_taxa);
}


/**
 * Get the divergence times meassured since the time of the origin.
 * We get the ages from the nodes and simply subtruct these from the time of the origin.
 *
 * Fills vector of times. The caller needs to deallocate this vector.
 */
void AbstractRootedTreeDistribution::recomputeDivergenceTimesSinceOrigin( void ) const
{
    
    // get the time of the process
    double org = process_age->getValue();
    
    // retrieved the speciation times
    divergence_times = std::vector<double>();
    for (size_t i = 0; i < value->getNumberOfInteriorNodes()+1; ++i)
    {
        const TopologyNode& n = value->getInteriorNode( i );
        double t = org - n.getAge();
        divergence_times.push_back(t);
    }
    
    // sort the vector of times in ascending order
    std::sort(divergence_times.begin(), divergence_times.end());
}


void AbstractRootedTreeDistribution::redrawValue( void )
{
    
    simulateTree();
    
}


void AbstractRootedTreeDistribution::restoreSpecialization(DagNode *affecter)
{
    if ( affecter == process_age )
    {
        if ( use_origin == false )
        {
            value->getRoot().setAge( process_age->getValue() );
        }
        
        if ( dag_node != NULL )
        {
            dag_node->restoreAffected();
        }
    }
    
}


void AbstractRootedTreeDistribution::simulateClade(std::vector<TopologyNode *> &n, double age, double present)
{

    // Get the rng
    RandomNumberGenerator* rng = GLOBAL_RNG;

    // get the minimum age
    double current_age = n[0]->getAge();
    for (size_t i = 1; i < n.size(); ++i)
    {

        if ( current_age > n[i]->getAge() )
        {
            current_age = n[i]->getAge();
        }

    }
    
    std::vector<double> ages;
    while ( n.size() > 2 && current_age < age )
    {


        // get all the nodes before the current age
        std::vector<TopologyNode*> active_nodes;
        for (size_t i = 0; i < n.size(); ++i)
        {

            if ( current_age >= n[i]->getAge() )
            {
                active_nodes.push_back( n[i] );
            }

        }

        // we need to get next age of a node larger than the current age
        double next_node_age = age;
        for (size_t i = 0; i < n.size(); ++i)
        {

            if ( current_age < n[i]->getAge() && n[i]->getAge() < next_node_age )
            {
                next_node_age = n[i]->getAge();
            }

        }

        // only simulate if there are at least two valid/active nodes
        if ( active_nodes.size() <= 2 )
        {
            current_age = next_node_age;
        }
        else
        {

            // now we simulate new ages
            double next_sim_age = simulateNextAge(active_nodes.size()-2, age, present, current_age);

            if ( next_sim_age < next_node_age )
            {

                // randomly pick two nodes
                size_t index_left = static_cast<size_t>( floor(rng->uniform01()*active_nodes.size()) );
                TopologyNode* left_child = active_nodes[index_left];
                active_nodes.erase(active_nodes.begin()+long(index_left));
                size_t index_right = static_cast<size_t>( floor(rng->uniform01()*active_nodes.size()) );
                TopologyNode* right_child = active_nodes[index_right];
                active_nodes.erase(active_nodes.begin()+long(index_right));

                // erase the nodes also from the origin nodes vector
                n.erase(std::remove(n.begin(), n.end(), left_child), n.end());
                n.erase(std::remove(n.begin(), n.end(), right_child), n.end());


                // create a parent for the two
                TopologyNode *parent = new TopologyNode();
                parent->addChild( left_child );
                parent->addChild( right_child );
                left_child->setParent( parent );
                right_child->setParent( parent );
                parent->setAge( next_sim_age );

                // insert the parent to our list
                n.push_back( parent );
                
                current_age = next_sim_age;
                ages.push_back( next_sim_age );
            }
            else
            {
                current_age = next_node_age;
            }

        }
        
        if ( n.size() > 2 && current_age >= age  )
        {
            throw RbException("Unexpected number of taxa (remaining #taxa was " + StringUtilities::toString(n.size()) + " and age was " + current_age + " with maximum age of " + age + ") in tree simulation");
        }
        
    }


    if ( n.size() == 2 )
    {

        // pick two nodes
        TopologyNode* left_child = n[0];
        TopologyNode* right_child = n[1];

        // erase the nodes also from the origin nodes vector
        n.clear();

        // create a parent for the two
        TopologyNode *parent = new TopologyNode();
        parent->addChild( left_child );
        parent->addChild( right_child );
        left_child->setParent( parent );
        right_child->setParent( parent );
        parent->setAge( age );

        // insert the parent to our list
        n.push_back( parent );
    }
    else
    {
        throw RbException("Unexpected number of taxa (" + StringUtilities::toString(n.size()) + ") in tree simulation");
    }


}


double AbstractRootedTreeDistribution::simulateNextAge(size_t n, double origin, double present, double min) const
{

    std::vector<double> times = simulateDivergenceTimes(n, origin, present, min);

    return times.front();
}


void AbstractRootedTreeDistribution::simulateTree( void )
{

    // the time tree object (topology & times)
    Tree *psi = new Tree();

    // internally we treat unrooted topologies the same as rooted
    psi->setRooted( true );

    // create the tip nodes
    std::vector<TopologyNode*> nodes;
    for (size_t i=0; i<taxa.size(); ++i)
    {

        // create the i-th taxon
        TopologyNode* node = new TopologyNode( taxa[i], i );

        // set the age of this tip node
        node->setAge( taxa[i].getAge() );

        // add the new node to the list
        nodes.push_back( node );

    }


    double ra = getRootAge();
    double max_age = getOriginAge();

    double max_node_age = 0;
    for (size_t j = 0; j < nodes.size(); ++j)
    {
        if ( nodes[j]->getAge() > max_node_age )
        {
            max_node_age = nodes[j]->getAge();
        }
    }
    if ( ra <= max_node_age )
    {
        if (ra > 0.0)
        {
            throw(RbException("Root age younger than oldest taxon age"));
        }

        // Get the rng
        RandomNumberGenerator* rng = GLOBAL_RNG;

        ra = rng->uniform01() * ( max_age - max_node_age ) + max_node_age;
    }
    
    double min_node_age = max_node_age;
    for (size_t j = 0; j < nodes.size(); ++j)
    {
        if ( nodes[j]->getAge() < min_node_age )
        {
            min_node_age = nodes[j]->getAge();
        }
    }

    simulateClade(nodes, ra, min_node_age);

    TopologyNode *root = nodes[0];

    // initialize the topology by setting the root
    psi->setRoot(root, true);

    // finally store the new value
    delete value;
    value = psi;

}


void AbstractRootedTreeDistribution::setValue(Tree *v, bool f )
{
    
    // delegate to super class
    TypedDistribution<Tree>::setValue(v, f);

    if ( process_age != NULL && use_origin == false )
    {
        const StochasticNode<double> *stoch_process_age = dynamic_cast<const StochasticNode<double>* >(process_age);
        if ( stoch_process_age != NULL )
        {
            const_cast<StochasticNode<double> *>(stoch_process_age)->setValue( new double( value->getRoot().getAge() ), f);
        }
        else
        {
            //            double factor = process_age->getValue() / value->getRoot().getAge();
            //            TreeUtilities::rescaleTree( value, &value->getRoot(), factor);
            if (process_age->getValue() != value->getRoot().getAge())
            {
                throw RbException("Tree height and root age values must match when root age is not a stochastic node.");
            }
            value->getRoot().setAge( process_age->getValue() );
        }
        
    }
    
}


void AbstractRootedTreeDistribution::swapParameterInternal( const DagNode *oldP, const DagNode *newP )
{
    
    if ( oldP == process_age )
    {
        process_age = static_cast<const TypedDagNode<double>* >( newP );
    }
    
}


void AbstractRootedTreeDistribution::touchSpecialization(DagNode *affecter, bool touchAll)
{
    
    if ( affecter == process_age )
    {
        if ( use_origin == false)
        {
            value->getRoot().setAge( process_age->getValue() );
        }
        
        if ( dag_node != NULL )
        {
            dag_node->touchAffected();
        }
        
    }
    
}
