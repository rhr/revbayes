//
//  AdmixtureEdgeAddResidualWeights.cpp
//  revbayes_mlandis
//
//  Created by Michael Landis on 1/21/13.
//  Copyright (c) 2013 Michael Landis. All rights reserved.
//

#include "AdmixtureEdgeAddResidualWeights.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "TopologyNode.h"
#include "TreeUtilities.h"

#include <cmath>

using namespace RevBayesCore;

AdmixtureEdgeAddResidualWeights::AdmixtureEdgeAddResidualWeights(StochasticNode<AdmixtureTree> *v, StochasticNode<double>* r, DeterministicNode<std::vector<double> >* res, ConstantNode<int>* dt, double w) : Move( v, w), variable( v ), rate(r), residuals(res), delayTimer(dt), changed(false), failed(false) {
    
    nodes.insert(rate);
    nodes.insert(residuals);
    nodes.insert(dt);

}



/* Clone object */
AdmixtureEdgeAddResidualWeights* AdmixtureEdgeAddResidualWeights::clone( void ) const {
    
    return new AdmixtureEdgeAddResidualWeights( *this );
}



const std::string& AdmixtureEdgeAddResidualWeights::getMoveName( void ) const {
    static std::string name = "AdmixtureEdgeAddResidualWeights";
    
    return name;
}

void AdmixtureEdgeAddResidualWeights::findNewBrothers(std::vector<AdmixtureNode *> &b, AdmixtureNode &p, AdmixtureNode *n) {
    
    if (&p != n) {
        
        size_t numChildren = n->getNumberOfChildren();
        AdmixtureNode* child;
        
        for (size_t i = 0; i < numChildren; i++)
        {
            child = &n->getChild( i );
            if ( child->getAge() < p.getAge() ) {
                b.push_back( child );
            } else {
                findNewBrothers(b, p, child);
            }
        }
    }
}

/** Perform the move */
double AdmixtureEdgeAddResidualWeights::performSimpleMove( void ) {
    
    std::cout << "add_RW\n";
    
    failed = false;
    
    // Get random number generator
    RandomNumberGenerator* rng     = GLOBAL_RNG;
    
    // sample a random position on the tree
    AdmixtureTree& tau = variable->getValue();
    double treeLength = tau.getTreeLength();
    double u = rng->uniform01() * treeLength;
    size_t numNodes = tau.getNumberOfNodes();
    
    //std::cout << "\nAEA\n";
    //std::cout << "before AEA proposal\n";
    // tau.checkAllEdgesRecursively(&tau.getRoot());
    
    // record position
    double v = 0.0;
    double admixtureAge = 0.0;
    double branchLength = 0.0;
    AdmixtureNode* child = NULL;
    AdmixtureNode* parent = NULL;
    for (size_t i = 0; i < numNodes; i++)
    {
        child = &tau.getNode(i);
        if (!child->isRoot())
        {
            parent = &child->getParent();
            branchLength = parent->getAge() - child->getAge();
            v += branchLength;
            if (v > u)
            {
                admixtureAge = child->getAge() - (u - v);
                break;
            }
        }
    }
    
    storedAdmixtureAge = admixtureAge;
    
    // store adjacent nodes to new parent node
    storedAdmixtureParentChild = child;
    storedAdmixtureParentParent = parent;
    
    // insert admixtureParent into graph
    storedAdmixtureParent = new AdmixtureNode();
    storedAdmixtureParent->setAge(admixtureAge);
    storedAdmixtureParent->setParent(storedAdmixtureParentParent);
    storedAdmixtureParent->addChild(storedAdmixtureParentChild);
    storedAdmixtureParentParent->removeChild(storedAdmixtureParentChild);
    storedAdmixtureParentParent->addChild(storedAdmixtureParent);
    storedAdmixtureParentChild->setParent(storedAdmixtureParent);
    tau.pushAdmixtureNode(storedAdmixtureParent);
    
    // sample random position for admixture edge destination for a given tree height
    std::vector<AdmixtureNode*> brothers;
    findNewBrothers(brothers, *storedAdmixtureParent, &tau.getRoot());
    /*
     if (brothers.size() == 0)
     {
     failed = true;
     return RbConstants::Double::neginf;
     }
     */
    int index = int(rng->uniform01() * brothers.size());
    
    // store adjacent nodes to new child node
    storedAdmixtureChildChild = brothers[index];
    storedAdmixtureChildParent = &storedAdmixtureChildChild->getParent();
    
    // insert admixtureChild into graph
    storedAdmixtureChild = new AdmixtureNode();
    storedAdmixtureChild->setAge(admixtureAge);
    storedAdmixtureChild->setParent(storedAdmixtureChildParent);
    storedAdmixtureChild->addChild(storedAdmixtureChildChild);
    storedAdmixtureChildParent->removeChild(storedAdmixtureChildChild);
    storedAdmixtureChildParent->addChild(storedAdmixtureChild);
    storedAdmixtureChildChild->setParent(storedAdmixtureChild);
    double w = rng->uniform01() * 0.5;
    storedAdmixtureChild->setWeight(w);
    tau.pushAdmixtureNode(storedAdmixtureChild);
    
    // create admixture edge
    storedAdmixtureChild->setAdmixtureParent(storedAdmixtureParent);
    storedAdmixtureParent->setAdmixtureChild(storedAdmixtureChild);
    
    //std::cout << "\nafter AEA proposal\n";
    //tau.checkAllEdgesRecursively(&tau.getRoot());
    
    // prior * proposal ratio
    size_t numEvents = tau.getNumberOfAdmixtureChildren();
    double unitTreeLength = tau.getUnitTreeLength();
    double lnP = log( (rate->getValue() * unitTreeLength) / (numEvents) );
    //  double lnJ = 2 * log(1 - w);
    //    double lnP = log( (rate->getValue() * treeLength) / (numEvents) );
    std::cout << "add\t" << lnP << "\t" << rate->getValue() << "\t" << unitTreeLength << "\t" << numEvents << "\n";
    
    // quick fix -- should inherit from Move instead of SimpleMove
    //lnP -= variable->getLnProbabilityRatio();0
    
    // bombs away
    return lnP;// + lnJ;
    
}


