#include "DistributionExponential.h"
#include "EpisodicBirthDeathSamplingTreatmentProcess.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbMathCombinatorialFunctions.h"
#include "RbMathLogic.h"
#include "StochasticNode.h"

#include <algorithm>
#include <cmath>

using namespace RevBayesCore;

/**
 * Constructor.
 * We delegate most parameters to the base class and initialize the members.
 *
 * \param[in]    s              Speciation rates.
 * \param[in]    e              Extinction rates.
 * \param[in]    p              Serial sampling rates.
 * \param[in]    r              Instantaneous sampling probabilities.
 * \param[in]    t              Rate change times.
 * \param[in]    cdt            Condition of the process (none/survival/#Taxa).
 * \param[in]    tn             Taxa.
 * \param[in]    c              Clades conditioned to be present.
 */
EpisodicBirthDeathSamplingTreatmentProcess::EpisodicBirthDeathSamplingTreatmentProcess(const TypedDagNode<double> *ra,
                                                                                           const DagNode *inspeciation,
                                                                                           const DagNode *inextinction,
                                                                                           const DagNode *inserialsampling,
                                                                                           const DagNode *intreatment,
                                                                                           const DagNode *ineventspeciation,
                                                                                           const DagNode *ineventextinction,
                                                                                           const DagNode *ineventsampling,
                                                                                           const TypedDagNode< RbVector<double> > *ht,
                                                                                           const std::string &cdt,
                                                                                           const std::vector<Taxon> &tn,
                                                                                           bool uo,
                                                                                           TypedDagNode<Tree> *t) : AbstractBirthDeathProcess( ra, cdt, tn, uo ),
    interval_times(ht),
    offset( 0.0 )
{
    // initialize all the pointers to NULL
    homogeneous_lambda   = NULL;
    homogeneous_mu       = NULL;
    homogeneous_phi      = NULL;
    homogeneous_r        = NULL;
    // homogeneous_Lambda   = NULL;
    // homogeneous_Mu       = NULL;
    homogeneous_Phi      = NULL;
    heterogeneous_lambda = NULL;
    heterogeneous_mu     = NULL;
    heterogeneous_phi    = NULL;
    heterogeneous_r      = NULL;
    heterogeneous_Lambda = NULL;
    heterogeneous_Mu     = NULL;
    heterogeneous_Phi    = NULL;

    std::vector<double> times = timeline;
    std::vector<double> times_sorted_ascending = times;

    sort(times_sorted_ascending.begin(), times_sorted_ascending.end() );

    if ( times != times_sorted_ascending )
    {
        throw(RbException("Rate change times must be provided in ascending order."));
    }

    addParameter( interval_times );

    heterogeneous_lambda = dynamic_cast<const TypedDagNode<RbVector<double> >*>(inspeciation);
    homogeneous_lambda = dynamic_cast<const TypedDagNode<double >*>(inspeciation);

    addParameter( homogeneous_lambda );
    addParameter( heterogeneous_lambda );

    heterogeneous_mu = dynamic_cast<const TypedDagNode<RbVector<double> >*>(inextinction);
    homogeneous_mu = dynamic_cast<const TypedDagNode<double >*>(inextinction);

    addParameter( homogeneous_mu );
    addParameter( heterogeneous_mu );

    heterogeneous_phi = dynamic_cast<const TypedDagNode<RbVector<double> >*>(inserialsampling);
    homogeneous_phi = dynamic_cast<const TypedDagNode<double >*>(inserialsampling);

    addParameter( homogeneous_phi );
    addParameter( heterogeneous_phi );

    heterogeneous_r = dynamic_cast<const TypedDagNode<RbVector<double> >*>(intreatment);
    homogeneous_r = dynamic_cast<const TypedDagNode<double >*>(intreatment);

    addParameter( homogeneous_r );
    addParameter( heterogeneous_r );

    heterogeneous_Lambda = dynamic_cast<const TypedDagNode<RbVector<double> >*>(ineventspeciation);
    // homogeneous_Lambda = dynamic_cast<const TypedDagNode<double >*>(ineventspeciation);

    // addParameter( homogeneous_Lambda );
    addParameter( heterogeneous_Lambda );

    heterogeneous_Mu = dynamic_cast<const TypedDagNode<RbVector<double> >*>(ineventextinction);
    // homogeneous_Mu = dynamic_cast<const TypedDagNode<double >*>(ineventextinction);

    // addParameter( homogeneous_Mu );
    addParameter( heterogeneous_Mu );

    heterogeneous_Phi = dynamic_cast<const TypedDagNode<RbVector<double> >*>(ineventsampling);
    homogeneous_Phi = dynamic_cast<const TypedDagNode<double >*>(ineventsampling);

    addParameter( homogeneous_Phi );
    addParameter( heterogeneous_Phi );

    //TODO: make sure the offset is added properly into the computation, need to offset *all* times, including interval times
    //          thie means we also need to check that the first interval time is not less than the first tip (which we should probably to anyways)

    //TODO: returning neginf and nan are not currently coherent

    //check that lengths of vector arguments are sane
    if ( heterogeneous_lambda != NULL && !(interval_times->getValue().size() == heterogeneous_lambda->getValue().size() - 1) )
    {
      throw(RbException("If provided as a vector, argument lambda must have one more element than timeline."));
    }

    if ( heterogeneous_mu != NULL && !(interval_times->getValue().size() == heterogeneous_mu->getValue().size() - 1) )
    {
      throw(RbException("If provided as a vector, argument mu must have one more element than timeline."));
    }

    if ( heterogeneous_phi != NULL && !(interval_times->getValue().size() == heterogeneous_phi->getValue().size() - 1) )
    {
      throw(RbException("If provided as a vector, argument phi must have one more element than timeline."));
    }

    if ( heterogeneous_r != NULL && !(interval_times->getValue().size() == heterogeneous_r->getValue().size() - 1) )
    {
      throw(RbException("If provided as a vector, argument r must have one more element than timeline."));
    }

    if ( heterogeneous_Lambda != NULL && !(interval_times->getValue().size() == heterogeneous_Lambda->getValue().size()) )
    {
      throw(RbException("If provided, argument Lambda must be of same length as timeline."));
    }

    if ( heterogeneous_Mu != NULL && !(interval_times->getValue().size() == heterogeneous_Mu->getValue().size()) )
    {
      throw(RbException("If provided, argument Mu must be of same length as timeline."));
    }

    if ( heterogeneous_Phi != NULL && !(interval_times->getValue().size() == heterogeneous_Phi->getValue().size() - 1) )
    {
      throw(RbException("If provided as a vector, argument Phi must have one more element than timeline."));
    }

    updateVectorParameters();
    prepareProbComputation();

    if (t != NULL)
    {
      delete value;
      value = &(t->getValue());
    }
    else
    {
      simulateTree();
    }

    countAllNodes();

}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'B'.
 *
 * \return A new copy of myself
 */
