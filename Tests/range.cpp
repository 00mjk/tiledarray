#include "TiledArray/range.h"
#include "TiledArray/permutation.h"
#include "unit_test_config.h"
#include "range_fixture.h"
#include <sstream>

using namespace TiledArray;

RangeFixture::RangeFixture() : r(start, finish)
{
}

const RangeFixture::index RangeFixture::start         = fill_index<RangeFixture::index>(0);
const RangeFixture::index RangeFixture::finish        = fill_index<RangeFixture::index>(5);
const RangeFixture::size_array RangeFixture::size     = RangeFixture::finish.data();
const RangeFixture::size_array RangeFixture::weight   = GlobalFixture::coordinate_system::calc_weight(RangeFixture::size);
const RangeFixture::volume_type RangeFixture::volume  = GlobalFixture::coordinate_system::calc_volume(RangeFixture::size);
const RangeFixture::index RangeFixture::p0            = fill_index<RangeFixture::index>(0);
const RangeFixture::index RangeFixture::p1            = fill_index<RangeFixture::index>(1);
const RangeFixture::index RangeFixture::p2            = fill_index<RangeFixture::index>(2);
const RangeFixture::index RangeFixture::p3            = fill_index<RangeFixture::index>(3);
const RangeFixture::index RangeFixture::p4            = fill_index<RangeFixture::index>(4);
const RangeFixture::index RangeFixture::p5            = fill_index<RangeFixture::index>(5);
const RangeFixture::index RangeFixture::p6            = fill_index<RangeFixture::index>(6);


BOOST_FIXTURE_TEST_SUITE( range_suite, RangeFixture )

BOOST_AUTO_TEST_CASE( dimension_accessor )
{
  BOOST_CHECK_EQUAL(r.start(), start);   // check start()
  BOOST_CHECK_EQUAL(r.finish(), finish);  // check finish()
  BOOST_CHECK_EQUAL(r.size(), size); // check size()
  BOOST_CHECK_EQUAL(r.weight(), weight); // check weight()
  BOOST_CHECK_EQUAL(r.volume(), volume);  // check volume()
}

BOOST_AUTO_TEST_CASE( constructors )
{
  BOOST_REQUIRE_NO_THROW(RangeN r0); // Default Constructor
  RangeN r0;
  BOOST_CHECK_EQUAL(r0.start(), start);
  BOOST_CHECK_EQUAL(r0.finish(), start);
  BOOST_CHECK_EQUAL(r0.size(), start.data());
  BOOST_CHECK_EQUAL(r0.volume(), 0u);

  BOOST_REQUIRE_NO_THROW(RangeN r2(p2, p2 + finish)); // Start/Finish Constructor
#ifdef TA_EXCEPTION_ERROR
  BOOST_CHECK_THROW(RangeN r2(p2 + finish, p2), std::runtime_error);
#endif
  RangeN r2(p2, p2 + finish);
  BOOST_CHECK_EQUAL(r2.start(), p2);
  BOOST_CHECK_EQUAL(r2.finish(), p2 + finish);
  BOOST_CHECK_EQUAL(r2.size(), size);
  BOOST_CHECK_EQUAL(r2.volume(), volume);

  BOOST_REQUIRE_NO_THROW(RangeN r4(r)); // Copy Constructor
  RangeN r4(r);
  BOOST_CHECK_EQUAL(r4.start(), start);
  BOOST_CHECK_EQUAL(r4.finish(), finish);
  BOOST_CHECK_EQUAL(r4.size(), size);
  BOOST_CHECK_EQUAL(r4.volume(), volume);

  BOOST_REQUIRE_NO_THROW(RangeN r5(p2, p2)); // Zero Size Construction
  RangeN r5(p2, p2);
  BOOST_CHECK_EQUAL(r5.start(), p2);
  BOOST_CHECK_EQUAL(r5.finish(), p2);
  BOOST_CHECK_EQUAL(r5.size(), start.data());
  BOOST_CHECK_EQUAL(r5.volume(), 0u);
}

BOOST_AUTO_TEST_CASE( ostream )
{
  std::stringstream stm;
  stm << "[ " << start << ", " << finish << " )";

  boost::test_tools::output_test_stream output;
  output << r;
  BOOST_CHECK( !output.is_empty( false ) ); // check for correct output.
  BOOST_CHECK( output.check_length( stm.str().size(), false ) );
  BOOST_CHECK( output.is_equal(stm.str().c_str()) );
}

BOOST_AUTO_TEST_CASE( comparision )
{
  RangeN r1(r);
  RangeN r2(p0, p1);
  BOOST_CHECK(r1 == r); // check operator==
  BOOST_CHECK( ! (r2 == r) ); // check for failure
  BOOST_CHECK(r2 != r); // check operator!=
  BOOST_CHECK( ! (r1 != r) ); // check for failure
}

BOOST_AUTO_TEST_CASE( assignment )
{
  RangeN r1;
  BOOST_CHECK_EQUAL( (r1 = r), r); // check that assignment returns itself.
  BOOST_CHECK_EQUAL(r1, r);        // check that assignment is correct.

  RangeN r2 = r;
  BOOST_CHECK_EQUAL(r2, r); // check construction assignment.
}

