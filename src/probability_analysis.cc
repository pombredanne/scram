/// @file probability_analysis.cc
/// Implementations of functions to provide probability and importance
/// informations.
#include "probability_analysis.h"

#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/pointer_cast.hpp>

#include "error.h"

namespace scram {

ProbabilityAnalysis::ProbabilityAnalysis(std::string approx, int nsums,
                                         double cut_off)
    : warnings_(""),
      p_total_(0),
      num_prob_mcs_(-1),
      coherent_(true),
      p_time_(-1),
      imp_time_(-1) {
  // Check for right number of sums.
  if (nsums < 1) {
    std::string msg = "The number of sums in the probability calculation "
                      "cannot be less than one";
    throw InvalidArgument(msg);
  }
  nsums_ = nsums;

  // Check for valid cut-off probability.
  if (cut_off < 0 || cut_off > 1) {
    std::string msg = "The cut-off probability cannot be negative or"
                      " more than 1.";
    throw InvalidArgument(msg);
  }
  cut_off_ = cut_off;

  // Check the right approximation for probability calculations.
  if (approx != "no" && approx != "rare" && approx != "mcub") {
    std::string msg = "The probability approximation is not recognized.";
    throw InvalidArgument(msg);
  }
  approx_ = approx;
}

void ProbabilityAnalysis::UpdateDatabase(
    const boost::unordered_map<std::string, BasicEventPtr>& basic_events) {
  basic_events_ = basic_events;
  ProbabilityAnalysis::AssignIndices();
}

void ProbabilityAnalysis::Analyze(
    const std::set< std::set<std::string> >& min_cut_sets) {
  min_cut_sets_ = min_cut_sets;

  ProbabilityAnalysis::IndexMcs(min_cut_sets_);

  using boost::container::flat_set;
  // Minimal cut sets with higher than cut-off probability.
  std::set< flat_set<int> > mcs_for_prob;
  // Iterate minimal cut sets and find probabilities for each set.
  std::vector< flat_set<int> >::const_iterator it_min;
  int i = 0;  // Indices for minimal cut sets in the vector.
  for (it_min = imcs_.begin(); it_min != imcs_.end();
       ++i, ++it_min) {
    // Calculate a probability of a set with AND relationship.
    double p_sub_set = ProbabilityAnalysis::ProbAnd(*it_min);
    if (p_sub_set > cut_off_) {
      flat_set<int> mcs(*it_min);
      mcs_for_prob.insert(mcs_for_prob.end(), mcs);
    }

    // Update a container with minimal cut sets and probabilities.
    prob_of_min_sets_.insert(std::make_pair(imcs_to_smcs_[i], p_sub_set));
    ordered_min_sets_.insert(std::make_pair(p_sub_set, imcs_to_smcs_[i]));
  }
  // Minimal cut sets with higher than cut-off probability for importance
  // analysis.
  std::set< flat_set<int> > mcs_for_imp(mcs_for_prob);

  // Timing Initialization
  std::clock_t start_time;
  start_time = std::clock();

  // Get the total probability.
  // Check if the rare event approximation is requested.
  if (approx_ == "rare") {
    warnings_ += "Using the rare event approximation\n";
    bool rare_event_legit = true;
    std::map< std::set<std::string>, double >::iterator it_pr;
    for (it_pr = prob_of_min_sets_.begin();
         it_pr != prob_of_min_sets_.end(); ++it_pr) {
      // Check if a probability of a set does not exceed 0.1,
      // which is required for the rare event approximation to hold.
      if (rare_event_legit && (it_pr->second > 0.1)) {
        rare_event_legit = false;
        warnings_ += "The rare event approximation may be inaccurate for this"
            "\nfault tree analysis because one of minimal cut sets'"
            "\nprobability exceeded 0.1 threshold requirement.\n\n";
      }
      p_total_ += it_pr->second;
    }

  } else if (approx_ == "mcub") {
    warnings_ += "Using the MCUB approximation\n";
    if (!coherent_) {
      warnings_ += " The cut sets are not coherent and contain negation."
          "\nThe MCUB approximation may not hold.\n\n";
    }
    double m = 1;
    std::map< std::set<std::string>, double >::iterator it;
    for (it = prob_of_min_sets_.begin(); it != prob_of_min_sets_.end();
         ++it) {
      m *= 1 - it->second;
    }
    p_total_ = 1 - m;

  } else {  // The default calculations.
    // Choose cut sets with high enough probabilities.
    num_prob_mcs_ = mcs_for_prob.size();
    if (nsums_ > mcs_for_prob.size()) nsums_ = mcs_for_prob.size();
    if (coherent_) {
      p_total_ = ProbabilityAnalysis::CoherentProbOr(nsums_, &mcs_for_prob);
    } else {
      p_total_ = ProbabilityAnalysis::ProbOr(nsums_, &mcs_for_prob);
    }
  }
  // Duration of the calculations.
  p_time_ = (std::clock() - start_time) / static_cast<double>(CLOCKS_PER_SEC);

  ProbabilityAnalysis::PerformImportanceAnalysis(mcs_for_imp);

  // Duration of the calculations.
  imp_time_ = (std::clock() - start_time) / static_cast<double>(CLOCKS_PER_SEC);
  imp_time_ -= p_time_;
}

void ProbabilityAnalysis::AssignIndices() {
  // Cleanup the previous information.
  int_to_basic_.clear();
  basic_to_int_.clear();
  iprobs_.clear();
  // Indexation of events.
  int j = 1;
  boost::unordered_map<std::string, BasicEventPtr>::iterator itp;
  // Dummy basic event at index 0.
  int_to_basic_.push_back(BasicEventPtr(new BasicEvent("dummy")));
  iprobs_.push_back(0);
  for (itp = basic_events_.begin(); itp != basic_events_.end(); ++itp) {
    int_to_basic_.push_back(itp->second);
    basic_to_int_.insert(std::make_pair(itp->second->id(), j));
    iprobs_.push_back(itp->second->p());
    ++j;
  }
}

void ProbabilityAnalysis::IndexMcs(
    const std::set<std::set<std::string> >& min_cut_sets) {
  // Update databases of minimal cut sets with indexed events.
  std::set< std::set<std::string> >::const_iterator it;
  for (it = min_cut_sets.begin(); it != min_cut_sets.end(); ++it) {
    using boost::container::flat_set;
    flat_set<int> mcs_with_indices;  // Minimal cut set with indices.
    std::set<std::string>::const_iterator it_set;
    for (it_set = it->begin(); it_set != it->end(); ++it_set) {
      std::vector<std::string> names;
      boost::split(names, *it_set, boost::is_any_of(" "),
                   boost::token_compress_on);
      assert(names.size() == 1 || names.size() == 2);
      if (names.size() == 1) {
        assert(basic_to_int_.count(names[0]));
        mcs_with_indices.insert(mcs_with_indices.end(),
                                basic_to_int_.find(names[0])->second);
        mcs_basic_events_.insert(basic_to_int_.find(names[0])->second);
      } else {
        // This must be a complement of an event.
        assert(names[0] == "not");
        assert(basic_to_int_.count(names[1]));

        if (coherent_) coherent_ = false;  // Detected non-coherency.

        mcs_with_indices.insert(mcs_with_indices.begin(),
                                -basic_to_int_.find(names[1])->second);
        mcs_basic_events_.insert(basic_to_int_.find(names[1])->second);
      }
    }
    imcs_.push_back(mcs_with_indices);
    imcs_to_smcs_.push_back(*it);
  }
}

double ProbabilityAnalysis::ProbOr(
    int nsums,
    std::set< boost::container::flat_set<int> >* min_cut_sets) {
  assert(nsums >= 0);

  // Recursive implementation.
  if (min_cut_sets->empty()) return 0;

  if (nsums == 0) return 0;

  // Base case.
  if (min_cut_sets->size() == 1) {
    // Get only element in this set.
    return ProbabilityAnalysis::ProbAnd(*min_cut_sets->begin());
  }

  using boost::container::flat_set;

  // Get one element.
  std::set< flat_set<int> >::iterator it = min_cut_sets->begin();
  flat_set<int> element_one(*it);

  // Delete element from the original set.
  min_cut_sets->erase(it);
  std::set< flat_set<int> > combo_sets;
  ProbabilityAnalysis::CombineElAndSet(element_one, *min_cut_sets, &combo_sets);

  return ProbabilityAnalysis::ProbAnd(element_one) +
      ProbabilityAnalysis::ProbOr(nsums, min_cut_sets) -
      ProbabilityAnalysis::ProbOr(nsums - 1, &combo_sets);
}

double ProbabilityAnalysis::ProbAnd(
    const boost::container::flat_set<int>& min_cut_set) {
  // Test just in case the min cut set is empty.
  if (min_cut_set.empty()) return 0;

  using boost::container::flat_set;
  double p_sub_set = 1;  // 1 is for multiplication.
  flat_set<int>::const_iterator it_set;
  for (it_set = min_cut_set.begin(); it_set != min_cut_set.end(); ++it_set) {
    if (*it_set > 0) {
      p_sub_set *= iprobs_[*it_set];
    } else {
      p_sub_set *= 1 - iprobs_[std::abs(*it_set)];  // Never zero.
    }
  }
  return p_sub_set;
}

void ProbabilityAnalysis::CombineElAndSet(
    const boost::container::flat_set<int>& el,
    const std::set< boost::container::flat_set<int> >& set,
    std::set< boost::container::flat_set<int> >* combo_set) {
  using boost::container::flat_set;
  std::set< flat_set<int> >::iterator it_set;
  for (it_set = set.begin(); it_set != set.end(); ++it_set) {
    bool include = true;  // Indicates that the resultant set is not null.
    flat_set<int>::const_iterator it;
    for (it = el.begin(); it != el.end(); ++it) {
      if (it_set->count(-*it)) {
        include = false;
        break;  // A complement is found; the set is null.
      }
    }
    if (include) {
      flat_set<int> member_set(*it_set);
      member_set.insert(el.begin(), el.end());
      combo_set->insert(combo_set->end(), member_set);
    }
  }
}

double ProbabilityAnalysis::CoherentProbOr(
    int nsums,
    std::set< boost::container::flat_set<int> >* min_cut_sets) {
  assert(nsums >= 0);

  // Recursive implementation.
  if (min_cut_sets->empty()) return 0;

  if (nsums == 0) return 0;

  // Base case.
  if (min_cut_sets->size() == 1) {
    // Get only element in this set.
    return ProbabilityAnalysis::CoherentProbAnd(*min_cut_sets->begin());
  }

  using boost::container::flat_set;

  // Get one element.
  std::set< flat_set<int> >::iterator it = min_cut_sets->begin();
  flat_set<int> element_one(*it);

  // Delete element from the original set.
  min_cut_sets->erase(it);
  std::set< flat_set<int> > combo_sets;
  ProbabilityAnalysis::CoherentCombineElAndSet(element_one,
                                               *min_cut_sets, &combo_sets);

  return ProbabilityAnalysis::CoherentProbAnd(element_one) +
      ProbabilityAnalysis::CoherentProbOr(nsums, min_cut_sets) -
      ProbabilityAnalysis::CoherentProbOr(nsums - 1, &combo_sets);
}

double ProbabilityAnalysis::CoherentProbAnd(
    const boost::container::flat_set<int>& min_cut_set) {
  // Test just in case the min cut set is empty.
  if (min_cut_set.empty()) return 0;

  using boost::container::flat_set;
  double p_sub_set = 1;  // 1 is for multiplication.
  flat_set<int>::const_iterator it_set;
  for (it_set = min_cut_set.begin(); it_set != min_cut_set.end(); ++it_set) {
      p_sub_set *= iprobs_[*it_set];
  }
  return p_sub_set;
}

void ProbabilityAnalysis::CoherentCombineElAndSet(
    const boost::container::flat_set<int>& el,
    const std::set< boost::container::flat_set<int> >& set,
    std::set< boost::container::flat_set<int> >* combo_set) {
  using boost::container::flat_set;
  std::set< flat_set<int> >::iterator it_set;
  for (it_set = set.begin(); it_set != set.end(); ++it_set) {
    flat_set<int> member_set(*it_set);
    member_set.insert(el.begin(), el.end());
    combo_set->insert(combo_set->end(), member_set);
  }
}

void ProbabilityAnalysis::PerformImportanceAnalysis(
    const std::set< boost::container::flat_set<int> >& min_cut_sets) {
  // The main data for all the importance types is P(top/event) or
  // P(top/Not event).
  std::set<int>::iterator it;
  for (it = mcs_basic_events_.begin(); it != mcs_basic_events_.end(); ++it) {
    // Calculate P(top/event)
    iprobs_[*it] = 1;
    std::set< boost::container::flat_set<int> > cut_sets(min_cut_sets);
    double p_e = ProbabilityAnalysis::ProbOr(nsums_, &cut_sets);

    // Calculate P(top/Not event)
    iprobs_[*it] = 0;
    cut_sets = min_cut_sets;
    double p_not_e = ProbabilityAnalysis::ProbOr(nsums_, &cut_sets);
    // Restore the probability.
    iprobs_[*it] = int_to_basic_[*it]->p();

    // Mapped vector for importances.
    std::vector<double> imp;
    // Fussel-Vesely Diagnosis importance factor.
    imp.push_back(1 - p_not_e / p_total_);
    // Birnbaum Marginal importance factor.
    imp.push_back(p_e - p_not_e);
    // Critical Importance factor.
    imp.push_back(imp[1] * iprobs_[*it] / p_not_e);
    // Risk Reduction Worth.
    imp.push_back(p_total_ / p_not_e);
    // Risk Achievement Worth.
    imp.push_back(p_e / p_total_);

    importance_.insert(std::make_pair(int_to_basic_[*it]->id(), imp));
    ordered_primaries_.insert(std::make_pair(imp[0],
                                             int_to_basic_[*it]->id()));
  }
}

void ProbabilityAnalysis::CombineEventAndSets(
    int event,
    const std::set< boost::container::flat_set<int> >& min_cut_sets,
    std::set< boost::container::flat_set<int> >* final_cut_sets) {

}

}  // namespace scram