EpisodicBirthDeathSamplingTreatmentProcess* EpisodicBirthDeathSamplingTreatmentProcess::clone( void ) const
{
    return new EpisodicBirthDeathSamplingTreatmentProcess( *this );
}

/**
 * Compute the log-transformed probability of the current value under the current parameter values.
 *
 */
double EpisodicBirthDeathSamplingTreatmentProcess::computeLnProbabilityDivergenceTimes( void ) const
{
    // update parameter vectors
    updateVectorParameters();

    // Assign nodes to sets
    countAllNodes();

    if ( offset > DBL_EPSILON && phi_event[0] > DBL_EPSILON )
    {
      throw(RbException("Event sampling fraction at the present is non-zero but there are no tips at the present."));
    }

    // precompute A_i, B_i, C_i, E_i(t_i)
    prepareProbComputation();

    // variable declarations and initialization
    double lnProbTimes = computeLnProbabilityTimes();

    return lnProbTimes;
}


/**
 * Compute the log probability of the current value under the current parameter values.
 */
double EpisodicBirthDeathSamplingTreatmentProcess::computeLnProbabilityTimes( void ) const
{
    // variable declarations and initialization
    double lnProbTimes = 0.0;
    double process_time = getOriginAge();
    size_t num_initial_lineages = 0;
    TopologyNode* root = &value->getRoot();

    if ( use_origin == true )
    {
        // If we are conditioning on survival from the origin,
        // then we must divide by 2 the log survival probability computed by AbstractBirthDeathProcess
        // TODO: Generalize AbstractBirthDeathProcess to allow conditioning on the origin
        num_initial_lineages = 1;
    }
    // if conditioning on root, root node must be a "true" bifurcation event
    else
    {
        if (root->getChild(0).isSampledAncestor() || root->getChild(1).isSampledAncestor())
        {
            return RbConstants::Double::neginf;
        }

        num_initial_lineages = 2;
    }

    // get node/time variables
    size_t num_nodes = value->getNumberOfNodes();


    // add the event-sampling terms (ii)
    for (size_t i = 0; i < timeline.size(); ++i)
    {
        // Only compute sampling probability when there is a sampling event
        if (phi_event[i] > DBL_EPSILON)
        {
            if ( RbMath::isFinite(lnProbTimes) == false )
            {
                // throw(RbException("nan-likelihood in (ii)"));
                return RbConstants::Double::nan;
            }

            // Calculate probability of the samples
            double ln_sampling_event_prob = 0.0;
            int R_i = int(event_sampled_ancestor_ages[i].size());
            int N_i = R_i + int(event_tip_ages[i].size());
            int active_lineages_at_t = survivors(timeline[i]); //A(t_{\rho_i})

            if (N_i == 0)
            {
                return RbConstants::Double::neginf;
                //std::stringstream ss;
                //ss << "The event sampling rate at timeline[ " << i << "] is > 0, but the tree has no samples at this time.";
                //throw RbException(ss.str());
            }

            // Make sure that we aren't claiming to have sampled all lineages without having sampled all lineages
            if (phi_event[i] >= (1.0 - DBL_EPSILON) && (active_lineages_at_t != N_i) )
            {
                return RbConstants::Double::neginf;
                //std::stringstream ss;
                //ss << "The event sampling rate at timeline[ " << i << "] is one, but the tree has unsampled tips at this time.";
                //throw RbException(ss.str());

            }
            else
            {
                ln_sampling_event_prob += N_i * log(phi_event[i]);
                if ( (active_lineages_at_t - N_i) > 0 )
                {
                    ln_sampling_event_prob += (active_lineages_at_t - N_i) * log(1 - phi_event[i]);
                }
            }

            // Calculate probability of the sampled ancestors
            if ( r[i] >= (1.0 - DBL_EPSILON) && R_i > 0 )
            {
                // Cannot have sampled ancestors if r(t) == 1
                return RbConstants::Double::neginf;
                //std::stringstream ss;
                //ss << "The conditional probability of death on sampling rate in interval " << i << " is one, but the tree has sampled ancesors in this interval.";
                //throw RbException(ss.str());
            }
            if ( timeline[i] > DBL_EPSILON )
            {
                // only add these terms for sampling that is not at the present
                ln_sampling_event_prob += R_i * log(1 - r[i]);
                ln_sampling_event_prob += (N_i - R_i) * log(r[i] * (1 - r[i])*E(i,timeline[i]));
            }
            lnProbTimes += ln_sampling_event_prob;
        }
    }

    // add the serial tip age terms (iii)
    for (size_t i = 0; i < serial_tip_ages.size(); ++i)
    {
        if ( RbMath::isFinite(lnProbTimes) == false )
        {
            return RbConstants::Double::nan;
        }

        double t = serial_tip_ages[i];
        size_t index = findIndex(t);

        // add the log probability for the serial sampling events
        if ( phi[index] == 0.0 )
        {
            return RbConstants::Double::neginf;
            //std::stringstream ss;
            //ss << "The serial sampling rate in interval " << i << " is zero, but the tree has serial sampled tips in this interval.";
            //throw RbException(ss.str());
        }
        else
        {
            double this_prob = r[index] + (1 - r[index]) * E(index,t);
            this_prob *= phi[index];
            // double this_prob = phi[index] * r[index];
            // // Avoid computation in the case of r = 1
            // if (r[t] <= 1 - DBL_EPSILON)
            // {
            //   this_prob += phi[index] * (1 - r[index]) * E(index,t);
            // }
            lnProbTimes += log( this_prob );
        }
    }
// std::cout << "computed (iii); lnProbability = " << lnProbTimes << std::endl;

    // add the serial sampled ancestor terms (iv)
    for (size_t i=0; i < serial_sampled_ancestor_ages.size(); ++i)
    {
        if ( RbMath::isFinite(lnProbTimes) == false )
        {
            return RbConstants::Double::nan;
        }

        double t = serial_sampled_ancestor_ages[i];
        size_t index = findIndex(t);

        if ( r[index] > 1.0 - DBL_EPSILON )
        {
            return RbConstants::Double::neginf;
            //std::stringstream ss;
            //ss << "The conditional probability of death on sampling rate in interval " << i << " is one, but the tree has sampled ancesors in this interval.";
            //throw RbException(ss.str());
        }

        lnProbTimes += log(phi[index]) + log(1 - r[index]);

    }
// std::cout << "computed (iv); lnProbability = " << lnProbTimes << std::endl;

    // add the burst bifurcation age terms (v)
    for (size_t i = 0; i < timeline.size(); ++i)
    {
        // Nothing to compute if the burst probability is 0
        if (lambda_event[i] > DBL_EPSILON)
        {
            lnProbTimes += event_bifurcation_times[i].size() * log(lambda_event[i]);
            int active_lineages_at_t = survivors(timeline[i]); //A(t_{\rho_i})
            int A_minus_K = active_lineages_at_t - int(event_bifurcation_times[i].size());
            lnProbTimes += log(pow(lambda_event[i],A_minus_K)*E(i,timeline[i])+pow(1.0 - lambda_event[i],A_minus_K));
        }
    }
// std::cout << "computed (v); lnProbability = " << lnProbTimes << std::endl;

    // add the non-burst bifurcation age terms (vi)
    for (size_t i = 0; i < serial_bifurcation_times.size(); ++i)
    {
        if ( RbMath::isFinite(lnProbTimes) == false )
        {
            return RbConstants::Double::nan;
        }

        double t = serial_bifurcation_times[i];
        size_t index = findIndex(t);
        lnProbTimes += log( lambda[index] );
    }
// std::cout << "computed (vi); lnProbability = " << lnProbTimes << std::endl;

    // Compute probabilities of branch segments on all branches
    for (size_t i=0; i<num_nodes; ++i)
    {
        const TopologyNode& n = value->getNode( i );

        if ( !n.isRoot() && !n.isSampledAncestor() )
        {
//        double t_start = n.getParent().getAge();
//        double t_end = n.getAge();
            double t_start = n.getAge();
            double t_end = n.getParent().getAge();

            size_t interval_t_start = findIndex(t_start);
            size_t interval_t_end = findIndex(t_end);

            double t_o = t_start;
            size_t interval_t_o = interval_t_start;

            while ( interval_t_o < interval_t_end )
            {
                double t_y = timeline[interval_t_o+1];
                size_t interval_t_y = interval_t_o + 1;

                lnProbTimes -= lnD(interval_t_o,t_o);
                lnProbTimes += lnD(interval_t_y,t_y);

                t_o = t_y;
                interval_t_o = interval_t_y;
            }

            lnProbTimes -= lnD(interval_t_o,t_o);
            lnProbTimes += lnD(interval_t_end,t_end);

        }
    }

    // // Handle all branch segments by creating master list of all intervals and tracking number alive in each
    // // Assemble sorted master timeline defining all branch-interval beginnings and endings
    // std::vector<double> all_events = timeline;
    // all_events.reserve(1 + timeline.size() + serial_bifurcation_times.size() + serial_sampled_ancestor_ages.size() + serial_tip_ages.size());
    // all_events.insert(all_events.end(),(&value->getRoot())->getAge() - offset);
    // all_events.insert(all_events.end(),serial_bifurcation_times.begin(),serial_bifurcation_times.end());
    // all_events.insert(all_events.end(),serial_sampled_ancestor_ages.begin(),serial_sampled_ancestor_ages.end());
    // all_events.insert(all_events.end(),serial_tip_ages.begin(),serial_tip_ages.end());
    // std::sort(all_events.begin(),all_events.end());
    //
    // // Ensure list of times is unique
    // std::vector<double>::iterator it = std::unique(all_events.begin(),all_events.end());
    // all_events.resize(std::distance(all_events.begin(),it));
    //
    // // Compute probabilities of branch segments
    // for (size_t i=0; i<all_events.size()-1; ++i)
    // {
    //   if ( RbMath::isAComputableNumber(lnProbTimes) == false )
    //   {
    //       return RbConstants::Double::nan;
    //   }
    //   double t_y = all_events[i]; // The times are sorted in ascending order
    //   double t_o = all_events[i+1];
    //   int active_lineages_in_interval = survivors((t_o + t_y)/2.0);
    //   double lnD_t_o = lnD(findIndex(t_o),t_o);
    //   double lnD_t_y = lnD(findIndex(t_y),t_y);
    //   lnProbTimes += active_lineages_in_interval * lnD_t_o - active_lineages_in_interval * lnD_t_y;
    //
    // }
// std::cout << "computed (vii); lnProbability = " << lnProbTimes << std::endl;

    // condition on survival
    if ( condition == "survival" )
    {
        double root_age = (&value->getRoot())->getAge() - offset;
        lnProbTimes -= num_initial_lineages * log( 1 - E(findIndex(root_age),root_age) );
    }
    // condition on nTaxa
    else if ( condition == "nTaxa" )
    {
        // lnProbTimes -= lnProbNumTaxa( value->getNumberOfTips(), 0, process_time, true );
    }

    if ( RbMath::isFinite(lnProbTimes) == false )
    {
        return RbConstants::Double::nan;
    }


    return lnProbTimes;
}