BOOST_AUTO_TEST_CASE( resize )
{
  RangeN r1;

  // Check initial conditions
  BOOST_CHECK_EQUAL(r1.volume(), 0ul);

  // check resize and return value
  BOOST_CHECK_EQUAL(r1.resize(start, finish), r);
  // check that size was correctly recalculated
  BOOST_CHECK_EQUAL(r1.size(), r.size());
  // Check that weight was correctly recalculated
  BOOST_CHECK_EQUAL(r1.weight(), r.weight());
  // Check that volume was correctly recalculated
  BOOST_CHECK_EQUAL(r1.volume(), r.volume());
}

BOOST_AUTO_TEST_CASE( permutation )
{
  index s;
  index f;
  size_array a;
  for(unsigned int d = 0; d < GlobalFixture::coordinate_system::dim; ++d) {
    s[d] = d;
    f[d] = d + d * 5;
    a[GlobalFixture::coordinate_system::dim - d - 1] = d;
  }
  RangeN r1(s, f);
  // create a reverse order permutation
  Permutation<3> p(a.begin());
  RangeN r2 = p ^ r1;
  RangeN r3 = r1;

  // check start, finish, size, volume, and weight of permuted range
  BOOST_CHECK_EQUAL_COLLECTIONS(r1.start().rbegin(), r1.start().rend(),
                                r2.start().begin(),  r2.start().end());
  BOOST_CHECK_EQUAL_COLLECTIONS(r1.finish().rbegin(), r1.finish().rend(),
                                r2.finish().begin(),  r2.finish().end());
  BOOST_CHECK_EQUAL_COLLECTIONS(r1.size().rbegin(), r1.size().rend(),
                                r2.size().begin(),  r2.size().end());
  BOOST_CHECK_EQUAL(r2.volume(), r1.volume());

  BOOST_CHECK_EQUAL(r2.weight(), GlobalFixture::coordinate_system::calc_weight(r2.size()));

  // check for correct finish permutation
  BOOST_CHECK_EQUAL(r3 ^= p, r2);
  BOOST_CHECK_EQUAL(r3.start(), r2.start());
  BOOST_CHECK_EQUAL(r3.finish(), r2.finish());
  BOOST_CHECK_EQUAL(r3.size(), r2.size());
  BOOST_CHECK_EQUAL(r3.volume(), r2.volume());
  BOOST_CHECK_EQUAL(r3.weight(), r2.weight());
  BOOST_CHECK_EQUAL(r3, r2);
}

BOOST_AUTO_TEST_CASE( include )
{
  typedef Range<CoordinateSystem<3> > Range3;
  typedef Range3::index index;
  typedef Range3::key_type key_type;

  index s(1,1,1);
  index f(5,5,5);
  Range3 r1(s, f);
  index t1(0,3,3);
  index t2(1,3,3);
  index t3(2,3,3);
  index t4(4,3,3);
  index t5(5,3,3);
  index t6(6,3,3);
  index t7(0,0,3);
  index t8(1,1,3);
  index t9(2,2,3);
  index t10(4,4,3);
  index t11(5,5,3);
  index t12(6,6,3);
  index t13(0,6,3);
  index t14(1,5,3);
  index t15(2,4,3);
  index t16(4,2,3);
  index t17(5,1,3);
  index t18(6,0,3);
  index t19(1,4,3);
  index t20(4,1,3);

  BOOST_CHECK(! r1.includes(t1)); // check side include
  BOOST_CHECK(r1.includes(t2));
  BOOST_CHECK(r1.includes(t3));
  BOOST_CHECK(r1.includes(t4));
  BOOST_CHECK(!r1.includes(t5));
  BOOST_CHECK(!r1.includes(t6));
  BOOST_CHECK(!r1.includes(t7)); // check diagonal include
  BOOST_CHECK(r1.includes(t8));
  BOOST_CHECK(r1.includes(t9));
  BOOST_CHECK(r1.includes(t10));
  BOOST_CHECK(!r1.includes(t11));
  BOOST_CHECK(!r1.includes(t12));
  BOOST_CHECK(!r1.includes(t13)); // check other diagonal include
  BOOST_CHECK(!r1.includes(t14));
  BOOST_CHECK(r1.includes(t15));
  BOOST_CHECK(r1.includes(t16));
  BOOST_CHECK(!r1.includes(t17));
  BOOST_CHECK(!r1.includes(t18));
  BOOST_CHECK(r1.includes(t19));  // check corners
  BOOST_CHECK(r1.includes(t20));

  BOOST_CHECK(! r1.includes(key_type(t1))); // check side include
  BOOST_CHECK(r1.includes(key_type(t2)));
  BOOST_CHECK(r1.includes(key_type(t3)));
  BOOST_CHECK(r1.includes(key_type(t4)));
  BOOST_CHECK(!r1.includes(key_type(t5)));
  BOOST_CHECK(!r1.includes(key_type(t6)));
  BOOST_CHECK(!r1.includes(key_type(t7))); // check diagonal include
  BOOST_CHECK(r1.includes(key_type(t8)));
  BOOST_CHECK(r1.includes(key_type(t9)));
  BOOST_CHECK(r1.includes(key_type(t10)));
  BOOST_CHECK(!r1.includes(key_type(t11)));
  BOOST_CHECK(!r1.includes(key_type(t12)));
  BOOST_CHECK(!r1.includes(key_type(t13))); // check other diagonal include
  BOOST_CHECK(!r1.includes(key_type(t14)));
  BOOST_CHECK(r1.includes(key_type(t15)));
  BOOST_CHECK(r1.includes(key_type(t16)));
  BOOST_CHECK(!r1.includes(key_type(t17)));
  BOOST_CHECK(!r1.includes(key_type(t18)));
  BOOST_CHECK(r1.includes(key_type(t19)));  // check corners
  BOOST_CHECK(r1.includes(key_type(t20)));

  Range3::ordinal_index o = 0;
  for(; o < r.volume(); ++o) {
    BOOST_CHECK(r.includes(o));
    BOOST_CHECK(r.includes(key_type(o)));
  }

  BOOST_CHECK(! r.includes(o));
  BOOST_CHECK(r.includes(key_type(o)));
}

