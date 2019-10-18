#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_csg.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

void TestCSG::setUp()
{
    csg = new Scene();
}

void TestCSG::tearDown()
{
    delete csg;
}

void TestCSG::testSimpleCSG()
{
    // unit tests could be made much more comprehensive than this
    // in particular this fails to test intersection

    cerr << "START CSG SIMPLE SCENE" << endl;
    csg->sampleScene();
    csg->voxelise(0.1f);
    CPPUNIT_ASSERT(!csg->getVox()->get(100,100,100)); // inside extracted cylinder
    CPPUNIT_ASSERT(!csg->getVox()->get(50,150,100)); // just outside central sphere
    CPPUNIT_ASSERT(csg->getVox()->get(70,100,100)); // just inside central sphere
    CPPUNIT_ASSERT(csg->getVox()->get(150,150,100)); // inside extruding cylinder

    cerr << "CSG SIMPLE SCENE PASSED" << endl << endl;
}

void TestCSG::testIntersectCSG()
{
    cerr << "START CSG INTERSECT SCENE" << endl;
    csg->intersectScene();
    csg->voxelise(0.1f);
    CPPUNIT_ASSERT(csg->getVox()->get(101,101,101)); // inside intersection
    CPPUNIT_ASSERT(!csg->getVox()->get(101,101,190)); // inside cylinder, outside sphere
    CPPUNIT_ASSERT(!csg->getVox()->get(190,101,101)); // inside sphere, outside cylinder
    CPPUNIT_ASSERT(!csg->getVox()->get(0,0,0)); // outside all

    cerr << "CSG INTERSECT SCENE PASSED" << endl << endl;
}

void TestCSG::testExpensiveCSG()
{
    cerr << "START CSG EXPENSIVE SCENE" << endl;
    csg->expensiveScene("meshes/bunny.stl");
    csg->voxelise(0.5f);
    // CPPUNIT_ASSERT(csg->getVox()->get(36,36,0)); // inside cylinder
    CPPUNIT_ASSERT(csg->getVox()->get(21,21,21)); // inside bunny and cyl
    CPPUNIT_ASSERT(!csg->getVox()->get(40,0,40)); // outside all

    cerr << "CSG COMPLEX SCENE PASSED" << endl << endl;
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestCSG, TestSet::perBuild());
//#endif
