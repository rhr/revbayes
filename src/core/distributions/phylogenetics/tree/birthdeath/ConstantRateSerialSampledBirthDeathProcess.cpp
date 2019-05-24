#include "ConstantRateSerialSampledBirthDeathProcess.h"
#include "DistributionExponential.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbMathCombinatorialFunctions.h"
#include "StochasticNode.h"

#include <cmath>


using namespace RevBayesCore;


/**
 * Constructor.
 * We delegate most parameters to the base class and initialize the members.
 *
 * \param[in]    o              Time of the origin/present/length of the process.
 * \param[in]    ra             Age or the root (=time of the process).
 * \param[in]    s              Speciation rate.
 * \param[in]    e              Extinction rate.
 * \param[in]    p              Extinction sampling rate.
 * \param[in]    r              Sampling probability at present time.
 * \param[in]    cdt            Condition of the process (none/survival/#Taxa).
 * \param[in]    tn             Taxa.
 * \param[in]    c              Clades conditioned to be present.
 */
ConstantRateSerialSampledBirthDeathProcess::ConstantRateSerialSampledBirthDeathProcess( const TypedDagNode<double> *o,
                                                                                  const TypedDagNode<double> *s,
                                                                                  const TypedDagNode<double> *e,
                                                                                  const TypedDagNode<double> *p,
                                                                                  const TypedDagNode<double> *r,
                                                                                  const std::string& cdt,
                                                                                  const std::vector<Taxon> &tn,
                                                                                  bool uo,
                                                                                  TypedDagNode<Tree> *t) : AbstractBirthDeathProcess( o, cdt, tn, uo ),
    lambda( s ),
    mu( e ),
    psi( p ),
    rho( r )
{
    addParameter( lambda );
    addParameter( mu );
    addParameter( psi );
    addParameter( rho );

    if (t != NULL)
    {
      delete value;
      value = &(t->getValue());
    }
    else
    {
      simulateTree();
    }

}

/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'B'.
 *
 * \return A new copy of myself
 */
ConstantRateSerialSampledBirthDeathProcess* ConstantRateSerialSampledBirthDeathProcess::clone( void ) const
{

    return new ConstantRateSerialSampledBirthDeathProcess( *this );
}


/**
 * Compute the log-transformed probability of the current value under the current parameter values.
 *
 */
double ConstantRateSerialSampledBirthDeathProcess::computeLnProbabilityDivergenceTimes( void ) const
{
    // prepare the probability computation
    prepareProbComputation();

    // variable declarations and initialization
    double lnProbTimes = computeLnProbabilityTimes();

    return lnProbTimes;
}


/**
 * Compute the log-transformed probability of the current value under the current parameter values.
 *
 * \return    The log-probability density.
 */
double ConstantRateSerialSampledBirthDeathProcess::computeLnProbabilityTimes( void ) const
{

    double lnProbTimes = 0.0;
    double process_time = getOriginAge();
    size_t num_initial_lineages = 2;
    const TopologyNode& root = value->getRoot();

    if (use_origin) {
        // If we are conditioning on survival from the origin,
        // then we must divide by 2 the log survival probability computed by AbstractBirthDeathProcess
        num_initial_lineages = 1;
    }

    // if conditioning on root, root node must be a "true" bifurcation event
    else if (root.getChild(0).isSampledAncestor() || root.getChild(1).isSampledAncestor())
    {
        return RbConstants::Double::neginf;
    }

    // variable declarations and initialization
    double birth_rate = lambda->getValue();
    double death_rate = mu->getValue();
    double serial_rate = psi->getValue();
    double sampling_prob = rho->getValue();

    // get helper variables
    double a = birth_rate - death_rate - serial_rate;
    double c1 = std::fabs(sqrt(a * a + 4 * birth_rate * serial_rate));
    double c2 = -(a - 2 * birth_rate * sampling_prob) / c1;

    // get node/time variables
    size_t num_nodes = value->getNumberOfNodes();

    // classify nodes
    int num_sampled_ancestors = 0;
    int num_extant_taxa = 0;

    std::vector<double> serial_tip_ages = std::vector<double>();
    std::vector<double> internal_node_ages = std::vector<double>();
    for (size_t i = 0; i < num_nodes; i++)
    {
        const TopologyNode& n = value->getNode( i );

        if ( n.isFossil() && n.isSampledAncestor() )
        {
            // node is sampled ancestor
            num_sampled_ancestors++;
        }
        else if ( n.isFossil() && !n.isSampledAncestor() )
        {
            // node is serial leaf
            serial_tip_ages.push_back( n.getAge() );
        }
        else if ( n.isTip() && !n.isFossil() )
        {
            // node is extant leaf
            num_extant_taxa++;
        }
        else if ( n.isInternal() && !n.getChild(0).isSampledAncestor() && !n.getChild(1).isSampledAncestor() )
        {
            if (!n.isRoot() || use_origin)
            {
                // node is bifurcation event (a "true" node)
                internal_node_ages.push_back( n.getAge() );
            }
        }
    }

    // add the log probability for the serial sampling events
    if (serial_rate == 0.0)
    {
        if ( serial_tip_ages.size() + num_sampled_ancestors > 0 )
        {
            return RbConstants::Double::neginf;
            //throw RbException("The serial sampling rate is zero, but the tree has serial sampled tips.");
        }
    }
    else
    {
        lnProbTimes += (serial_tip_ages.size() + num_sampled_ancestors) * log( serial_rate );
    }

    // add the log probability for sampling the extant taxa
    if (num_extant_taxa > 0)
    {
        lnProbTimes += num_extant_taxa * log( 4.0 * sampling_prob );
    }

    // add the log probability of the initial sequences
    lnProbTimes += -lnQ(process_time, c1, c2) * num_initial_lineages;

    // add the log probability for the internal node ages
    lnProbTimes += internal_node_ages.size() * log( birth_rate );
    for (size_t i=0; i<internal_node_ages.size(); i++)
    {
        lnProbTimes -= lnQ(internal_node_ages[i], c1, c2);
    }

    // add the log probability for the serial tip ages
    for (size_t i=0; i < serial_tip_ages.size(); i++)
    {
        double t = serial_tip_ages[i];
        lnProbTimes += log(pZero(t, c1, c2)) + lnQ(t, c1, c2);
    }

    // condition on survival
    if ( condition == "survival")
    {
        lnProbTimes -= num_initial_lineages * log(1.0 - pHatZero(process_time));
    }
    // condition on nTaxa
    else if ( condition == "nTaxa" )
    {
        lnProbTimes -= lnProbNumTaxa( value->getNumberOfTips(), 0, process_time, true );
    }

    return lnProbTimes;

}


