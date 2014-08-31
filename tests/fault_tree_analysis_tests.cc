#include "fault_tree_analysis_tests.h"

TEST_F(FaultTreeAnalysisTest, NO_GATE) {
  std::vector<SupersetPtr> sets;

  // Testing for some UNKNOWN gate.
  SetUpGate("unknown_gate");
  inter->AddChild(A);
  inter->AddChild(B);
  inter->AddChild(C);
  GetIndices();
  EXPECT_THROW(ExpandSets(inter_id, sets), ValueError);
}

TEST_F(FaultTreeAnalysisTest, OR_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;
  std::set<int> result_set;

  // Testing for OR gate.
  SetUpGate("or");
  inter->AddChild(A);
  inter->AddChild(B);
  inter->AddChild(C);
  inter->AddChild(D);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(4, sets.size());
  bool a_found = false;
  bool b_found = false;
  bool c_found = false;
  bool d_found = false;
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    if (!(*it_set)->primes().empty()) {
      std::set<int> result = (*it_set)->primes();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(a_id) + result.count(b_id)
                + result.count(c_id));
      if (!a_found && result.count(a_id)) a_found = true;
      else if (!b_found && result.count(b_id)) b_found = true;
      else if (!c_found && result.count(c_id)) c_found = true;
    } else {
      std::set<int> result = (*it_set)->gates();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(d_id));
      d_found = true;
    }
  }
  EXPECT_EQ(true, a_found && b_found && c_found && d_found);
  // Negative OR gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));
  EXPECT_EQ(1, sets.size());
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(3, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * a_id));
  EXPECT_EQ(1, result_set.count(-1 * b_id));
  EXPECT_EQ(1, result_set.count(-1 * c_id));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * d_id));
}

TEST_F(FaultTreeAnalysisTest, AND_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;
  std::set<int> result_set;

  // Testing for AND gate.
  SetUpGate("and");
  inter->AddChild(A);
  inter->AddChild(B);
  inter->AddChild(C);
  inter->AddChild(D);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(1, sets.size());
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(3, result_set.size());
  EXPECT_EQ(1, result_set.count(a_id));
  EXPECT_EQ(1, result_set.count(b_id));
  EXPECT_EQ(1, result_set.count(c_id));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(d_id));
  // Negative AND gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));
  EXPECT_EQ(4, sets.size());
  bool a_found = false;
  bool b_found = false;
  bool c_found = false;
  bool d_found = false;
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    if (!(*it_set)->primes().empty()) {
      std::set<int> result = (*it_set)->primes();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(-1 * a_id) + result.count(-1 * b_id)
                + result.count(-1 * c_id));
      if (!a_found && result.count(-1 * a_id)) a_found = true;
      else if (!b_found && result.count(-1 * b_id)) b_found = true;
      else if (!c_found && result.count(-1 * c_id)) c_found = true;
    } else {
      std::set<int> result = (*it_set)->gates();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(-1 * d_id));
      d_found = true;
    }
  }
  EXPECT_EQ(true, a_found && b_found && c_found && d_found);
}

TEST_F(FaultTreeAnalysisTest, NOT_GATE) {
  std::vector<SupersetPtr> sets;
  std::set<int> result_set;

  // Testing for NOT gate with a primary event child.
  SetUpGate("not");
  inter->AddChild(A);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * a_id));
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));  // Negative Gate.
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(a_id));

  // Testing for NOT gate with a intermediate event child.
  // delete fta();
  new_fta(new FaultTreeAnalysis("default"));
  SetUpGate("not");
  inter->AddChild(D);
  GetIndices();
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * d_id));
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));  // Negative Gate.
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(d_id));
}

TEST_F(FaultTreeAnalysisTest, NOR_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;
  std::set<int> result_set;

  // Testing for NOR gate.
  SetUpGate("nor");
  inter->AddChild(A);
  inter->AddChild(B);
  inter->AddChild(C);
  inter->AddChild(D);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(1, sets.size());
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(3, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * a_id));
  EXPECT_EQ(1, result_set.count(-1 * b_id));
  EXPECT_EQ(1, result_set.count(-1 * c_id));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * d_id));
  // Negative NOR gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));  // Negative Gate.
  EXPECT_EQ(4, sets.size());
  bool a_found = false;
  bool b_found = false;
  bool c_found = false;
  bool d_found = false;
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    if (!(*it_set)->primes().empty()) {
      std::set<int> result = (*it_set)->primes();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(a_id) + result.count(b_id)
                + result.count(c_id));
      if (!a_found && result.count(a_id)) a_found = true;
      else if (!b_found && result.count(b_id)) b_found = true;
      else if (!c_found && result.count(c_id)) c_found = true;
    } else {
      std::set<int> result = (*it_set)->gates();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(d_id));
      d_found = true;
    }
  }
  EXPECT_EQ(true, a_found && b_found && c_found && d_found);
}

