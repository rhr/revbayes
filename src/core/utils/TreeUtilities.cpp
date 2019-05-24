#include "AbstractHomologousDiscreteCharacterData.h"
#include "MatrixReal.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbBitSet.h"
#include "RbException.h"
#include "StringUtilities.h"
#include "Tree.h"
#include "TreeUtilities.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <set>

using namespace RevBayesCore;


double RevBayesCore::TreeUtilities::computeRobinsonFouldDistance(const RevBayesCore::Tree &a, const RevBayesCore::Tree &b)
{

    //const TopologyNode& r = tree->getValue().getRoot();
    std::vector<RbBitSet> bipartitions_a = a.getNodesAsBitset();
    std::vector<RbBitSet> bipartitions_b = b.getNodesAsBitset();
    bool found = false;
    double distance = 0.0;
    for (size_t i = 0; i< bipartitions_a.size(); ++i)
    {
        found = false;
        for (size_t j = 0; j < bipartitions_b.size(); ++j)
        {
            if (bipartitions_a[i] == bipartitions_b[j])
            {
                found = true;
                break;
            }
        }
        if (found == false)
        {
            distance += 1.0;
        }
    }
    for (size_t i = 0; i< bipartitions_b.size(); ++i)
    {
        found = false;
        for (size_t j = 0; j < bipartitions_b.size(); ++j)
        {
            if (bipartitions_b[i] == bipartitions_a[j])
            {
                found = true;
                break;
            }
        }

        if (found == false)
        {
            distance += 1.0;
        }
    }

    return distance;
}


void RevBayesCore::TreeUtilities::constructTimeTreeRecursively(TopologyNode *tn, const TopologyNode &n, std::vector<TopologyNode*> &nodes, std::vector<double> &ages, double depth)
{

    // set the name
    tn->setName( n.getName() );

    // copy the index
    tn->setIndex( n.getIndex() );

    // copy the branch "comments"
    const std::vector<std::string> &branchComments = n.getBranchParameters();
    for (size_t i = 0; i < branchComments.size(); ++i)
    {
        std::string tmp = branchComments[i];
        if ( tmp[0] == '&')
        {
            tmp = tmp.substr(1,tmp.size());
        }
        std::vector<std::string> pair;
        StringUtilities::stringSplit(tmp, "=", pair);
        tn->addBranchParameter(pair[0], pair[1]);
    }
    // copy the node "comments"
    const std::vector<std::string> &nodeComments = n.getNodeParameters();
    for (size_t i = 0; i < nodeComments.size(); ++i)
    {
        std::string tmp = nodeComments[i];
        if ( tmp[0] == '&')
        {
            tmp = tmp.substr(1,tmp.size());
        }
        std::vector<std::string> pair;
        StringUtilities::stringSplit(tmp, "=", pair);
        tn->addNodeParameter(pair[0], pair[1]);
    }

    // set the node flags
    tn->setSampledAncestor( n.isSampledAncestor() );

    // remember the node
    nodes.push_back( tn );

    // set the age
    double a = depth - n.getBranchLength();
    if ( a < 1E-4 )
    {
        a = 0.0;
    }
    ages.push_back( a );

    // create children
    for (size_t i = 0; i < n.getNumberOfChildren(); ++i)
    {
        const TopologyNode& child = n.getChild( i );
        TopologyNode* newChild = new TopologyNode();

        // set parent child relationship
        newChild->setParent( tn );
        tn->addChild( newChild );

        // start recursive call
        constructTimeTreeRecursively(newChild, child, nodes, ages, a);
    }

    if ( tn->getNumberOfChildren() == 1 )
    {
        tn->setSampledAncestor( true );
    }

}


RevBayesCore::Tree* RevBayesCore::TreeUtilities::convertTree(const Tree &t, bool resetIndex)
{
    // create time tree object (topology + times)
    Tree *tt = new Tree();

    // clock trees should always be rooted
    tt->setRooted( true );

    // get the root of the original tree
    const TopologyNode& bln = t.getRoot();

    TopologyNode* root = new TopologyNode();

    // copy the root index
    root->setIndex( bln.getIndex() );

    std::vector<double> ages;
    std::vector<TopologyNode*> nodes;

    double maxDepth = bln.getMaxDepth() + bln.getBranchLength();

    // recursive creation of the tree
    constructTimeTreeRecursively(root, bln, nodes, ages, maxDepth);

    // add the root which creates the topology
    tt->setRoot( root, resetIndex );

    // set the ages
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        nodes[i]->setAge( ages[i] );
    }

    // copy the root edge
    root->setBranchLength( bln.getBranchLength() );

    return tt;
}