/*
 * Counts all nodes in a number of sets
 * 1) bifurcation (birth) events NOT at burst times
 * 2) bifurcation (birth) events at burst times
 * 3) sampled ancestor nodes NOT at sampling events
 * 4) sampled nodes from extinct lineages NOT at sampling events
 * 5) sampled ancestor nodes at sampling events
 * 6) sampled nodes from extinct lineages at sampling events
 *
 * Non-burst trackers (1,3,4) are vectors of times of the samples.
 * All burst trackers (2,5,6) are vectors of vectors of samples, each vector corresponding to an event
 */
void EpisodicBirthDeathSamplingTreatmentProcess::countAllNodes(void) const
{
  // get node/time variables
  size_t num_nodes = value->getNumberOfNodes();

  size_t num_extant_taxa = 0;

  // clear current
  serial_tip_ages.clear();
  serial_sampled_ancestor_ages.clear();
  event_tip_ages.clear();
  event_sampled_ancestor_ages.clear();
  serial_bifurcation_times.clear();
  event_bifurcation_times.clear();

  // initialize vectors of vectors
  event_sampled_ancestor_ages = std::vector<std::vector<double> >(timeline.size(),std::vector<double>(0,1.0));
  event_tip_ages = std::vector<std::vector<double> >(timeline.size(),std::vector<double>(0,1.0));
  event_bifurcation_times = std::vector<std::vector<double> >(timeline.size(),std::vector<double>(0,1.0));

  // Assign all node times (bifurcations, sampled ancestors, and tips) to their sets, accounting for offset
  for (size_t i = 0; i < num_nodes; i++)
  {
      const TopologyNode& n = value->getNode( i );

      double t = n.getAge() - offset;

      if ( n.isTip() && n.isFossil() && n.isSampledAncestor() )
      {
        // node is sampled ancestor
          int at_event = whichIntervalTime(t);

          if (at_event == -1)
          {
            serial_sampled_ancestor_ages.push_back(t);
          }
          else
          {
            event_sampled_ancestor_ages[i].push_back(t);
          }
      }
      else if ( n.isTip() && n.isFossil() && !n.isSampledAncestor() )
      {
          // node is serial leaf
          int at_event = whichIntervalTime(t);

          // If this tip is not at an event time (and at an event time with Phi[i] > 0), it's a serial tip
          if (at_event == -1 || phi_event[at_event] < DBL_EPSILON)
          {
            serial_tip_ages.push_back(t);
          }
          else
          {
            event_tip_ages[i].push_back(t);
          }
      }
      else if ( n.isTip() && !n.isFossil() )
      {
        // Node is at present, this can happen even if Phi[0] = 0, so we check if there is really a sampling event at the present
        if (phi_event[0] >= DBL_EPSILON)
        {
          // node is extant leaf
          num_extant_taxa++;
          event_tip_ages[0].push_back(0.0);
        }
        else
        {
          serial_tip_ages.push_back(0.0);
        }
      }
      else if ( n.isInternal() && !n.getChild(0).isSampledAncestor() && !n.getChild(1).isSampledAncestor() )
      {
          if ( n.isRoot() == false || use_origin == true )
          {
              // node is bifurcation event (a "true" node)
              int at_event = whichIntervalTime(t);

              if (at_event == -1)
              {
                serial_bifurcation_times.push_back(t);
              }
              else
              {
                event_bifurcation_times[i].push_back(t);
              }
          }
      }
  }
}

