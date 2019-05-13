#ifndef SRC_CORE_MONITORS_NEXUSMONITOR_H_
#define SRC_CORE_MONITORS_NEXUSMONITOR_H_

#include "AbstractFileMonitor.h"

#include <TypedDagNode.h>

namespace RevBayesCore {

/** @brief Monitor to output a distribution of trees in Nexus format.
 * The provided branch- or node-specific variables will be output on the trees as metadata.
 *
 * @see ExtendedNewickTreeMonitor for output in table format
* */
class NexusMonitor: public AbstractFileMonitor {
public:
    NexusMonitor(TypedDagNode<Tree> *t, const std::vector<DagNode*> &n, bool np, unsigned long g, const std::string &fname,
                 bool ap = false, bool taxa = true);
    NexusMonitor* clone(void) const;

    virtual void printHeader();
    virtual void monitor(unsigned long gen);
    virtual void closeStream();

protected:
    bool isNodeParameter;  //!< whether data is on the nodes or branches
    bool writeTaxa;  //!< whether to write a taxa block
    TypedDagNode<Tree>* tree;  //!< monitored tree
    std::vector<DagNode*> nodeVariables;  //!< variables associated with the tree
};

} /* namespace RevBayesCore */

#endif /* SRC_CORE_MONITORS_NEXUSMONITOR_H_ */