void RevBayesCore::TreeUtilities::getAges(Tree *t, TopologyNode *n, std::vector<double>& ages, bool internalsOnly)
{
    // we only get internal node ages if internalsOnly==true
    if (n->isTip() == false )
    {
        // get the age of the node
        ages[n->getIndex()] = n->getAge();

        // get both children ages
        std::vector<TopologyNode*> children = n->getChildren();
        for (size_t i = 0; i < children.size(); i++)
            getAges( t, children[i], ages);
    }
    else if (!internalsOnly) {
      // get the age of the tip if internalsOnly==false
      ages[n->getIndex()] = n->getAge();
    }

}


RevBayesCore::DistanceMatrix* RevBayesCore::TreeUtilities::getDistanceMatrix(const Tree& tree)
{

    RevBayesCore::MatrixReal* matrix = new MatrixReal( tree.getNumberOfTips() );

    std::vector<Taxon> names = tree.getTaxa( ) ;

    std::map< std::string, int > namesToId;

    for (size_t i = 0; i < names.size(); ++i)
    {
        namesToId[ names[i].getName() ] = int(i);
    }

    std::vector< std::pair<std::string, double> > distsToRoot;

    processDistsInSubtree( tree.getRoot() , *matrix, distsToRoot, namesToId);

    DistanceMatrix* distMat = new DistanceMatrix(*matrix, names);

    // free memory
    delete matrix;

    return distMat;
}




size_t RevBayesCore::TreeUtilities::getMrcaIndex(const TopologyNode *left, const TopologyNode *right)
{
    
    if ( left == right )  //same
    {
        return left->getIndex();
    }
    else if ( left->getAge() < right->getAge() )
    {
        return RevBayesCore::TreeUtilities::getMrcaIndex( &left->getParent(), right );
    }
    else
    {
        return RevBayesCore::TreeUtilities::getMrcaIndex( left, &right->getParent() );
    }
    
}


void RevBayesCore::TreeUtilities::getOldestTip(Tree* t, TopologyNode *n, double& oldest)
{

    // we only rescale internal nodes
    if ( !n->isTip() )
    {

        // assertion that we have binary trees
#ifdef ASSERTIONS_TREE
        if ( n->getNumberOfChildren() != 2 )
        {
            throw RbException("NNI is only implemented for binary trees!");
        }
#endif

        // rescale both children
        getOldestTip( t, &n->getChild(0), oldest);
        getOldestTip( t, &n->getChild(1), oldest);
    }
    else
    {
        if (n->getAge() > oldest)
        {
            oldest = n->getAge();
        }
    }
}


void RevBayesCore::TreeUtilities::getTaxaInSubtree(TopologyNode *n, std::vector<TopologyNode*> &taxa )
{

    if ( n->isTip() )
    {
        taxa.push_back( n );
    }
    else
    {

        // recursively add children to the list of nodes in this subtree
        for (size_t i = 0; i < n->getNumberOfChildren(); ++i)
        {
            TopologyNode& child = n->getChild( i );

            getTaxaInSubtree( &child, taxa );
        }
    }

}


void RevBayesCore::TreeUtilities::offsetTree(Tree *t, TopologyNode *n, double factor)
{
    // rescale the time of the node
    double newAge = n->getAge() + factor;
    t->getNode(n->getIndex()).setAge( newAge);

    // offset all children
    std::vector<TopologyNode*> children = n->getChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        offsetTree( t, children[i], factor);
    }

}



void RevBayesCore::TreeUtilities::makeUltrametric(Tree *t)
{

    double max = 0.0;
    std::vector<double > ages ;
    for (size_t i = 0; i < t->getNumberOfTips(); ++i)
    {
        TopologyNode* node = &(t->getTipNode( i ) );
        double age = node->getBranchLength();
        node = &(node->getParent());
        
        while (!node->isRoot() )
        {
            age += node->getBranchLength();
            node = &(node->getParent());
        }
        if (age > max) {
          max = age;
        }
        ages.push_back(age);

    }

    // We extend terminal branches
    for (size_t i = 0; i < t->getNumberOfTips(); ++i)
    {
        t->getTipNode( i ).setBranchLength(t->getTipNode( i ).getBranchLength() + max - ages[i]);
//        t->getTipNode( i ).setAge(0.0);
    }
    
    setAgesRecursively(t, &(t->getRoot()), max);

    // make sure that all the tips have an age of 0
    for (size_t i = 0; i < t->getNumberOfTips(); ++i)
    {
        t->getTipNode( i ).setAge(0.0);
    }
    
}