// /**
//  * Compute D_i(t)
//  */
// double EpisodicBirthDeathSamplingTreatmentProcess::D(size_t i, double t) const
// {
//   // D(0) = 1
//   if ( t < DBL_EPSILON )
//   {
//     return 1.0;
//   }
//   else
//   {
//     double D_i = 4 * exp(-A_i[i] * (t - timeline[i]));
//     D_i /= pow((1 + B_i[i] + (exp(-A_i[i] * (t - timeline[i]))) * (1 - B_i[i])), 2.0);
//     return D_i;
//   }
// }

/**
 * Compute ln(D_i(t))
 */
double EpisodicBirthDeathSamplingTreatmentProcess::lnD(size_t i, double t) const
{
    // D(0) = 1
    if ( t < DBL_EPSILON )
    {
        // TODO: this can't be right, if phi_event[0] = 0 this will blow up
        return log(phi_event[0]);
    }
    else
    {
        double s = timeline[i];
        double this_lnD_i = 0.0;
        if (i > 0)
        {
            // D <- D * (1-this_p_s) * (1-this_p_d) * (1-this_p_b + 2*this_p_b*E)
            this_lnD_i = lnD_previous[i];
            this_lnD_i += log(1.0-phi_event[i]) + log(1.0-mu_event[i]) + log(1-lambda_event[i]+2*lambda_event[i]*E_previous[i]);
        }
        else
        {
            this_lnD_i = log(phi_event[0]);
        }
        // D <- D * 4 * exp(-A*(next_t-current_t))
        // D <- D / ( 1+B+exp(-A*(next_t-current_t))*(1-B) )^2
        this_lnD_i += 2*RbConstants::LN2 + (-A_i[i] * (t - s));
        this_lnD_i -= 2 * log(1 + B_i[i] + exp(-A_i[i] * (t - s)) * (1 - B_i[i]));

        return this_lnD_i;
    }
}

