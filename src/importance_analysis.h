/*
 * Copyright (C) 2014-2016 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file importance_analysis.h
/// Contains functionality to do numerical analysis
/// of importance factors.

#ifndef SCRAM_SRC_IMPORTANCE_ANALYSIS_H_
#define SCRAM_SRC_IMPORTANCE_ANALYSIS_H_

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bdd.h"
#include "probability_analysis.h"
#include "settings.h"

namespace scram {

namespace mef {  // Decouple from the analysis code header.
class BasicEvent;
}  // namespace mef

namespace core {

/// Collection of importance factors for variables.
struct ImportanceFactors {
  double mif;  ///< Birnbaum marginal importance factor.
  double cif;  ///< Critical importance factor.
  double dif;  ///< Fussel-Vesely diagnosis importance factor.
  double raw;  ///< Risk achievement worth factor.
  double rrw;  ///< Risk reduction worth factor.
};

/// Analysis of importance factors of risk model variables.
class ImportanceAnalysis : public Analysis {
 public:
  /// Mapping of an event and its importance.
  using ImportanceRecord = std::pair<const mef::BasicEvent*, ImportanceFactors>;

  /// Importance analysis
  /// on the fault tree represented by
  /// its probability analysis.
  ///
  /// @param[in] prob_analysis  Completed probability analysis.
  explicit ImportanceAnalysis(const ProbabilityAnalysis* prob_analysis);

  virtual ~ImportanceAnalysis() = default;

  /// Performs quantitative analysis of importance factors
  /// of basic events in products.
  ///
  /// @pre Analysis is called only once.
  void Analyze() noexcept;

  /// @returns Map with basic events and their importance factors.
  ///
  /// @pre The importance analysis is done.
  const std::unordered_map<std::string, ImportanceFactors>& importance() const {
    return importance_;
  }

  /// @returns A collection of important events and their importance factors.
  ///
  /// @pre The importance analysis is done.
  const std::vector<ImportanceRecord>& important_events() const {
    return important_events_;
  }

 protected:
  /// Gathers all events present in products.
  /// Only this events can have importance factors.
  ///
  /// @param[in] graph  Boolean graph with basic event indices and pointers.
  /// @param[in] products  Products with basic event indices.
  ///
  /// @returns A unique collection of important basic events.
  std::vector<std::pair<int, const mef::BasicEvent*>> GatherImportantEvents(
      const BooleanGraph* graph,
      const std::vector<std::vector<int>>& products) noexcept;

 private:
  /// @returns Total probability from the probability analysis.
  virtual double p_total() noexcept = 0;

  /// Find all events that are in the products.
  ///
  /// @returns Indices and pointers to the basic events.
  virtual std::vector<std::pair<int, const mef::BasicEvent*>>
  GatherImportantEvents() noexcept = 0;

  /// Calculates Marginal Importance Factor.
  ///
  /// @param[in] index  Positive index of an event.
  ///
  /// @returns Calculated value for MIF.
  virtual double CalculateMif(int index) noexcept = 0;

  /// Container for basic event importance factors.
  std::unordered_map<std::string, ImportanceFactors> importance_;
  /// Container of pointers to important events and their importance factors.
  std::vector<ImportanceRecord> important_events_;
};

/// Base class for analyzers of importance factors
/// with the help from probability analyzers.
///
/// @tparam Calculator  Quantitative calculator of probability values.
template <class Calculator>
class ImportanceAnalyzerBase : public ImportanceAnalysis {
 public:
  /// Constructs importance analyzer from probability analyzer.
  /// Probability analyzer facilities are used
  /// to calculate the total and conditional probabilities for factors.
  ///
  /// @param[in] prob_analyzer  Instantiated probability analyzer.
  explicit ImportanceAnalyzerBase(
      ProbabilityAnalyzer<Calculator>* prob_analyzer)
      : ImportanceAnalysis(prob_analyzer),
        prob_analyzer_(prob_analyzer) {}

 protected:
  virtual ~ImportanceAnalyzerBase() = default;

  /// @returns A pointer to the helper probability analyzer.
  ProbabilityAnalyzer<Calculator>* prob_analyzer() { return prob_analyzer_; }

 private:
  /// @returns Total probability calculated by probability analyzer.
  double p_total() noexcept override { return prob_analyzer_->p_total(); }

  /// Find all events that are in the products.
  ///
  /// @returns Indices and pointers to the basic events.
  std::vector<std::pair<int, const mef::BasicEvent*>>
  GatherImportantEvents() noexcept override {
    return ImportanceAnalysis::GatherImportantEvents(
        prob_analyzer_->graph(),
        prob_analyzer_->products());
  }

  /// Calculator of the total probability.
  ProbabilityAnalyzer<Calculator>* prob_analyzer_;
};

/// Analyzer of importance factors
/// with the help from probability analyzers.
///
/// @tparam Calculator  Quantitative calculator of probability values.
template <class Calculator>
class ImportanceAnalyzer : public ImportanceAnalyzerBase<Calculator> {
 public:
  /// @copydoc ImportanceAnalyzerBase<Calculator>::ImportanceAnalyzerBase
  explicit ImportanceAnalyzer(ProbabilityAnalyzer<Calculator>* prob_analyzer)
      : ImportanceAnalyzerBase<Calculator>(prob_analyzer),
        p_vars_(prob_analyzer->p_vars()) {}

 private:
  double CalculateMif(int index) noexcept override;
  std::vector<double> p_vars_;  ///< A private copy of variable probabilities.
};

template <class Calculator>
double ImportanceAnalyzer<Calculator>::CalculateMif(int index) noexcept {
  using Base = ImportanceAnalyzerBase<Calculator>;

  double p_store = p_vars_[index];  // Save the original value for restoring.

  // Calculate P(top/event)
  p_vars_[index] = 1;
  double p_e = Base::prob_analyzer()->CalculateTotalProbability(p_vars_);
  assert(p_e >= 0 && p_e <= 1);

  // Calculate P(top/Not event)
  p_vars_[index] = 0;
  double p_not_e = Base::prob_analyzer()->CalculateTotalProbability(p_vars_);
  assert(p_not_e >= 0 && p_not_e <= 1);

  // Restore the probability for next importance calculation.
  p_vars_[index] = p_store;

  return p_e - p_not_e;
}

/// Specialization of importance analyzer with Binary Decision Diagrams.
template <>
class ImportanceAnalyzer<Bdd> : public ImportanceAnalyzerBase<Bdd> {
 public:
  /// Constructs importance analyzer from probability analyzer.
  /// Probability analyzer facilities are used
  /// to calculate the total and conditional probabilities for factors.
  ///
  /// @param[in] prob_analyzer  Instantiated probability analyzer.
  explicit ImportanceAnalyzer(ProbabilityAnalyzer<Bdd>* prob_analyzer)
      : ImportanceAnalyzerBase(prob_analyzer),
        bdd_graph_(prob_analyzer->bdd_graph()) {}

 private:
  double CalculateMif(int index) noexcept override;

  /// Calculates Marginal Importance Factor of a variable.
  ///
  /// @param[in] vertex  The root vertex of a function graph.
  /// @param[in] order  The identifying order of the variable.
  /// @param[in] mark  A flag to mark traversed vertices.
  ///
  /// @returns Importance factor value.
  ///
  /// @note Probability factor fields are used to save results.
  /// @note The graph needs cleaning its marks after this function
  ///       because the graph gets continuously-but-partially marked.
  double CalculateMif(const Bdd::VertexPtr& vertex, int order,
                      bool mark) noexcept;

  /// Retrieves memorized probability values for BDD function graphs.
  ///
  /// @param[in] vertex  Vertex with calculated probabilities.
  ///
  /// @returns Saved probability value of the vertex.
  double RetrieveProbability(const Bdd::VertexPtr& vertex) noexcept;

  Bdd* bdd_graph_;  ///< Binary decision diagram for the analyzer.
};

}  // namespace core
}  // namespace scram

#endif  // SCRAM_SRC_IMPORTANCE_ANALYSIS_H_