int RevBayesCore::TreeUtilities::getNodalDistance(const TopologyNode *left, const TopologyNode *right)
{
    if ( left == right || &left->getParent() == right || left == &right->getParent() )
    {
        return 0;
    }
    else if ( left->getAge() < right->getAge() )
    {
        return 1 + RevBayesCore::TreeUtilities::getNodalDistance( &left->getParent(), right );
    }
    else
    {
        return 1 + RevBayesCore::TreeUtilities::getNodalDistance( left, &right->getParent() );
    }
}


RevBayesCore::DistanceMatrix* RevBayesCore::TreeUtilities::getNodalDistanceMatrix(const Tree& tree)
{
    RevBayesCore::MatrixReal matrix = MatrixReal( tree.getNumberOfTips() );

    std::vector<Taxon> names = tree.getTaxa( ) ;
    for (size_t i = 0; i < names.size(); i++)
    {
        for (size_t j = i + 1; j < names.size(); j++)
        {
            matrix[i][j] = matrix[j][i] = TreeUtilities::getNodalDistance(&tree.getTipNode(i), &tree.getTipNode(j));
        }
    }

    return new DistanceMatrix(matrix, names);
}


void RevBayesCore::TreeUtilities::rescaleTree(Tree *t, TopologyNode *n, double factor)
{
    // rescale the time of the node
    double newAge = n->getAge() * factor;
    t->getNode(n->getIndex()).setAge( newAge);

    // recursive call for internal nodes
    if ( n->isTip() == false )
    {

        // assertion that we have binary trees
#ifdef ASSERTIONS_TREE
        if ( n->getNumberOfChildren() != 2 )
        {
            throw RbException("Tree scaling  is only implemented for binary trees!");
        }
#endif

        // rescale both children
        rescaleTree( t, &n->getChild(0), factor);
        rescaleTree( t, &n->getChild(1), factor);
    }

}


void RevBayesCore::TreeUtilities::rescaleSubtree(Tree *t, TopologyNode *n, double factor, bool verbose)
{
    // we only rescale internal nodes
    if ( n->isTip() == false )
    {
        // rescale the age of the node
        double newAge = n->getAge() * factor;
        t->getNode( n->getIndex() ).setAge(newAge);

        // assertion that we have binary trees
        if ( verbose == true )
        {
            if ( n->getNumberOfChildren() != 2 )
            {
                throw RbException("Subtree scaling is only implemented for binary trees!");
            }
        }

        // rescale both children
        rescaleSubtree( t, &n->getChild(0), factor, verbose);
        rescaleSubtree( t, &n->getChild(1), factor, verbose);
    }

}

void RevBayesCore::TreeUtilities::setAges(Tree *t, TopologyNode *n, std::vector<double>& ages)
{
    // we only rescale internal nodes
    if ( n->isTip() == false )
    {
        // rescale the age of the node
        t->getNode( n->getIndex() ).setAge( ages[n->getIndex()] );

        // rescale both children
        std::vector<TopologyNode*> children = n->getChildren();
        for (size_t i = 0; i < children.size(); i++)
        {
            setAges( t, children[i], ages);
        }
        
    }

}

void RevBayesCore::TreeUtilities::setAgesRecursively(RevBayesCore::Tree *t, RevBayesCore::TopologyNode *n, double age)
{
    // first, we set the age of this node
    n->setAge( age );
    
    // we only rescale internal nodes
    if ( n->isTip() == false )
    {
        
        // rescale both children
        std::vector<TopologyNode*> children = n->getChildren();
        for (size_t i = 0; i < children.size(); ++i)
        {
            setAgesRecursively( t, children[i], age-children[i]->getBranchLength());
        }
        
    }
    
}