/**
 * Compute E_i(t)
 */
double EpisodicBirthDeathSamplingTreatmentProcess::E(size_t i, double t) const
{
  // E_{-1}(0) = 1
  // if ( t < DBL_EPSILON )
  // {
  //   return 1.0;
  // }
  // else
  // {

    // E <- (b + d + s - A *(1+B-exp(-A*(next_t-current_t))*(1-B))/(1+B+exp(-A*(next_t-current_t))*(1-B)) ) / (2*b)
    double s = timeline[i];

    double E_i = lambda[i] + mu[i] + phi[i];
    E_i -= A_i[i] * (1 + B_i[i] - exp(-A_i[i] * (t - s)) * (1 - B_i[i])) / (1 + B_i[i] + exp(-A_i[i] * (t - s)) * (1 - B_i[i]));
    E_i /= (2 * lambda[i]);

    return E_i;
  // }
}

/**
 * return the index i so that t_{i-1} <= t < t_i
 * where t_i is the instantaneous sampling time (i = 0,...,l)
 * t_0 = 0.0
 * t_l = Inf
 */
size_t EpisodicBirthDeathSamplingTreatmentProcess::findIndex(double t) const
{
    // Linear search for interval because upper_bound isn't cooperating
    for (size_t i=0; i < timeline.size()-1; ++i)
    {
        if (t >= timeline[i] && t < timeline[i+1])
        {
            return i;
        }
    }
    
    return timeline.size() - 1;

    // // Binary search for interval because std::upper_bound isn't cooperating
    // // The run-time cost differential of binary versus linear seems negligible in limited testing
    // // First check if t > s_l, since we don't have an l+1 this interval is unbounded and we can't include it in the search
    // if (t >= timeline[timeline.size()-1])
    // {
    //   return timeline.size() - 1;
    // }
    //
    // int left = 0;
    // int right = timeline.size() - 1;
    //
    // while (left <= right)
    // {
    //   int m = floor((left + right) / 2.0);
    //   if (timeline[m] <= t)
    //   {
    //     if (t < timeline[m+1])
    //     {
    //       return m;
    //     }
    //     else
    //     {
    //       left = m + 1;
    //     }
    //   }
    //   else
    //   {
    //     right = m - 1;
    //   }
    // }
    // std::cout << "Cannot locate interval for time " << t << std::endl;
    // throw(RbException("Binary search failure."));
}

