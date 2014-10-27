#ifndef TILER_TEST_SCENE_H
#define TILER_TEST_SCENE_H


#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/scene.h"

/// Test code for @ref Scene
class TestScene : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestScene);
    CPPUNIT_TEST(testTesselate);
    CPPUNIT_TEST(testValidity);
    CPPUNIT_TEST_SUITE_END();

private:
    Scene * scene;

public:

    void setUp();

    void tearDown();

    /// run standard validity tests on scene
    void testTesselate();

    /// test the validity methods themselves to ensure that they correctly identify failure
    void testValidity();
};

#endif /* !TILER_TEST_SCENE_H */
