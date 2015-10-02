/*
 * Copyright (C) 2014-2015 Olzhas Rakhimov
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

#include "settings.h"

#include <gtest/gtest.h>

#include "error.h"

namespace scram {
namespace test {

TEST(SettingsTest, IncorrectSetup) {
  Settings s;
  /// Incorrect algorithm.
  EXPECT_THROW(s.algorithm("the-best"), InvalidArgument);
  // Incorrect approximation argument.
  EXPECT_THROW(s.approximation("approx"), InvalidArgument);
  // Incorrect limit order for minimal cut sets.
  EXPECT_THROW(s.limit_order(-1), InvalidArgument);
  // Incorrect cut-off probability.
  EXPECT_THROW(s.cut_off(-1), InvalidArgument);
  EXPECT_THROW(s.cut_off(10), InvalidArgument);
  // Incorrect number of trials.
  EXPECT_THROW(s.num_trials(-10), InvalidArgument);
  EXPECT_THROW(s.num_trials(0), InvalidArgument);
  // Incorrect number of quantiles.
  EXPECT_THROW(s.num_quantiles(-10), InvalidArgument);
  EXPECT_THROW(s.num_quantiles(0), InvalidArgument);
  // Incorrect number of bins.
  EXPECT_THROW(s.num_bins(-10), InvalidArgument);
  EXPECT_THROW(s.num_bins(0), InvalidArgument);
  // Incorrect seed.
  EXPECT_THROW(s.seed(-1), InvalidArgument);
  // Incorrect mission time.
  EXPECT_THROW(s.mission_time(-10), InvalidArgument);
}

}  // namespace test
}  // namespace scram