TEST_F(FaultTreeAnalysisTest, NAND_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;
  std::set<int> result_set;

  // Testing for NAND gate.
  SetUpGate("nand");
  inter->AddChild(A);
  inter->AddChild(B);
  inter->AddChild(C);
  inter->AddChild(D);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(4, sets.size());
  bool a_found = false;
  bool b_found = false;
  bool c_found = false;
  bool d_found = false;
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    if (!(*it_set)->primes().empty()) {
      std::set<int> result = (*it_set)->primes();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(-1 * a_id) + result.count(-1 * b_id)
                + result.count(-1 * c_id));
      if (!a_found && result.count(-1 * a_id)) a_found = true;
      else if (!b_found && result.count(-1 * b_id)) b_found = true;
      else if (!c_found && result.count(-1 * c_id)) c_found = true;
    } else {
      std::set<int> result = (*it_set)->gates();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(-1 * d_id));
      d_found = true;
    }
  }
  EXPECT_EQ(true, a_found && b_found && c_found && d_found);
  // Negative NAND gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));
  EXPECT_EQ(1, sets.size());
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(3, result_set.size());
  EXPECT_EQ(1, result_set.count(a_id));
  EXPECT_EQ(1, result_set.count(b_id));
  EXPECT_EQ(1, result_set.count(c_id));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(d_id));
}

TEST_F(FaultTreeAnalysisTest, XOR_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;
  std::set<int> result_set;

  // Testing for XOR gate.
  SetUpGate("xor");
  inter->AddChild(A);
  inter->AddChild(D);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(2, sets.size());
  std::set<int> set_one;
  std::set<int> set_two;
  std::set<int> result_one;
  std::set<int> result_two;
  set_one.insert(*(*sets.begin())->primes().begin());
  set_one.insert(*(*sets.begin())->gates().begin());
  set_two.insert(*(*++sets.begin())->primes().begin());
  set_two.insert(*(*++sets.begin())->gates().begin());
  result_one.insert(a_id);
  result_one.insert(-1 * d_id);
  result_two.insert(-1 * a_id);
  result_two.insert(d_id);
  ASSERT_TRUE(set_one.count(a_id) || set_one.count(-1 * a_id));
  if (set_one.count(a_id)) {
    ASSERT_EQ(result_one, set_one);
    ASSERT_EQ(result_two, set_two);
  } else {
    ASSERT_EQ(result_two, set_one);
    ASSERT_EQ(result_one, set_two);
  }
  // Negative XOR gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));
  set_one.clear();
  set_two.clear();
  result_one.clear();
  result_two.clear();
  set_one.insert(*(*sets.begin())->primes().begin());
  set_one.insert(*(*sets.begin())->gates().begin());
  set_two.insert(*(*++sets.begin())->primes().begin());
  set_two.insert(*(*++sets.begin())->gates().begin());
  result_one.insert(a_id);
  result_one.insert(d_id);
  result_two.insert(-1 * a_id);
  result_two.insert(-1 * d_id);
  ASSERT_TRUE(set_one.count(a_id) || set_one.count(-1 * a_id));
  if (set_one.count(a_id)) {
    EXPECT_EQ(result_one, set_one);
    EXPECT_EQ(result_two, set_two);
  } else {
    EXPECT_EQ(result_two, set_one);
    EXPECT_EQ(result_one, set_two);
  }
}

TEST_F(FaultTreeAnalysisTest, NULL_GATE) {
  std::vector<SupersetPtr> sets;
  std::set<int> result_set;

  // Testing for NULL gate with a primary event child.
  SetUpGate("null");
  inter->AddChild(A);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(a_id));
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));  // Negative Gate.
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * a_id));

  // Testing for NULL gate with a intermediate event child.
  // delete fta();
  new_fta(new FaultTreeAnalysis("default"));
  SetUpGate("null");
  inter->AddChild(D);
  GetIndices();
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(d_id));
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));  // Negative Gate.
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(-1 * d_id));
}

