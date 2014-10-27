/**
 * @file
 *
 * Helper code for asserting that two @ref Region objects are equal.
 */

#ifndef UTS_TEST_COMMON_TEST_REGION_H
#define UTS_TEST_COMMON_TEST_REGION_H

#include <string>
#include <cppunit/extensions/HelperMacros.h>

struct Region;

namespace CppUnit
{

template<>
class assertion_traits<Region>
{
public:
    static bool equal(const Region &a, const Region &b);
    static std::string toString(const Region &region);
};

} // namespace CppUnit

#endif /* !UTS_TEST_COMMON_TEST_REGION_H */
