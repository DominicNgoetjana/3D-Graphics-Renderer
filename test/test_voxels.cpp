#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_voxels.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

const int resx = 1000;
const int resy = 2050;
const int resz = 3125;

void TestVoxels::setUp()
{
    vox = new VoxelVolume(resx, resy, resz);
}

void TestVoxels::tearDown()
{
    delete vox;
}

void TestVoxels::testDim()
{
    int dx, dy, dz, padx;

    vox->getDim(dx, dy, dz);
    // std::cerr << "dimx = " << dx << " dimy = " << dy << " dimz = " << dz << std::endl;

    // round resx up to nearest unit of 32
    padx = (int) ceil((float) resx / 32.0) * 32;
    CPPUNIT_ASSERT(dx == padx);
    CPPUNIT_ASSERT(dy == resy);
    CPPUNIT_ASSERT(dz == resz);
}

void TestVoxels::testFill()
{
    int x, y, z, dx, dy, dz;

    vox->getDim(dx, dy, dz);

    // set all to empty and check
    vox->fill(false);
    for(z = 0; z < dz; z+=10)
        for(y = 0; y < dy; y+=10)
            for(x = 0; x < dx; x+=10)
                CPPUNIT_ASSERT(!vox->get(x, y, z));

    // set all to occupied and check
    vox->fill(true);
    for(z = 0; z < dz; z+=10)
        for(y = 0; y < dy; y+=10)
            for(x = 0; x < dx; x+=10)
                CPPUNIT_ASSERT(vox->get(x, y, z));
}

void TestVoxels::testBounds()
{
    int dx, dy, dz;

    vox->getDim(dx, dy, dz);

    // set and test on before, on and after multiple of 32 boundary
    vox->fill(false);
    vox->set(31, 100, 300, true);
    CPPUNIT_ASSERT(vox->get(31, 100, 300));
    CPPUNIT_ASSERT(!vox->get(32, 100, 300));
    CPPUNIT_ASSERT(!vox->get(32, 100, 300));
    vox->fill(false);
    vox->set(32, 100, 300, true);
    CPPUNIT_ASSERT(!vox->get(31, 100, 300));
    CPPUNIT_ASSERT(vox->get(32, 100, 300));
    CPPUNIT_ASSERT(!vox->get(33, 100, 300));
    vox->fill(false);
    vox->set(33, 100, 300, true);
    CPPUNIT_ASSERT(!vox->get(31, 100, 300));
    CPPUNIT_ASSERT(!vox->get(32, 100, 300));
    CPPUNIT_ASSERT(vox->get(33, 100, 300));

    // also test inverse
    vox->fill(true);
    vox->set(31, 100, 300, false);
    CPPUNIT_ASSERT(!vox->get(31, 100, 300));
    CPPUNIT_ASSERT(vox->get(32, 100, 300));
    CPPUNIT_ASSERT(vox->get(33, 100, 300));
    vox->fill(true);
    vox->set(32, 100, 300, false);
    CPPUNIT_ASSERT(vox->get(31, 100, 300));
    CPPUNIT_ASSERT(!vox->get(32, 100, 300));
    CPPUNIT_ASSERT(vox->get(33, 100, 300));
    vox->fill(true);
    vox->set(33, 100, 300, false);
    CPPUNIT_ASSERT(vox->get(31, 100, 300));
    CPPUNIT_ASSERT(vox->get(32, 100, 300));
    CPPUNIT_ASSERT(!vox->get(33, 100, 300));

    // test setting on corners
    vox->fill(false);
    vox->set(0, 0, 0, true);
    vox->set(dx-1, dy-1, dz-1, true);
    vox->set(dx-1, 0, 0, true);
    vox->set(0, dy-1, 0, true);
    vox->set(0, 0, dz-1, true);
    vox->set(dx-1, dy-1, 0, true);
    vox->set(0, dy-1, dz-1, true);
    vox->set(dx-1, 0, dz-1, true);

    CPPUNIT_ASSERT(vox->get(0, 0, 0));
    CPPUNIT_ASSERT(vox->get(dx-1, dy-1, dz-1));
    CPPUNIT_ASSERT(vox->get(dx-1, 0, 0));
    CPPUNIT_ASSERT(vox->get(0, dy-1, 0));
    CPPUNIT_ASSERT(vox->get(0, 0, dz-1));
    CPPUNIT_ASSERT(vox->get(dx-1, dy-1, 0));
    CPPUNIT_ASSERT(vox->get(0, dy-1, dz-1));
    CPPUNIT_ASSERT(vox->get(dx-1, 0, dz-1));

    // check set on out of bounds
    vox->fill(false);
    CPPUNIT_ASSERT(!vox->set(-1, -1, -1, true));
    CPPUNIT_ASSERT(!vox->set(dx, dy, dz, true));

    // check get on out of bounds
    CPPUNIT_ASSERT(!vox->get(-1, -1, -1));
    CPPUNIT_ASSERT(!vox->get(dx, dy, dz));

}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestVoxels, TestSet::perBuild());
//#endif