TEST_F(FaultTreeAnalysisTest, INHIBIT_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;
  std::set<int> result_set;

  // INHIBIT GATE.
  SetUpGate("inhibit");
  inter->AddChild(A);
  inter->AddChild(D);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(1, sets.size());
  result_set = (*sets.begin())->primes();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(a_id));
  result_set = (*sets.begin())->gates();
  EXPECT_EQ(1, result_set.size());
  EXPECT_EQ(1, result_set.count(d_id));
  // Negative AND gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));
  EXPECT_EQ(2, sets.size());
  bool a_found = false;
  bool d_found = false;
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    if (!(*it_set)->primes().empty()) {
      std::set<int> result = (*it_set)->primes();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(-1 * a_id));
      a_found = true;
    } else {
      std::set<int> result = (*it_set)->gates();
      EXPECT_EQ(1, result.size());
      EXPECT_EQ(1, result.count(-1 * d_id));
      d_found = true;
    }
  }
  EXPECT_EQ(true, a_found && d_found);
}

TEST_F(FaultTreeAnalysisTest, VOTE_GATE) {
  std::vector<SupersetPtr> sets;
  std::vector<SupersetPtr>::iterator it_set;

  // Testing for VOTE gate.
  SetUpGate("vote");
  inter->AddChild(A);
  inter->AddChild(B);
  inter->AddChild(C);
  inter->AddChild(D);
  inter->vote_number(3);
  GetIndices();
  ASSERT_NO_THROW(ExpandSets(inter_id, sets));
  EXPECT_EQ(4, sets.size());
  std::set< std::set<int> > output;
  std::set<int> mcs;
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    mcs.insert((*it_set)->primes().begin(), (*it_set)->primes().end());
    mcs.insert((*it_set)->gates().begin(), (*it_set)->gates().end());
    output.insert(mcs);
    mcs.clear();
  }
  std::set< std::set<int> > exp;
  mcs.clear();
  mcs.insert(a_id);
  mcs.insert(b_id);
  mcs.insert(c_id);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(a_id);
  mcs.insert(b_id);
  mcs.insert(d_id);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(a_id);
  mcs.insert(c_id);
  mcs.insert(d_id);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(b_id);
  mcs.insert(c_id);
  mcs.insert(d_id);
  exp.insert(mcs);
  mcs.clear();

  EXPECT_EQ(exp, output);

  // Negative VOTE gate.
  sets.clear();
  ASSERT_NO_THROW(ExpandSets(-1 * inter_id, sets));
  EXPECT_EQ(6, sets.size());
  output.clear();
  mcs.clear();
  for (it_set = sets.begin(); it_set != sets.end(); ++it_set) {
    mcs.insert((*it_set)->primes().begin(), (*it_set)->primes().end());
    mcs.insert((*it_set)->gates().begin(), (*it_set)->gates().end());
    output.insert(mcs);
    mcs.clear();
  }
  exp.clear();
  mcs.clear();
  mcs.insert(a_id * -1);
  mcs.insert(b_id * -1);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(a_id * -1);
  mcs.insert(d_id * -1);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(a_id * -1);
  mcs.insert(c_id * -1);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(b_id * -1);
  mcs.insert(d_id * -1);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(b_id * -1);
  mcs.insert(c_id * -1);
  exp.insert(mcs);
  mcs.clear();
  mcs.insert(c_id * -1);
  mcs.insert(d_id * -1);
  exp.insert(mcs);
  mcs.clear();

  EXPECT_EQ(exp, output);
}

TEST_F(FaultTreeAnalysisTest, ProbAndInt) {
  std::set<int> min_cut_set;

  // 0 probability for an empty set.
  EXPECT_DOUBLE_EQ(0, ProbAnd(min_cut_set));

  AddPrimeIntProb(0.0);  // Dummy element.

  min_cut_set.insert(1);
  AddPrimeIntProb(0.1);
  EXPECT_DOUBLE_EQ(0.1, ProbAnd(min_cut_set));
  min_cut_set.insert(2);
  AddPrimeIntProb(0.2);
  EXPECT_DOUBLE_EQ(0.02, ProbAnd(min_cut_set));
  min_cut_set.insert(3);
  AddPrimeIntProb(0.3);
  EXPECT_DOUBLE_EQ(0.006, ProbAnd(min_cut_set));

  // Test for negative event calculations.
  min_cut_set.clear();
  min_cut_set.insert(-1);
  EXPECT_DOUBLE_EQ(0.9, ProbAnd(min_cut_set));
  min_cut_set.insert(-2);
  EXPECT_DOUBLE_EQ(0.72, ProbAnd(min_cut_set));
  min_cut_set.insert(3);
  EXPECT_DOUBLE_EQ(0.216, ProbAnd(min_cut_set));
}