// calculate offset so we can set t_0 to time of most recent tip
void EpisodicBirthDeathSamplingTreatmentProcess::getOffset(void) const
{
    offset = RbConstants::Double::max;
    for (size_t i = 0; i < value->getNumberOfNodes(); i++)
    {
        const TopologyNode& n = value->getNode( i );

        if ( n.getAge() < offset )
        {
            offset = n.getAge();
        }
    }

}

// double EpisodicBirthDeathSamplingTreatmentProcess::lnProbNumTaxa(size_t n, double start, double end, bool MRCA) const
// {
//   throw(RbException("Cannot compute lnProbNumTaxa."));
//   return RbConstants::Double::neginf;
// }

double EpisodicBirthDeathSamplingTreatmentProcess::lnProbTreeShape(void) const
{
    // the birth death divergence times density is derived for a (ranked) unlabeled oriented tree
    // so we convert to a (ranked) labeled non-oriented tree probability by multiplying by 2^{n+m-1} / n!
    // where n is the number of extant tips, m is the number of extinct tips

    int num_taxa = (int)value->getNumberOfTips();
    int num_extinct = (int)value->getNumberOfExtinctTips();
    int num_sa = (int)value->getNumberOfSampledAncestors();

    //TODO: Check against Gavryushkina (2014)
    return (num_taxa - num_sa - 1) * RbConstants::LN2 - RbMath::lnFactorial(num_taxa - num_extinct);
}

/**
 * Here we ensure that we have a length-l vector of rates and events for all parameters and the timeline.
 * In the case of homogeneous/constant-rate parameters, we fill the vector with this rate.
 * In the case of homogenous/single events, we make all but the first event rates 0.
 *
 */
