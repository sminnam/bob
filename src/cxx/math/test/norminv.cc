/**
 * @file src/cxx/math/test/norminv.cc
 * @author <a href="mailto:Laurent.El-Shafey@idiap.ch">Laurent El Shafey</a> 
 *
 * @brief Test the inverse normal cumulative distribution
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE math-norminv Tests
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include "math/norminv.h"

namespace math = Torch::math;

struct T {
  double sols_d05, sols_d25, sols_d50, sols_d75, sols_d95;
  double sol_m2s4_d37, sol_m2s4_d48, sol_m2s4_d79;
  double eps;

  T(): sols_d05(-1.64485362695), sols_d25(-0.674489750196), sols_d50(0.), 
    sols_d75(0.674489750196), sols_d95(1.64485362695), 
    sol_m2s4_d37(0.672586614252), sol_m2s4_d48(1.799385666141), 
    sol_m2s4_d79(5.225684988072), eps(1e-5)
  { }

  ~T() {}
};

BOOST_FIXTURE_TEST_SUITE( test_setup, T )

BOOST_AUTO_TEST_CASE( test_normsinv )
{
  // Compute and compare again OpenOffice! reference values
  BOOST_CHECK_SMALL( fabs( sols_d05 - math::normsinv(0.05)), eps);
  BOOST_CHECK_SMALL( fabs( sols_d25 - math::normsinv(0.25)), eps);
  BOOST_CHECK_SMALL( fabs( sols_d50 - math::normsinv(0.5)), eps);
  BOOST_CHECK_SMALL( fabs( sols_d75 - math::normsinv(0.75)), eps);
  BOOST_CHECK_SMALL( fabs( sols_d95 - math::normsinv(0.95)), eps);
}

BOOST_AUTO_TEST_CASE( test_norminv )
{
  // Compute and compare again OpenOffice! reference values
  BOOST_CHECK_SMALL( fabs( sol_m2s4_d37 - math::norminv(0.37, 2., 4.)), eps);
  BOOST_CHECK_SMALL( fabs( sol_m2s4_d48 - math::norminv(0.48, 2., 4.)), eps);
  BOOST_CHECK_SMALL( fabs( sol_m2s4_d79 - math::norminv(0.79, 2., 4.)), eps);
}

BOOST_AUTO_TEST_SUITE_END()
