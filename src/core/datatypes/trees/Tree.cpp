/**
 * @file
 * This file contains the implementation of a time-calibrated tree. This version of a tree
 * contains nodes with times.
 *
 * @brief Implementation of a time calibrated tree
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-07-05 16:47:08 +0200 (Thu, 05 Jul 2012) $
 * @author The RevBayes core development team
 * @license GPL version 3
 * @version 1.0
 * @since 2012-07-17, version 1.0
 *
 * $Id: Tree.cpp 1651 2012-07-05 14:47:08Z hoehna $
 */

#include "Tree.h"
#include "RbException.h"
#include "RbOptions.h"
#include "Topology.h"
#include "TopologyNode.h"

using namespace RevBayesCore;

/* Default constructor */
Tree::Tree(void) : topology( NULL ) {
    
}


/* Copy constructor */
Tree::Tree(const Tree& t) : changeEventHandler( t.changeEventHandler ) {
    
    // set the topology
    topology      = t.topology;
    
    // set the tree for each node
    topology->getNodes()[topology->getNumberOfNodes()-1]->setTopology( this );
}


/* Destructor */
Tree::~Tree(void) {
    // nothing to do
}


Tree& Tree::operator=(const Tree &t) {
    
    if (this != &t) {
        // nothing really to do here, should be done in the derived classes
        // @TODO: Find a better solution - Sebastian
        // Problem: If we redraw the tree because the initial states are invalid, 
        // then we somehow need to remember the tree event change listeners.
        // But it is not nice if the tree distribution needs to remember this!!!
//        changeEventHandler = t.changeEventHandler;
        
        topology = t.topology;
        
        // set the tree for each node
        topology->getNodes()[topology->getNumberOfNodes()-1]->setTopology( this );
    }
    
    return *this;
}


bool Tree::operator==(const Tree &t) const {
    return getNewickRepresentation() == t.getNewickRepresentation();
}


bool Tree::operator!=(const Tree &t) const {
    return !operator==(t);
}


bool Tree::operator<(const Tree &t) const {
    return getNewickRepresentation() < t.getNewickRepresentation();
}


void Tree::addBranchParameter(std::string const &name, const std::vector<double> &parameters, bool internalOnly) {
    getRoot().addBranchParameter(name,parameters,internalOnly);
}


void Tree::addNodeParameter(std::string const &name, const std::vector<double> &parameters, bool internalOnly) {
    getRoot().addParameter(name,parameters,internalOnly);
}


void Tree::clearBranchParameters() {
    getRoot().clearBranchParameters();
}

std::vector<std::string> Tree::getNames() const {
    return topology->getNames();
}

TopologyNode& Tree::getNode(size_t idx) {
    return *topology->getNodes()[ idx ];
}

const TopologyNode& Tree::getNode(size_t idx) const {
    return topology->getNode( idx );
}

std::vector<TopologyNode*> Tree::getNodes(void) const {
    return topology->getNodes();
}

size_t Tree::getNumberOfInteriorNodes(void) const {
    return topology->getNumberOfInteriorNodes();
}

size_t Tree::getNumberOfNodes(void) const {
    return topology->getNumberOfNodes();
}

size_t Tree::getNumberOfTips(void) const {
    return topology->getNumberOfTips();
}

const TopologyNode& Tree::getInteriorNode(size_t indx) const {
    return topology->getInteriorNode( indx );
}

const std::string& Tree::getNewickRepresentation() const {
    return topology->getNewickRepresentation();
}

TopologyNode& Tree::getRoot(void) {
    return *topology->getNodes()[topology->getNumberOfNodes()-1];
}

const TopologyNode& Tree::getRoot(void) const {
    return topology->getRoot();
}

TopologyNode& Tree::getTipNode(size_t indx) {
    return *topology->getNodes()[indx];
}

const TopologyNode& Tree::getTipNode(size_t indx) const {
    return topology->getTipNode(indx);
}


double Tree::getTmrca(const TopologyNode &n) {
    return topology->getRoot().getTmrca( n );
}

//TreeChangeEventHandler& Tree::getTreeChangeEventHandler(void) const;//!< Get the change-event handler for this tree

bool Tree::isBinary(void) const {
    return topology->isBinary();
}

bool Tree::isRooted(void) const {
    return topology->isRooted();
}

//void Tree::setRooted(bool tf) {
//    throw RbException("Cannot set rootedness of tree. This needs to be done in the topology!!!");
//}
//
//void Tree::setRoot(TopologyNode* r) {
//    throw RbException("Cannot set root of tree. This needs to be done in the topology!!!");
//}


TreeChangeEventHandler& Tree::getTreeChangeEventHandler( void ) const {
    return changeEventHandler;
}


void Tree::setTopology(const Topology *t) {
    // set the topology of this tree
    topology = t;
    
    // set the tree for each node
    topology->getNodes()[topology->getNumberOfNodes()-1]->setTopology( this );
    
    resizeElementVectors( t->getNumberOfNodes() );
}

std::ostream& RevBayesCore::operator<<(std::ostream& o, const Tree& x) {
    o << x.getNewickRepresentation();
    
    return o;
}