void EpisodicBirthDeathSamplingTreatmentProcess::updateVectorParameters( void ) const
{
    // clean and get timeline
    timeline.clear();
    timeline = interval_times->getValue();

    // Offset the timeline
    for (size_t i=0; i<timeline.size(); ++i)
    {
      timeline[i] += offset;
    }

    // Add t_0
    timeline.insert(timeline.begin(),0.0);

    // clean all the sets
    lambda.clear();
    mu.clear();
    phi.clear();
    r.clear();
    lambda_event.clear();
    mu_event.clear();
    phi_event.clear();

    // Get vector of birth rates
    if ( heterogeneous_lambda != NULL )
    {
      lambda = heterogeneous_lambda->getValue();
    }
    else
    {
      lambda = std::vector<double>(timeline.size(),homogeneous_lambda->getValue());
    }

    // Get vector of death rates
    if ( heterogeneous_mu != NULL )
    {
      mu = heterogeneous_mu->getValue();
    }
    else
    {
      mu = std::vector<double>(timeline.size(),homogeneous_mu->getValue());
    }

    // Get vector of serial sampling rates
    if ( heterogeneous_phi != NULL )
    {
      phi = heterogeneous_phi->getValue();
    }
    else
    {
      phi = std::vector<double>(timeline.size(),homogeneous_phi->getValue());
    }

    // Get vector of conditional death upon sampling probabilities
    if ( heterogeneous_r != NULL )
    {
      r = heterogeneous_r->getValue();
    }
    else
    {
      r = std::vector<double>(timeline.size(),homogeneous_r->getValue());
    }

    // Get vector of burst birth probabilities
    if ( heterogeneous_Lambda != NULL )
    {
      // User has specified lambda_event_1,...,lambda_event_{l-1}
      lambda_event = heterogeneous_Lambda->getValue();
      // lambda_event_0 must be 0 (there can be no burst at the present)
      lambda_event.insert(lambda_event.begin(),0.0);
    }
    else
    {
      // User specified nothing, there are no birth bursts
      lambda_event = std::vector<double>(timeline.size(),0.0);
    }

    // Get vector of burst death (mass extinction) probabilities
    if ( heterogeneous_Mu != NULL )
    {
      // User has specified mu_event_1,...,mu_event_{l-1}
      mu_event = heterogeneous_Mu->getValue();
      // mu_event_0 must be 0 (there can be no burst at the present)
      mu_event.insert(mu_event.begin(),0.0);
    }
    else
    {
      // User specified nothing, there are no birth bursts
      mu_event = std::vector<double>(timeline.size(),0.0);
    }

    // Get vector of event sampling probabilities
    if ( heterogeneous_Phi != NULL )
    {
      // User has specified phi_event_0,...,phi_event_{l-1}
      phi_event = heterogeneous_Phi->getValue();
    }
    else
    {
        phi_event = std::vector<double>(timeline.size(),0.0);
        if ( homogeneous_Phi != NULL )
        {
            // User specified the sampling fraction at the present
            phi_event[0] = homogeneous_Phi->getValue();
        }
        else
        {
            // set the final sampling to one (for sampling at the present)
            phi_event[0] = 1.0;
      }

    }

}

/*
 * Here we calculate all A_i, B_i, C_i, D_i(s_i), and E_i(s_i) for i = 1,...,l
 *
 */
void EpisodicBirthDeathSamplingTreatmentProcess::prepareProbComputation( void ) const
{
    // // clean all the sets
    // A_i.clear();
    // B_i.clear();
    // C_i.clear();
    // E_previous.clear();

    // re-initialize all the sets
    A_i = std::vector<double>(timeline.size(),0.0);
    B_i = std::vector<double>(timeline.size(),0.0);
    C_i = std::vector<double>(timeline.size(),0.0);

    // E_previous[i] is E_{i-1}(s_i)
    // lnD_previous[i] is log(D_{i-1}(s_i))
    E_previous      = std::vector<double>(timeline.size(),0.0);
    lnD_previous    = std::vector<double>(timeline.size(),0.0);

    // timeline[0] == 0.0
    double t = timeline[0];

    // Compute all starting at 1
    A_i[0] = sqrt( pow(lambda[0] - mu[0] - phi[0],2.0) + 4 * lambda[0] * phi[0]);

    // At the present, only sampling is allowed, no birth/death bursts
    C_i[0] = (1 - phi_event[0]);

    B_i[0] = (1.0 - 2.0 * C_i[0]) * lambda[0] + mu[0] + phi[0];
    B_i[0] /= A_i[0];

    // E_{i-1}(0) = 1, and our E(i,t) function requires i >= 0, so we hard-code this explicitly
    // Sebastian: This should be the probability of going extinct, which is in this case the probability of non-sampling.
    // Andy: This is not E_0(t_0) this is E_{-1}(t_0)
    // E_previous[0] = (1 - phi_event[0]);
    E_previous[0] = 1.0;

    // we always initialize the probability of observing the lineage at the present with the sampling probability
    // TODO: This can't be right, if there is no event sampling this will blow up
    lnD_previous[0] = 0.0;

    for (size_t i=1; i<timeline.size(); ++i)
    {
        t = timeline[i];
        
        // first, we need to compute E and D at the end of the previous interval
        E_previous[i] = E(i-1, t);
        lnD_previous[i] = lnD(i-1, t);

        // now we can compute A_i, B_i and C_i at the end of this interval.
        A_i[i] = sqrt( pow(lambda[i] - mu[i] - phi[i],2.0) + 4 * lambda[i] * phi[i]);

        C_i[i] = (1.0 - lambda_event[i]) * (1 - mu_event[i]) * E_previous[i];
        C_i[i] += (1.0 - mu_event[i]) * lambda_event[i] * E_previous[i] * E_previous[i];
        C_i[i] += (1.0 - lambda_event[i]) * mu_event[i];
        C_i[i] *= (1.0 - phi_event[i]);

        B_i[i] = (1.0 - 2.0 * C_i[i]) * lambda[i] + mu[i] + phi[i];
        B_i[i] /= A_i[i];

    }
}

double EpisodicBirthDeathSamplingTreatmentProcess::pSurvival(double start, double end) const
{
  //TODO: we do not really need this function here, and survival does not have quite the same meaning here
  //      we should make sure this is this is the closest translation of survival to SSBDPs
  return (1.0 - E(findIndex(end),end)) / (1.0 - E(findIndex(end),end));
}