void RevBayesCore::TreeUtilities::processDistsInSubtree(const RevBayesCore::TopologyNode& node, RevBayesCore::MatrixReal& matrix, std::vector< std::pair<std::string, double> >& distsToNodeFather, const std::map< std::string, int >& namesToId)
{
	distsToNodeFather.clear();

	// node-is-leaf case
	if ( node.isTip() )
	{
		distsToNodeFather.push_back(make_pair ( node.getName(), node.getBranchLength() ) );
		return;
	}

	// For all leaves in node's subtree, get leaf-to-node distances.
	// Leaves are ordered according to the children of node.
	std::map< size_t, std::vector< std::pair<std::string, double> > > leavesDists;
	for (size_t i = 0; i < node.getNumberOfChildren(); ++i)
	{
		const RevBayesCore::TopologyNode* child = &( node.getChild(i) );
		processDistsInSubtree(*child, matrix, leavesDists[child->getIndex()], namesToId); // recursivity
	}

	// Write leaf-leaf distances to the distance matrix.
	// Only pairs in which the two leaves belong to different
	// children are considered.
	for (size_t son1_loc = 0; son1_loc < node.getNumberOfChildren(); ++son1_loc)
	{
		for (size_t son2_loc = 0; son2_loc < son1_loc; ++son2_loc)
		{
			const RevBayesCore::TopologyNode* son1 = &(node.getChild(son1_loc) );
			const RevBayesCore::TopologyNode* son2 = &(node.getChild(son2_loc) );

			for (std::vector< std::pair<std::string, double> >::iterator son1_leaf = leavesDists[son1->getIndex()].begin();
				 son1_leaf != leavesDists[son1->getIndex()].end();
				 ++son1_leaf)
			{
				for (std::vector< std::pair<std::string, double> >::iterator son2_leaf = leavesDists[son2->getIndex()].begin();
					 son2_leaf != leavesDists[son2->getIndex()].end();
					 ++son2_leaf)
				{
					int son1_leaf_id = namesToId.at( son1_leaf->first );
					int son2_leaf_id = namesToId.at( son2_leaf->first );
					matrix[son1_leaf_id] [son2_leaf_id] =
					matrix[son2_leaf_id] [son1_leaf_id] =
					( son1_leaf->second + son2_leaf->second );
				}
			}
		}
	}

	// node-is-root case
	if (node.isRoot())
	{
		// node-is-root-and-leaf case
		if (node.isTip() )
		{
			std::string root_name = node.getName();
			for (std::vector< std::pair<std::string, double> >::iterator other_leaf = leavesDists[node.getChild(0).getIndex()].begin();
				 other_leaf != leavesDists[node.getChild(0).getIndex()].end();
				 ++other_leaf)
			{
				matrix [ namesToId.at(root_name) ] [ namesToId.at(other_leaf->first) ]= matrix[ namesToId.at(other_leaf->first) ] [ namesToId.at(root_name) ] = other_leaf->second;
			}
		}

		return;
	}


	// Get distances from node's parent to considered leaves
	distsToNodeFather.clear();
	double nodeToFather = node.getBranchLength();

	for (std::map<size_t, std::vector<std::pair<std::string, double> > >::iterator son = leavesDists.begin(); son != leavesDists.end(); ++son)
	{
		for (std::vector< std::pair<std::string, double> >::iterator leaf = (son->second).begin(); leaf != (son->second).end(); ++leaf)
		{
			distsToNodeFather.push_back(make_pair(leaf->first, (leaf->second + nodeToFather)));
		}
	}

}



void RevBayesCore::TreeUtilities::climbUpTheTree(const TopologyNode& node, boost::unordered_set <const TopologyNode* >& pathFromNodeToRoot) {
    try {
        if (! (node.isRoot( ) ) ) {
            pathFromNodeToRoot.insert(&node);
            climbUpTheTree(node.getParent(), pathFromNodeToRoot);
        }
    }
    catch (RbException &e)
    {
        throw e;
    }

    return;
}


double RevBayesCore::TreeUtilities::getAgeOfMRCARecursive(const TopologyNode& node, boost::unordered_set <const TopologyNode* >& pathFromOtherNodeToRoot) {
    try {

        if ( node.isRoot() || pathFromOtherNodeToRoot.find(&node) != pathFromOtherNodeToRoot.end() ) {
            return node.getAge();
        }
        else {
            return getAgeOfMRCARecursive( node.getParent(), pathFromOtherNodeToRoot );
        }
    }
    catch (RbException &e)
    {
        throw e;
    }


}



double RevBayesCore::TreeUtilities::getAgeOfMRCA(const Tree &t, std::string first, std::string second) {

    const TopologyNode &node1 = t.getTipNodeWithName(first) ;

    const TopologyNode &node2 = t.getTipNodeWithName(second) ;

    if (! (node2.equals( node1 ) ) )
    {
        boost::unordered_set <const TopologyNode* > pathFromNode1ToRoot;
        climbUpTheTree(node1, pathFromNode1ToRoot);

        double age = getAgeOfMRCARecursive(node2, pathFromNode1ToRoot);
        return age;
    }
    else {
        return node1.getAge();
    }

}


int RevBayesCore::TreeUtilities::getCollessMetric(const TopologyNode & node, int& size)
{
    if( node.isTip() )
    {
        size = (node.getAge() == 0.0);
        return 0.0;
    }

    const TopologyNode& left = node.getChild(0);
    const TopologyNode& right = node.getChild(1);

    int left_size  = 0;
    int right_size = 0;

    double left_metric  = getCollessMetric(left, left_size);
    double right_metric = getCollessMetric(right, right_size);

    size = left_size + right_size;

    int metric = std::abs( left_size - right_size);

    if( left_size == 0 || right_size == 0 )
    {
        metric = 0;
    }

    return left_metric + right_metric + metric;
}


/* 
 * Gamma-statistic from Pybus & Harvey (2000) equation 1
 */