TEST_F(FaultTreeAnalysisTest, CombineElAndSet) {
  std::set<int> el_one;
  std::set<int> el_two;
  std::set< std::set<int> > set_one;
  std::set< std::set<int> > set_two;
  std::set< std::set<int> > combo_set;

  // One element checks.
  el_one.insert(1);
  set_one.insert(el_one);  // Insert (1)
  ASSERT_NO_THROW(CombineElAndSet(el_one, set_one, combo_set));
  EXPECT_EQ(set_one, combo_set);  // Must be only (1)
  combo_set.clear();

  el_two.insert(3);
  ASSERT_NO_THROW(CombineElAndSet(el_two, set_one, combo_set));

  set_one.insert(el_two);  // Insert (3)

  EXPECT_EQ(1, combo_set.size());
  el_two.insert(1);
  set_two.insert(el_two);  // set_two is (1,3)
  EXPECT_EQ(set_two, combo_set);  // Must be only (1,3)
  combo_set.clear();

  // Two element checks.
  el_one.insert(2);  // el_one is (1, 2)
  ASSERT_NO_THROW(CombineElAndSet(el_one, set_two, combo_set));

  set_one.insert(el_two);  // Insert (1, 3)

  el_two.insert(2);
  set_two.clear();
  set_two.insert(el_two);
  EXPECT_EQ(set_two, combo_set);  // Expected (1,2,3)
  combo_set.clear();

  // Multi element checks
  set_one.insert(el_one);  // Insert (1, 2)

  // After the above intantiation the set_one is [(1), (3), (1,2), (1,3)].
  // The result of [ el_one AND set_one ] is [(1,2), (1,2,3)].
  EXPECT_EQ(4, set_one.size());
  EXPECT_EQ(2, el_one.size());
  EXPECT_EQ(0, combo_set.size());
  ASSERT_NO_THROW(CombineElAndSet(el_one, set_one, combo_set));
  EXPECT_EQ(2, combo_set.size());
  set_one.clear();  // To construct the expected output set_one.
  set_one.insert(el_one);
  el_one.insert(3);
  set_one.insert(el_one);
  EXPECT_EQ(set_one, combo_set);

  // Testing for operations with negative elements.
  el_one.clear();
  el_two.clear();
  set_one.clear();
  set_two.clear();
  combo_set.clear();

  el_one.insert(-1);
  set_one.insert(el_one);
  ASSERT_NO_THROW(CombineElAndSet(el_one, set_one, combo_set));
  EXPECT_EQ(set_one, combo_set);

  el_two.insert(1);
  combo_set.clear();
  ASSERT_NO_THROW(CombineElAndSet(el_two, set_one, combo_set));
  EXPECT_TRUE(combo_set.empty());
}

TEST_F(FaultTreeAnalysisTest, ProbOrInt) {
  std::set<int> mcs;  // Minimal cut set.
  std::set<std::set<int> > min_cut_sets;  // A set of minimal cut sets.
  AddPrimeIntProb(0.0);  // Dummy element.
  AddPrimeIntProb(0.1);  // A is element 0.
  AddPrimeIntProb(0.2);  // B is element 1.
  AddPrimeIntProb(0.3);  // C is element 2.

  // 0 probability for an empty set.
  EXPECT_DOUBLE_EQ(0, ProbOr(min_cut_sets));

  // Check for one element calculation for A.
  mcs.insert(1);
  min_cut_sets.insert(mcs);
  EXPECT_DOUBLE_EQ(0.1, ProbOr(min_cut_sets));

  // Check that recursive nsums=0 returns without changing anything.
  mcs.insert(1);
  min_cut_sets.insert(mcs);
  EXPECT_EQ(0, ProbOr(min_cut_sets, 0));

  // Check for [A or B]
  min_cut_sets.clear();
  mcs.clear();
  mcs.insert(1);
  min_cut_sets.insert(mcs);
  mcs.clear();
  mcs.insert(2);
  min_cut_sets.insert(mcs);
  EXPECT_DOUBLE_EQ(0.28, ProbOr(min_cut_sets));

  // Check for [A or B or C]
  min_cut_sets.clear();
  mcs.clear();
  mcs.insert(1);
  min_cut_sets.insert(mcs);
  mcs.clear();
  mcs.insert(2);
  min_cut_sets.insert(mcs);
  mcs.clear();
  mcs.insert(3);
  min_cut_sets.insert(mcs);
  EXPECT_DOUBLE_EQ(0.496, ProbOr(min_cut_sets));

  // Check for [(A,B) or (B,C)]
  mcs.clear();
  min_cut_sets.clear();
  mcs.insert(1);
  mcs.insert(2);
  min_cut_sets.insert(mcs);
  mcs.clear();
  mcs.insert(2);
  mcs.insert(3);
  min_cut_sets.insert(mcs);
  EXPECT_DOUBLE_EQ(0.074, ProbOr(min_cut_sets));
}