BOOST_AUTO_TEST_CASE( unions )
{
  RangeN r1(p1, p3);
  RangeN r2(p2, p4);

  RangeN ru1 = r1 & r2;
  RangeN ru2 = r2 & r1;

  BOOST_CHECK_EQUAL(ru1.start(), p2);  // check with r2 start inside r1
  BOOST_CHECK_EQUAL(ru1.finish(), p3);
  BOOST_CHECK_EQUAL(ru1, ru2);         // and vis versa

  RangeN r3(p1, p2);
  RangeN r4(p3, p4);
  RangeN ru3 = r3 & r4;
  BOOST_CHECK_EQUAL(ru3.start(), start);  // no over lap
  BOOST_CHECK_EQUAL(ru3.finish(), start);
  BOOST_CHECK_EQUAL(ru3.volume(), 0u);

  RangeN r5(p1, p4);
  RangeN r6(p2, p3);
  RangeN ru4 = r5 & r6;
  RangeN ru5 = r6 & r5;
  BOOST_CHECK_EQUAL(ru4, r6); // check contained block
  BOOST_CHECK_EQUAL(ru5, r6);

  index p5(2, 1, 1);
  index p6(6, 5, 2);
  index p7(1, 2, 1);
  index p8(5, 6, 2);
  index p9(2, 2, 1);
  index p10(5, 5, 2);
  RangeN r7(p5,p6);
  RangeN r8(p7,p8);
  RangeN ru6 = r7 & r8;
  RangeN ru7 = r8 & r7;
  BOOST_CHECK_EQUAL(ru6.start(), p9);  // check union when start & finish are
  BOOST_CHECK_EQUAL(ru6.finish(), p10);// not inside each other.
  BOOST_CHECK_EQUAL(ru6, ru7);

  index p11(2, 1, 1);
  index p12(5, 6, 2);
  index p13(1, 2, 1);
  index p14(6, 5, 2);
  index p15(2, 2, 1);
  index p16(5, 5, 2);
  RangeN r9(p11,p12);
  RangeN r10(p13,p14);
  RangeN ru8 = r9 & r10;
  RangeN ru9 = r10 & r9;
  BOOST_CHECK_EQUAL(ru8.start(), p15);  // check union when start & finish are
  BOOST_CHECK_EQUAL(ru8.finish(), p16);// not inside each other.
  BOOST_CHECK_EQUAL(ru8, ru9);
  BOOST_CHECK_EQUAL(ru8, ru6);
}


BOOST_AUTO_TEST_CASE( iteration )
{
  typedef CoordinateSystem<3> cs3;
  std::vector<index> t(8);

#ifdef TEST_C_STYLE_CS
  typedef CoordinateSystem<3> cs3;
  typedef Range<cs3> Range3;
  t[0] = Range3::index(1,1,1);
  t[1] = Range3::index(1,1,2);
  t[2] = Range3::index(1,2,1);
  t[3] = Range3::index(1,2,2);
  t[4] = Range3::index(2,1,1);
  t[5] = Range3::index(2,1,2);
  t[6] = Range3::index(2,2,1);
  t[7] = Range3::index(2,2,2);
#else // TEST_FORTRAN_CS
  typedef CoordinateSystem<3, 1, TiledArray::detail::increasing_dimension_order> cs3;
  typedef Range<cs3> Range3;
  t[0] = Range3::index(1,1,1);
  t[1] = Range3::index(2,1,1);
  t[2] = Range3::index(1,2,1);
  t[3] = Range3::index(2,2,1);
  t[4] = Range3::index(1,1,2);
  t[5] = Range3::index(2,1,2);
  t[6] = Range3::index(1,2,2);
  t[7] = Range3::index(2,2,2);
#endif

  Range3 r1(Range3::index(1,1,1),Range3::index(3,3,3));
  BOOST_CHECK_EQUAL_COLLECTIONS(r1.begin(), r1.end(), t.begin(), t.end());
}

BOOST_AUTO_TEST_SUITE_END()