double RevBayesCore::TreeUtilities::getGammaStatistic(const Tree &t)
{
    std::vector<TopologyNode*> nodes = t.getNodes();

    std::vector<double> ages;
    for (size_t i = 0; i < nodes.size(); i++)
    {
        ages.push_back(nodes[i]->getAge());
    }

    // calculate internode distances
    std::sort(ages.begin(), ages.end());
    std::vector<double> distances;
    for (size_t i = (ages.size() - 1); i > 0; i--)
    {
        distances.push_back(ages[i] - ages[i - 1]);
        if (ages[i - 1] == 0)
        {
            break;
        }
    }
    
    double n = t.getNumberOfTips();
    if (n < 3)
    {
        //return NaN;
        return std::numeric_limits<double>::quiet_NaN();
    }

    double T = 0;
    for (int j = 2; j <= n; j++)
    {
        T = T + (j * distances[j - 2]); 
    }

    double a = 1 / ( n - 2 );
    double b = 0;
    for (int i = 2; i <= (n - 1); i++)
    {
        double temp = 0;
        for (int k = 2; k <= i; k++)
        {
            temp = temp + (k * distances[k - 2]);
        }
        b = b + temp; 
    }
    double num = (a * b) - (T / 2);
    
    double den = T * sqrt( (1 / (12 * (n - 2))) );


    return num/den;
}


/* 
 * Algorithm from Fitch (1970) "Distinguishing Homologous from Analogous Proteins"
 */
int RevBayesCore::TreeUtilities::getFitchScore(const Tree &t, const AbstractHomologousDiscreteCharacterData &c)
{
    int score = 0;
    for (size_t i = 0; i < c.getNumberOfCharacters(); i++)
    {
        recursivelyComputeFitch(t.getRoot(), c, i, score);
    }
    return score;
}


std::set<size_t> RevBayesCore::TreeUtilities::recursivelyComputeFitch(const TopologyNode &node, const AbstractHomologousDiscreteCharacterData &c, size_t site, int &score)
{
    if (node.isTip() == true)
    {
        std::set<size_t> tip_set;
        std::string n = node.getName();
        size_t state = c.getTaxonData(n).getCharacter(site).getStateIndex();
        tip_set.insert( state );
        return tip_set;
    }
    else
    {
        if ( node.getNumberOfChildren() != 2 )
        {
            throw RbException("Fitch score calculation is only implemented for binary trees.");
        }
        std::set<size_t> l = recursivelyComputeFitch(node.getChild(0), c, site, score);
        std::set<size_t> r = recursivelyComputeFitch(node.getChild(1), c, site, score);

        std::set<size_t> intersect;
        set_intersection(l.begin(), l.end(), r.begin(), r.end(), std::inserter(intersect, intersect.begin()));

        if (intersect.size() == 0)
        {
            score++;
            std::set<size_t> union_set;
            set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(union_set, union_set.begin()));
            return union_set;
        }
        return intersect;
    }
}


/* 
 *
 * The mean inverse equal splits metric for tips in a single state as described in:
 * Rabosky and Goldberg (2017) "FiSSE: A simple nonparametric test for the effects of a binary character on lineage diversiﬁcation rates"
 *
 * This metric is typically calculated for a single character at a time, but here it is extended over multiple characters.
 *
 */
double RevBayesCore::TreeUtilities::getMeanInverseES(const Tree &t, const AbstractHomologousDiscreteCharacterData &c, size_t state_index)
{
    if (t.isRooted() == false)
    {
        throw RbException("Mean inverse ES can only be calculated on rooted trees.");
    }

    std::vector<double> summed_inverse_es = std::vector<double>(c.getNumberOfCharacters(), 0);
    std::vector<double> num_tips_in_state = std::vector<double>(c.getNumberOfCharacters(), 0);
    std::vector<std::string> tip_names = t.getTipNames();

    // calculate equal splits (ES) measure for each tip as necessary
    for (size_t i = 0; i < tip_names.size(); i++)
    {
        bool calculated_for_tip = false;
        double tip_es = 0;
        size_t node_index = t.getTipNodeWithName( tip_names[i] ).getIndex();

        for (size_t j = 0; j < c.getNumberOfCharacters(); j++)
        {
            size_t state = c.getTaxonData(tip_names[i]).getCharacter(j).getStateIndex();
            if (state == state_index)
            {
                num_tips_in_state[j] += 1;
                if (calculated_for_tip == true)
                {
                    if (tip_es != 0)
                    {
                        summed_inverse_es[j] += 1/tip_es;
                    }
                }
                else
                {
                    // traverse from tip to root
                    double depth = 1;
                    while (true)
                    {
                        if (t.getNode(node_index).isRoot() == true)
                        {
                            break;
                        }
                        tip_es += t.getNode(node_index).getBranchLength() * (1 / pow(2, depth - 1));
                        node_index = t.getNode(node_index).getParent().getIndex();
                        depth++;
                    }
                    if (tip_es != 0)
                    {
                        summed_inverse_es[j] += 1/tip_es;
                    }
                    calculated_for_tip = true;
                }
            }
        }
    }

    // calculate mean inverse ES for the character state
    double mean_inverse_es = 0;
    for (size_t i = 0; i < c.getNumberOfCharacters(); i++)
    {
        if (num_tips_in_state[i] != 0)
        {
            mean_inverse_es += (1/num_tips_in_state[i]) * summed_inverse_es[i];
        }
    }
    return mean_inverse_es;
}