// ------------------------ Monte Carlo -----------------------------
TEST_F(FaultTreeAnalysisTest, MProbOr) {
  std::set<int> mcs;  // Minimal cut set.
  std::set< std::set<int> > p_terms;  // Positive terms of the equation.
  std::set< std::set<int> > n_terms;  // Negative terms of the equation.
  std::set< std::set<int> > temp_set;  // Temp set for dumping the output.
  std::set< std::set<int> > min_cut_sets;  // A set of minimal cut sets.
  ASSERT_NO_THROW(MProbOr(min_cut_sets));  // Empty sets.
  ASSERT_TRUE(pos_terms().empty() && neg_terms().empty());

  // Check for one element calculation for A.
  mcs.insert(0);
  p_terms.insert(mcs);
  min_cut_sets.insert(mcs);
  ASSERT_NO_THROW(MProbOr(min_cut_sets));
  EXPECT_EQ(0, min_cut_sets.size());  // This set is emptied recursively.
  temp_set.insert(pos_terms().begin(), pos_terms().end());
  EXPECT_EQ(p_terms, temp_set);

  // Check that recursive nsums=0 returns without changing anything.
  min_cut_sets.insert(mcs);
  pos_terms().clear();
  ASSERT_NO_THROW(MProbOr(min_cut_sets, 1, 0));
  EXPECT_EQ(1, min_cut_sets.size());
  EXPECT_EQ(0, pos_terms().size());

  // Check for [A or B]
  pos_terms().clear();
  neg_terms().clear();
  min_cut_sets.clear();
  mcs.clear();
  p_terms.clear();
  mcs.insert(0);
  p_terms.insert(mcs);
  min_cut_sets.insert(mcs);
  mcs.clear();
  mcs.insert(1);
  p_terms.insert(mcs);
  min_cut_sets.insert(mcs);
  mcs.insert(0);
  n_terms.insert(mcs);
  ASSERT_NO_THROW(MProbOr(min_cut_sets));
  temp_set.clear();
  temp_set.insert(pos_terms().begin(), pos_terms().end());
  EXPECT_EQ(p_terms, temp_set);
  temp_set.clear();
  temp_set.insert(neg_terms().begin(), neg_terms().end());
  EXPECT_EQ(n_terms, temp_set);

  // Check for [(A,B) or (B,C)]
  pos_terms().clear();
  neg_terms().clear();
  min_cut_sets.clear();
  mcs.clear();
  p_terms.clear();
  n_terms.clear();
  pos_terms().clear();
  neg_terms().clear();
  mcs.insert(0);
  mcs.insert(1);
  p_terms.insert(mcs);
  min_cut_sets.insert(mcs);
  mcs.clear();
  mcs.insert(1);
  mcs.insert(2);
  p_terms.insert(mcs);
  min_cut_sets.insert(mcs);
  mcs.insert(0);
  n_terms.insert(mcs);
  ASSERT_NO_THROW(MProbOr(min_cut_sets));
  temp_set.clear();
  temp_set.insert(pos_terms().begin(), pos_terms().end());
  EXPECT_EQ(p_terms, temp_set);
  temp_set.clear();
  temp_set.insert(neg_terms().begin(), neg_terms().end());
  EXPECT_EQ(n_terms, temp_set);
}
// ----------------------------------------------------------------------
// ---------------------- Test Public Functions --------------------------
// Invalid options for the constructor.
TEST_F(FaultTreeAnalysisTest, Constructor) {
  // Incorrect analysis type.
  ASSERT_THROW(FaultTreeAnalysis("analysis"), ValueError);
  // Incorrect approximation argument.
  ASSERT_THROW(FaultTreeAnalysis("default", "approx"), ValueError);
  // Incorrect limit order for minmal cut sets.
  ASSERT_THROW(FaultTreeAnalysis("default", "no", -1), ValueError);
  // Incorrect number of series in the probability equation.
  ASSERT_THROW(FaultTreeAnalysis("default", "no", 1, -1), ValueError);
  // Incorrect cut-off probability.
  ASSERT_THROW(FaultTreeAnalysis("default", "no", 1, 1, -1), ValueError);
  ASSERT_THROW(FaultTreeAnalysis("default", "no", 1, 1, 10), ValueError);
}