/**
 * Simulate new speciation times.
 */
double EpisodicBirthDeathSamplingTreatmentProcess::simulateDivergenceTime(double origin, double present) const
{
    // incorrect placeholder for constant SSBDP


    // Get the rng
    RandomNumberGenerator* rng = GLOBAL_RNG;

    size_t i = findIndex(present);

    // get the parameters
    double age = origin - present;
    double b = lambda[i];
    double d = mu[i];
    double p_e = phi_event[i];


    // get a random draw
    double u = rng->uniform01();

    // compute the time for this draw
    // see Hartmann et al. 2010 and Stadler 2011
    double t = 0.0;
    if ( b > d )
    {
        if( p_e > 0.0 )
        {
            t = ( log( ( (b-d) / (1 - (u)*(1-((b-d)*exp((d-b)*age))/(p_e*b+(b*(1-p_e)-d)*exp((d-b)*age) ) ) ) - (b*(1-p_e)-d) ) / (p_e * b) ) )  /  (b-d);
        }
        else
        {
            t = log( 1 - u * (exp(age*(d-b)) - 1) / exp(age*(d-b)) ) / (b-d);
        }
    }
    else
    {
        if( p_e > 0.0 )
        {
            t = ( log( ( (b-d) / (1 - (u)*(1-(b-d)/(p_e*b*exp((b-d)*age)+(b*(1-p_e)-d) ) ) ) - (b*(1-p_e)-d) ) / (p_e * b) ) )  /  (b-d);
        }
        else
        {
            t = log( 1 - u * (1 - exp(age*(b-d)))  ) / (b-d);
        }
    }

    return present + t;
}


/**
 * Compute the diversity of the tree at time t.
 *
 * \param[in]    t      time at which we want to know the diversity.
 *
 * \return The diversity (number of species in the reconstructed tree).
 */
int EpisodicBirthDeathSamplingTreatmentProcess::survivors(double t) const
{

    const std::vector<TopologyNode*>& nodes = value->getNodes();

    int survivors = 0;
    for (std::vector<TopologyNode*>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        TopologyNode* n = *it;
        double a = n->getAge();
        if ( (a - t) <= DBL_EPSILON )
        {
            if ( n->isRoot() == true || (n->getParent().getAge() - t) >= -DBL_EPSILON )
            {
                survivors++;
            }
        }
    }

    return survivors;
}


/**
 * For a time t, determine which if any interval (event) time it corresponds to
 *
 * \param[in]    t      time we want to assess
 *
 * \return -1 if time matches no interval, otherwise the interval
 */
int EpisodicBirthDeathSamplingTreatmentProcess::whichIntervalTime(double t) const
{
  // Find the i such that s_i <= t < s_{i+1}
  size_t i = findIndex(t);

  // Check if s_i == t
  if ( t > timeline[i] + DBL_EPSILON && t < timeline[i] - DBL_EPSILON )
  {
    return (int) i;
  }
  else
  {
    return -1;
  }

}

/**
 * Swap the parameters held by this distribution.
 *
 * \param[in]    oldP      Pointer to the old parameter.
 * \param[in]    newP      Pointer to the new parameter.
 */
void EpisodicBirthDeathSamplingTreatmentProcess::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    // Rate parameters
    if (oldP == heterogeneous_lambda)
    {
        heterogeneous_lambda = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == heterogeneous_mu)
    {
        heterogeneous_mu = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == heterogeneous_phi)
    {
        heterogeneous_phi = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == homogeneous_lambda)
    {
        homogeneous_lambda = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == homogeneous_mu)
    {
        homogeneous_mu = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == homogeneous_phi)
    {
        homogeneous_phi = static_cast<const TypedDagNode<double>* >( newP );
    }
    // Treatment
    else if (oldP == heterogeneous_r)
    {
        heterogeneous_r = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == homogeneous_r)
    {
        homogeneous_r = static_cast<const TypedDagNode<double>* >( newP );
    }
    // Event probability parameters
    if (oldP == heterogeneous_Lambda)
    {
        heterogeneous_Lambda = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == heterogeneous_Mu)
    {
        heterogeneous_Mu = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == heterogeneous_Phi)
    {
        heterogeneous_Phi = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    // else if (oldP == homogeneous_Lambda)
    // {
    //     homogeneous_Lambda = static_cast<const TypedDagNode<double>* >( newP );
    // }
    // else if (oldP == homogeneous_Mu)
    // {
    //     homogeneous_Mu = static_cast<const TypedDagNode<double>* >( newP );
    // }
    else if (oldP == homogeneous_Phi)
    {
        homogeneous_Phi = static_cast<const TypedDagNode<double>* >( newP );
    }
    else
    {
        // delegate the super-class
        AbstractBirthDeathProcess::swapParameterInternal(oldP, newP);
    }
}