std::vector<double> RevBayesCore::TreeUtilities::getInverseES(const Tree &t)
{
    if (t.isRooted() == false)
    {
        throw RbException("Mean inverse ES can only be calculated on rooted trees.");
    }

    std::vector<double> inverse_es;

    // calculate equal splits (ES) measure for each tip
    for (size_t i = 0; i < t.getNumberOfTips(); i++)
    {
        double tip_es = 0;
        size_t node_index = t.getTipNode(i).getIndex();

        // traverse from tip to root
        double depth = 1;
        while (true)
        {
            if (t.getNode(node_index).isRoot() == true)
            {
                break;
            }
            tip_es += t.getNode(node_index).getBranchLength() * (1 / pow(2, depth - 1));
            node_index = t.getNode(node_index).getParent().getIndex();
            depth++;
        }
        if (tip_es != 0)
        {
            inverse_es.push_back(1/tip_es);
        }
    }

    return inverse_es;
}


/* 
 * Returns the Parsimoniously Same State Paths (PSSP). This is the set of branch lengths 
 * from clades parsimoniously reconstructed to have the same state. Given (((A,B),C),(D,E)), if A, B, 
 * D, and E are in state 0, then PSSP(0) will contain the four branch lengths in (A,B) and (D,E). Uses 
 * Fitch's (1970) algorithm for parsimony ancestral state reconstruction.
 */
std::vector<double> RevBayesCore::TreeUtilities::getPSSP(const Tree &t, const AbstractHomologousDiscreteCharacterData &c, size_t state_index)
{
    std::vector<double> branch_lengths;
    if ( c.getNumberOfCharacters() != 1 )
    {
        throw RbException("getPSSP is only implemented for character alignments with a single site.");
    }
    recursivelyGetPSSP(t.getRoot(), c, branch_lengths, state_index);
    return branch_lengths;
}


std::set<size_t> RevBayesCore::TreeUtilities::recursivelyGetPSSP(const TopologyNode &node, const AbstractHomologousDiscreteCharacterData &c, std::vector<double> &branch_lengths, size_t state_index)
{
    if (node.isTip() == true)
    {
        std::set<size_t> tip_set;
        std::string n = node.getName();
        size_t state = c.getTaxonData(n).getCharacter(0).getStateIndex();
        tip_set.insert( state );
        return tip_set;
    }
    else
    {
        if ( node.getNumberOfChildren() != 2 )
        {
            throw RbException("getPSSP is only implemented for binary trees.");
        }
        std::set<size_t> l = recursivelyGetPSSP(node.getChild(0), c, branch_lengths, state_index);
        std::set<size_t> r = recursivelyGetPSSP(node.getChild(1), c, branch_lengths, state_index);

        std::set<size_t> intersect;
        set_intersection(l.begin(), l.end(), r.begin(), r.end(), std::inserter(intersect, intersect.begin()));

        if (intersect.size() == 0)
        {
            std::set<size_t> union_set;
            set_union(l.begin(), l.end(), r.begin(), r.end(), std::inserter(union_set, union_set.begin()));
            return union_set;
        }
        if (intersect.size() == 1)
        {
            if (intersect.find(state_index) != intersect.end())
            {
                branch_lengths.push_back(node.getChild(0).getBranchLength());
                branch_lengths.push_back(node.getChild(1).getBranchLength());
            }
        }
        return intersect;
    }
}


/* 
 *
 * Calculate the Mean Pairwise Distance (MPD; Webb 2000; Webb et al 2002) for taxa with a certain observed character state.
 * The z-score of the MPD (optionally calculated using randomizations) is equivalent to the Net Relatedness Index (NRI; Webb 2000).
 *
 */
