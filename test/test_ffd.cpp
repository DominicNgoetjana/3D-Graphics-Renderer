#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_ffd.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

void TestFFD::setUp()
{
    cgp::Point o = cgp::Point(0.0f, 0.0f, 0.0f);
    cgp::Vector d = cgp::Vector(1.0f, 1.0f, 1.0f);
    linearffd = new ffd(2,2,2,o,d);
    cubicffd = new ffd(4,4,4,o,d);
    mixffd = new ffd(2,3,4,o,d);
}

void TestFFD::tearDown()
{
    delete linearffd;
    delete cubicffd;
    delete mixffd;
}

void TestFFD::testCorners()
{
    cgp::Point c1, c2, d1, d2, p;

    d1 = cgp::Point(-1.0f, -1.0f, -1.0f);
    d2 = cgp::Point(2.0f, 2.0f, 2.0f);
    c1 = cgp::Point(0.0f, 0.0f, 0.0f);
    c2 = cgp::Point(1.0f, 1.0f, 1.0f);

    // test origin and far corner
    // by endpoint interpolation property of bezier curves
    // points at u,v,w = (0,0,0) and u,v,w = (1,1,1) should track the corner CP exactly

    // linear case
    linearffd->setCP(0,0,0,d1);
    linearffd->setCP(1,1,1,d2);
    p = c1; linearffd->deform(p);
    CPPUNIT_ASSERT(p == d1);
    p = c2; linearffd->deform(p);
    CPPUNIT_ASSERT(p == d2);

    // cubic case
    linearffd->setCP(0,0,0,d1);
    linearffd->setCP(3,3,3,d2);
    p = c1; linearffd->deform(p);
    CPPUNIT_ASSERT(p == d1);
    p = c2; linearffd->deform(p);
    CPPUNIT_ASSERT(p == d2);

    // mixed dimension case
    linearffd->setCP(0,0,0,d1);
    linearffd->setCP(1,2,3,d2);
    p = c1; linearffd->deform(p);
    CPPUNIT_ASSERT(p == d1);
    p = c2; linearffd->deform(p);
    CPPUNIT_ASSERT(p == d2);

    cerr << "FFD CORNER TEST PASSED" << endl;
}

void TestFFD::testCenter()
{
    cgp::Point c, d1, d2, p;

    d1 = cgp::Point(-1.0f, -1.0f, -1.0f);
    d2 = cgp::Point(2.0f, 2.0f, 2.0f);
    c = cgp::Point(0.5f, 0.5f, 0.5f);

    // test center
    // corner are moved in opposite directions by equal amounts so
    // center should remain unchanged

    // linear case
    linearffd->setCP(0,0,0,d1);
    linearffd->setCP(1,1,1,d2);
    p = c; linearffd->deform(p);
    CPPUNIT_ASSERT(p == c);

    // cubic case
    linearffd->setCP(0,0,0,d1);
    linearffd->setCP(3,3,3,d2);
    p = c; linearffd->deform(p);
    CPPUNIT_ASSERT(p == c);

    // mixed dimension case
    linearffd->setCP(0,0,0,d1);
    linearffd->setCP(1,2,3,d2);
    p = c; linearffd->deform(p);
    CPPUNIT_ASSERT(p == c);

    cerr << "FFD CENTER TEST PASSED" << endl;
}

void TestFFD::testIdentity()
{
    cgp::Point pre, post, origin;
    cgp::Vector diag;

    // lattices with random origin and diagonal
    linearffd->setFrame(cgp::Point((float) (rand()%1000), (float) (rand()%1000), (float) (rand()%1000)), cgp::Vector((float) (rand()%1000), (float) (rand()%1000), (float) (rand()%1000)));
    cubicffd->setFrame(cgp::Point((float) (rand()%1000), (float) (rand()%1000), (float) (rand()%1000)), cgp::Vector((float) (rand()%1000), (float) (rand()%1000), (float) (rand()%1000)));
    mixffd->setFrame(cgp::Point((float) (rand()%1000), (float) (rand()%1000), (float) (rand()%1000)), cgp::Vector((float) (rand()%1000), (float) (rand()%1000), (float) (rand()%1000)));

    srand (time(NULL));
    // test that random points within undeformed lattices are return unchanged
    for(int i = 0; i < 100; i++)
    {
        // random point in lattice
        pre = cgp::Point(((float) (rand()%1000))/1000.0f*diag.i-origin.x,((float) (rand()%1000))/1000.0f*diag.i-origin.y,((float) (rand()%1000))/1000.0f*diag.k-origin.z);

        // now test identity transforms
        post = pre;
        linearffd->deform(post);
        CPPUNIT_ASSERT(pre == post);
        post = pre;
        cubicffd->deform(post);
        CPPUNIT_ASSERT(pre == post);
        post = pre;
        mixffd->deform(post);
        CPPUNIT_ASSERT(pre == post);
    }
    cerr << "FFD IDENTITY TEST PASSED" << endl << endl;
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestFFD, TestSet::perBuild());
//#endif
