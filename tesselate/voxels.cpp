//
// Voxels
//

#include "voxels.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>

using namespace std;

//==========START BLOYD

// source for tables Marching Cubes Example Program by Cory Bloyd (corysama@yahoo.com)
// http://paulbourke.net/geometry/polygonise/marchingsource.cpp)
// code is public domain
// lists the positions, relative to bottom, front, left corner, of each of the 8 vertices of a cube
static const int cubePos[8][3] =
{
    {0, 0, 0},{1, 0, 0},{1, 1, 0},{0, 1, 0},{0, 0, 1},{1, 0, 1},{1, 1, 1},{0, 1, 1}
};

// For any edge, if one vertex is inside of the surface and the other is outside of the surface
//  then the edge intersects the surface
// For each of the 8 vertices of the cube can be two possible states : either inside or outside of the surface
// For any cube the are 2^8=256 possible sets of vertex states
// This table lists the edges intersected by the surface for all 256 possible vertex states
// There are 12 edges.  For each entry in the table, if edge #n is intersected, then bit #n is set to 1
int cubeEdgeFlags[256]=
{
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
};

/*
 // lists the index of the endpoint vertices for each of the 12 edges of the cube
 static const int EdgeConnection[12][2] =
 {
 {0,1}, {1,2}, {2,3}, {3,0},
 {4,5}, {5,6}, {6,7}, {7,4},
 {0,4}, {1,5}, {2,6}, {3,7}
 };*/
//========== END BLOYD

// intersection point for unit cube boolean voxels. Always half way along an edge.
static const float edgePos[12][3] =
{
    {0.5f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f}, {0.5f, 1.0f, 0.0f}, {0.0f, 0.5f, 0.0f},
    {0.5f, 0.0f, 1.0f}, {1.0f, 0.5f, 1.0f}, {0.5f, 1.0f, 1.0f}, {0.0f, 0.5f, 1.0f},
    {0.0f, 0.0f, 0.5f}, {1.0f, 0.0f, 0.5f}, {1.0f, 1.0f, 0.5f}, {0.0f, 1.0f, 0.5f}
};

bool VoxelVolume::flatten(int x, int y, int z, int &intidx, int &bitidx)
{
    if(x < 0 || x >= xdim || y < 0 || y >= ydim || z < 0 || z >= zdim) // out of bounds check
    {
        return false;
    }
    else // in bounds
    {
        intidx = z * (xspan * ydim) + y * (xspan) + (int) ((float) x / (float) intsize);
        bitidx = 31 - (x % intsize); // shifting from/to least significant bit required to select addressed bit
        return true;
    }
}


VoxelVolume::VoxelVolume()
{
    xdim = ydim = zdim = 0;
    xspan = 0;
    voxgrid = NULL;
    setFrame(cgp::Point(0.0f, 0.0f, 0.0f), cgp::Vector(0.0f, 0.0f, 0.0f));
}

VoxelVolume::VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag)
{
    voxgrid = NULL;
    setDim(xsize, ysize, zsize);
    setFrame(corner, diag);
}

VoxelVolume::~VoxelVolume()
{
    clear();
}

void VoxelVolume::clear()
{
    if(voxgrid != NULL)
    {
        delete [] voxgrid;
        voxgrid = NULL;
    }
}

void VoxelVolume::fill(bool setval)
{
    int memsize = xspan * ydim * zdim * sizeof(int);
    unsigned char fillval;
    if(setval) // all bits set
        fillval = (unsigned char) 0xff;
    else // no bits set
        fillval = (unsigned char) 0;
    memset(voxgrid, fillval, memsize);
}

void VoxelVolume::calcCellDiag()
{
    if(xdim > 0 && ydim > 0 && zdim > 0)
        cell = cgp::Vector(diagonal.i / (float) xdim, diagonal.j / (float) ydim, diagonal.k / (float) zdim);
    else
        cell = cgp::Vector(0.0f, 0.0f, 0.0f);
}

void VoxelVolume::getDim(int &dimx, int &dimy, int &dimz)
{
    dimx = xdim; dimy = ydim; dimz = zdim;
}

int VoxelVolume::getXSpan()
{
    return xspan;
}

void VoxelVolume::setDim(int dimx, int dimy, int dimz)
{
    int memsize;

    clear();
    xdim = dimx;
    ydim = dimy;
    zdim = dimz;
    intsize = (sizeof(int) * 8); // because size of an integer is supposedly platform dependent, although typically 32 bits
    // will address individual bits in x dimension, so must be divisible by integer size
    xspan = (int) ceil((float) xdim / (float) intsize);
    cout << "xspan = " << xspan << endl;
    xdim = xspan * intsize;

    memsize = xspan * ydim * zdim;
    voxgrid = new int[memsize];
    fill(false);

    calcCellDiag();
}

void VoxelVolume::getFrame(cgp::Point &corner, cgp::Vector &diag)
{
    corner = origin;
    diag = diagonal;
}

void VoxelVolume::setFrame(cgp::Point corner, cgp::Vector diag)
{
    origin = corner;
    diagonal = diag;
    calcCellDiag();
}

bool VoxelVolume::set(int x, int y, int z, bool setval)
{
    int intidx, bitidx;
    if(flatten(x, y, z, intidx, bitidx))
    {
        if(setval) // set the bit
            voxgrid[intidx] |= (0x1 << bitidx); // "or" left shifted mask with particular bit set
        else // clear the bit
            voxgrid[intidx] &= ~(0x1 << bitidx); // "and" left shifted mask with particular bit unset
        return true;
    }
    else // if out of bounds provide warning and return empty
    {
        cerr << "Error VoxelVolume::set: voxel request (" << x << ", " << y << ", " << z << ") out of bounds" << endl;
        return false;
    }
}

bool VoxelVolume::get(int x, int y, int z)
{
    int intidx, bitidx;
    if(flatten(x, y, z, intidx, bitidx))
    {
        return (bool) ((voxgrid[intidx] >> bitidx) & 0x1); // right shift voxgrid element to select individual bit
    }
    else // if out of bounds provde warning and return empty
    {
        cerr << "Error VoxelVolume::get: voxel request (" << x << ", " << y << ", " << z << ") out of bounds" << endl;
        return false;
    }
}

cgp::Point VoxelVolume::getVoxelPos(int x, int y, int z)
{
    cgp::Point pnt;
    cgp::Vector halfcell;
    float px, py, pz;

    px = (float) x / (float) (xdim-1);
    py = (float) y / (float) (ydim-1);
    pz = (float) z / (float) (zdim-1);

    pnt = cgp::Point(origin.x + px * diagonal.i, origin.y + py * diagonal.j, origin.z + pz * diagonal.k); // convert from voxel space to world coordinates
    return pnt;
}

int VoxelVolume::getMCVertIdx(int x, int y, int z)
{
    bool cube[8];
    int i, idx = 0;

    // flatten 8 voxel vertex values into an array
    for(i = 0; i < 8; i++)
        cube[i] = get(x+cubePos[i][0], y+cubePos[i][1], z+cubePos[i][2]);

    // construct bit index code, 1 for outside, 0 for inside
    for(i = 0; i < 8; i++)
        if(!cube[i])
            idx |= 1<<i;
    return idx;
}

int VoxelVolume::getMCEdgeIdx(int vcode)
{
    return cubeEdgeFlags[vcode];
}

cgp::Point VoxelVolume::getMCEdgeXsect(int ebit)
{
    cgp::Point xsect = cgp::Point(edgePos[ebit][0], edgePos[ebit][1], edgePos[ebit][2]);
    return xsect;
}