double RevBayesCore::TreeUtilities::calculateMPD(const Tree &t, const AbstractHomologousDiscreteCharacterData &c, size_t site_index, size_t state_index, bool zscore, bool branch_lengths, size_t num_randomizations)
{
 
    // use either pairwise branch length or nodal distances
    DistanceMatrix* distances;
    if (branch_lengths == true)
    {
        distances = getDistanceMatrix(t);
    }
    else
    {
        distances = getNodalDistanceMatrix(t);
    }

    // get the mean pairwise distances of all taxa in the observed state
    double total_dist = 0.0;
    double num_dist = 0.0;
    size_t num_taxa_in_state = 0;
    std::vector<std::string> tip_names = t.getTipNames();
    for (size_t i = 0; i < tip_names.size(); ++i)
    {
        std::string name1 = distances->getTaxa()[i].getName();
        size_t state1 = c.getTaxonData(name1).getCharacter(site_index).getStateIndex();
        if (state1 == state_index)
        {
            num_taxa_in_state += 1;
            for (size_t j = i + 1; j < tip_names.size(); ++j)
            {
                std::string name2 = distances->getTaxa()[j].getName();
                size_t state2 = c.getTaxonData(name2).getCharacter(site_index).getStateIndex();
                if (state2 == state_index)
                {
                    total_dist += distances->getMatrix()[i][j];
                    num_dist += 1.0;
                }
            }
        }   
    }
    double obs_mean_distance = 0.0;
    if (num_dist != 0)
    {
        obs_mean_distance = total_dist/num_dist;
    }
   
    if (zscore == false)
    {
        return obs_mean_distance;
    }

    // randomizations to calculate z-score (NRI)
    std::vector<double> random_mean_distances;
    for (size_t k = 0; k < num_randomizations; ++k)
    {
        std::vector<std::string> new_tip_names;

        // pick random taxa
        while (new_tip_names.size() < num_taxa_in_state)
        {
            RandomNumberGenerator* rng = GLOBAL_RNG;
            size_t u = rng->uniform01() * tip_names.size();
            std::string new_name = tip_names[u];
            if (std::find(new_tip_names.begin(), new_tip_names.end(), new_name) == new_tip_names.end())
            {
                new_tip_names.push_back(new_name);
            }
        }
        
        // calculate mean pairwise distances for the random taxa
        double total_dist = 0.0;
        double num_dist = 0.0;
        for (size_t i = 0; i < tip_names.size(); ++i)
        {
            std::string name1 = distances->getTaxa()[i].getName();
            if (std::find(new_tip_names.begin(), new_tip_names.end(), name1) != new_tip_names.end())
            {
                for (size_t j = i + 1; j < tip_names.size(); ++j)
                {
                    std::string name2 = distances->getTaxa()[j].getName();
                    if (std::find(new_tip_names.begin(), new_tip_names.end(), name2) != new_tip_names.end())
                    {
                        total_dist += distances->getMatrix()[i][j];
                        num_dist += 1.0;
                    }
                }
            }   
        }
        
        double temp_mean_distance = 0.0;
        if (num_dist != 0)
        {
            temp_mean_distance = total_dist/num_dist;
        }
        random_mean_distances.push_back(temp_mean_distance); 
    }

    // zscore = (mpd_obs - mpd_rand_mean) / mpd_rand_sd
    double random_mean = 0.0;
    for (size_t i = 0; i < random_mean_distances.size(); ++i)
    {
        random_mean += random_mean_distances[i];
    }
    if (random_mean_distances.size() != 0)
    {
        random_mean /= random_mean_distances.size();
    }
    double random_stdv = 0.0;
    for (size_t i = 0; i < random_mean_distances.size(); ++i)
    {
        random_stdv += pow(random_mean_distances[i] - random_mean, 2);
    }
    if (random_mean_distances.size() != 0)
    {
        random_stdv /= random_mean_distances.size();
    }
    random_stdv = sqrt(random_stdv);
    if (random_stdv != 0.0)
    {
        return (obs_mean_distance - random_mean) / random_stdv;
    }
    return 0.0;
}


/* 
 *
 * Calculate the Mean Nearest Taxon Distance (MNTD; Webb 2000; Webb et al 2002) for taxa with a certain observed character state.
 * The z-score of the MNTD (optionally calculated using randomizations) is equivalent to the Nearest Taxa Index (NTI; Webb 2000).
 *
 */