double ConstantRateSerialSampledBirthDeathProcess::lnProbTreeShape(void) const
{
    // the birth death divergence times density is derived for a (ranked) unlabeled oriented tree
    // so we convert to a (ranked) labeled non-oriented tree probability by multiplying by 2^{n+m-1} / n!
    // where n is the number of extant tips, m is the number of sampled extinct tips

    int num_taxa = (int)value->getNumberOfTips();
    int num_extinct = (int)value->getNumberOfExtinctTips();
    int num_sa = (int)value->getNumberOfSampledAncestors();

    return (num_taxa - num_sa - 1) * RbConstants::LN2 - RbMath::lnFactorial(num_taxa - num_extinct);
}


/**
 * Compute the probability of survival if the process starts with one species at time start and ends at time end.
 *
 *
 * \param[in]    start      Start time of the process.
 * \param[in]    end        End/stopping time of the process.
 *
 * \return Speciation rate at time t.
 */
double ConstantRateSerialSampledBirthDeathProcess::pSurvival(double start, double end) const
{
    return 1.0 - pHatZero(end);
}



/**
 * Simulate new speciation times.
 */
double ConstantRateSerialSampledBirthDeathProcess::simulateDivergenceTime(double origin, double present) const
{

    // incorrect placeholder for constant FBDP

    // Get the rng
    RandomNumberGenerator* rng = GLOBAL_RNG;

    // get the parameters
    double age = origin - present;
    double b = lambda->getValue();
    double d = mu->getValue();
    double r = rho->getValue();

    // get a random draw
    double u = rng->uniform01();


    // compute the time for this draw
    // see Hartmann et al. 2010 and Stadler 2011
    double t = 0.0;
    if ( b > d )
    {
        t = ( log( ( (b-d) / (1 - (u)*(1-((b-d)*exp((d-b)*age))/(r*b+(b*(1-r)-d)*exp((d-b)*age) ) ) ) - (b*(1-r)-d) ) / (r * b) ) )  /  (b-d);
    }
    else
    {
        t = ( log( ( (b-d) / (1 - (u)*(1-(b-d)/(r*b*exp((b-d)*age)+(b*(1-r)-d) ) ) ) - (b*(1-r)-d) ) / (r * b) ) )  /  (b-d);
    }

    return present + t;
}


double ConstantRateSerialSampledBirthDeathProcess::pZero(double t, double c1, double c2) const
{
    double b = lambda->getValue();
    double d = mu->getValue();
    double f = psi->getValue();
    double v1 = exp(-c1*t) * (1.0-c2) - (1.0+c2);
    double v2 = exp(-c1*t) * (1.0-c2) + (1.0+c2);
    double v = (b + d + f + c1 * (v1 / v2)) / (2.0 * b);
    return v;
}


double ConstantRateSerialSampledBirthDeathProcess::lnQ(double t, double c1, double c2) const
{
    //return log( 2*(1-c2*c2) + exp(-c1*t)*(1-c2)*(1-c2) + exp(c1*t)*(1+c2)*(1+c2) );

    // numerically safe code
    return c1*t + 2 * log( exp(-c1*t) * (1-c2) + (1+c2));
}


double ConstantRateSerialSampledBirthDeathProcess::pHatZero(double t) const
{
    double b = lambda->getValue();
    double d = mu->getValue();
    double r = rho->getValue();
    double val = r * (b-d) / (b*r + (b*(1-r)-d)*exp(-(b-d)*t));
    return 1.0 - val;
}


/**
 * Swap the parameters held by this distribution.
 *
 *
 * \param[in]    oldP      Pointer to the old parameter.
 * \param[in]    newP      Pointer to the new parameter.
 */
void ConstantRateSerialSampledBirthDeathProcess::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    if (oldP == lambda)
    {
        lambda = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == mu)
    {
        mu = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == psi)
    {
        psi = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == rho)
    {
        rho = static_cast<const TypedDagNode<double>* >( newP );
    }
    else
    {
        // delegate the super-class
        AbstractBirthDeathProcess::swapParameterInternal(oldP, newP);
    }

}