void AdmixtureEdgeAddResidualWeights::rejectSimpleMove( void ) {
    
    if (!failed)
    {
        // revert edges
        storedAdmixtureChildChild->setParent(storedAdmixtureChildParent);
        storedAdmixtureChildParent->removeChild(storedAdmixtureChild);
        storedAdmixtureChildParent->addChild(storedAdmixtureChildChild);
        storedAdmixtureParentChild->setParent(storedAdmixtureParentParent);
        storedAdmixtureParentParent->removeChild(storedAdmixtureParent);
        storedAdmixtureParentParent->addChild(storedAdmixtureParentChild);
        
        // remove nodes from graph structure
        AdmixtureTree& tau = variable->getValue();
        
        tau.eraseAdmixtureNode(storedAdmixtureParent);
        tau.eraseAdmixtureNode(storedAdmixtureChild);
        
        // std::cout << "add fail\t" << tau.getNumberOfAdmixtureChildren() << "\n";
        //std::cout << "\nafter AEA reject\n";
        //tau.checkAllEdgesRecursively(&tau.getRoot());
    }
    
}

void AdmixtureEdgeAddResidualWeights::acceptSimpleMove(void)
{
    // std::cout << "add success";
    //  std::cout << "\t" << variable->getValue().getNumberOfAdmixtureChildren();
    //  std::cout << "\n";
}


void AdmixtureEdgeAddResidualWeights::swapNode(DagNode *oldN, DagNode *newN) {
    // call the parent method
    Move::swapNode(oldN, newN);
    
    if (oldN == variable)
    {
        //std::cout << "AEA\ttau\n";
        variable = static_cast<StochasticNode<AdmixtureTree>* >(newN) ;
    }
    else if (oldN == rate)
    {
        //std::cout << "AEA\trate\n";
        rate = static_cast<StochasticNode<double>* >(newN);
    }
    else if (oldN == delayTimer)
    {
        delayTimer = static_cast<ConstantNode<int>* >(newN);
    }
    else if (oldN == residuals)
    {
        residuals = static_cast<DeterministicNode<std::vector<double> >* >(newN);
    }
}


void AdmixtureEdgeAddResidualWeights::rejectMove( void ) {
    changed = false;
    
    // delegate to the derived class. The derived class needs to restore the value(s).
    rejectSimpleMove();
    
    // touch the node
    variable->touch();
}


void AdmixtureEdgeAddResidualWeights::acceptMove( void ) {
    // nothing to do
    changed = false;
    
    acceptSimpleMove();
}

double AdmixtureEdgeAddResidualWeights::performMove( double &probRatio ) {
    
    
    if (delayTimer->getValue() > 0)
    {
        failed = true;
        //delayTimer->setValue(delayTimer->getValue()-1);
        //        delay--;
        return RbConstants::Double::neginf;
    }
    
    if (changed) {
        throw RbException("Trying to execute a simple moves twice without accept/reject in the meantime.");
    }
    changed = true;
    
    double hr = performSimpleMove();
    
    if ( hr != hr || hr == RbConstants::Double::inf ) {
        return RbConstants::Double::neginf;
    }
    
    // touch the node
    variable->touch();
    
    probRatio = 0.0;
    if ( probRatio != RbConstants::Double::inf && probRatio != RbConstants::Double::neginf ) {
        
        std::set<DagNode* > affectedNodes;
        variable->getAffectedNodes(affectedNodes);
        for (std::set<DagNode* >::iterator i=affectedNodes.begin(); i!=affectedNodes.end(); ++i) {
            DagNode* theNode = *i;
            probRatio += theNode->getLnProbabilityRatio();
            std::cout << theNode->getName() << "\t" << probRatio << "\n";
        }
    }
    
    return hr;
}