double RevBayesCore::TreeUtilities::calculateMNTD(const Tree &t, const AbstractHomologousDiscreteCharacterData &c, size_t site_index, size_t state_index, bool zscore, bool branch_lengths, size_t num_randomizations)
{
 
    // use either pairwise branch length or nodal distances
    DistanceMatrix* distances;
    if (branch_lengths == true)
    {
        distances = getDistanceMatrix(t);
    }
    else
    {
        distances = getNodalDistanceMatrix(t);
    }

    // get the distances of the closest relative of all taxa in the observed state
    double total_dist = 0.0;
    double num_dist = 0.0;
    size_t num_taxa_in_state = 0;
    std::vector<std::string> tip_names = t.getTipNames();
    for (size_t i = 0; i < tip_names.size(); ++i)
    {
        std::string name1 = distances->getTaxa()[i].getName();
        size_t state1 = c.getTaxonData(name1).getCharacter(site_index).getStateIndex();
        if (state1 == state_index)
        {
            num_taxa_in_state += 1;
            double min_dist = 0.0;
            for (size_t j = 0; j < tip_names.size(); ++j)
            {
                if (j != i)
                {
                    std::string name2 = distances->getTaxa()[j].getName();
                    size_t state2 = c.getTaxonData(name2).getCharacter(site_index).getStateIndex();
                    if (state2 == state_index)
                    {
                        double this_dist = distances->getMatrix()[i][j];
                        if (this_dist < min_dist || min_dist == 0.0)
                        {
                            min_dist = this_dist;
                        }
                    }
                }
            }
            total_dist += min_dist;
            num_dist += 1.0;
        }   
    }
    double obs_mean_distance = 0.0;
    if (num_dist != 0)
    {
        obs_mean_distance = total_dist/num_dist;
    }
   
    if (zscore == false)
    {
        return obs_mean_distance;
    }

    // randomizations to calculate z-score (NTI)
    std::vector<double> random_mean_distances;
    for (size_t k = 0; k < num_randomizations; ++k)
    {
        double total_dist = 0.0;
        double num_dist = 0.0;
        std::vector<std::string> new_tip_names;

        // pick random taxa
        while (new_tip_names.size() < num_taxa_in_state)
        {
            RandomNumberGenerator* rng = GLOBAL_RNG;
            size_t u = rng->uniform01() * tip_names.size();
            std::string new_name = tip_names[u];
            if (std::find(new_tip_names.begin(), new_tip_names.end(), new_name) == new_tip_names.end())
            {
                new_tip_names.push_back(new_name);
            }
        }
        
        // calculate mean nearest taxon distance for the random taxa
        for (size_t i = 0; i < tip_names.size(); ++i)
        {
            std::string name1 = distances->getTaxa()[i].getName();
            if (std::find(new_tip_names.begin(), new_tip_names.end(), name1) != new_tip_names.end())
            {
                double min_dist = 0.0;
                for (size_t j = 0; j < tip_names.size(); ++j)
                {
                    if (j != i)
                    {
                        std::string name2 = distances->getTaxa()[j].getName();
                        if (std::find(new_tip_names.begin(), new_tip_names.end(), name2) != new_tip_names.end())
                        {
                            double this_dist = distances->getMatrix()[i][j];
                            if (this_dist < min_dist || min_dist == 0.0)
                            {
                                min_dist = this_dist;
                            }
                        }
                    }
                }
                total_dist += min_dist;
                num_dist += 1.0;
            }   
        }
        
        double temp_mean_distance = 0.0;
        if (num_dist != 0)
        {
            temp_mean_distance = total_dist/num_dist;
        }
        random_mean_distances.push_back(temp_mean_distance); 
    }

    // zscore = (mntd_obs - mntd_rand_mean) / mntd_rand_sd
    double random_mean = 0.0;
    for (size_t i = 0; i < random_mean_distances.size(); ++i)
    {
        random_mean += random_mean_distances[i];
    }
    if (random_mean_distances.size() != 0)
    {
        random_mean /= random_mean_distances.size();
    }
    double random_stdv = 0.0;
    for (size_t i = 0; i < random_mean_distances.size(); ++i)
    {
        random_stdv += pow(random_mean_distances[i] - random_mean, 2);
    }
    if (random_mean_distances.size() != 0)
    {
        random_stdv /= random_mean_distances.size();
    }
    random_stdv = sqrt(random_stdv);
    if (random_stdv != 0.0)
    {
        return (obs_mean_distance - random_mean) / random_stdv;
    }
    return 0.0;
}


std::vector<double> RevBayesCore::TreeUtilities::calculateEDR(Tree &t)
{
    TopologyNode node = t.getRoot();
    std::vector<double> ages = std::vector<double>(t.getNumberOfNodes(), 0.0);
    TreeUtilities::getAges(&t, &node, ages);
    std::sort(ages.begin(), ages.end());

    std::vector<double> edr;
    double time = 0.0;
    size_t n = t.getNumberOfTips();
    for (size_t i = 0; i < ages.size(); ++i)
    {
        if (ages[i] > 0)
        {
            edr.push_back(n * (ages[i] - time));
            time = ages[i];
            --n;
        }
    }
    return edr;
}
      
