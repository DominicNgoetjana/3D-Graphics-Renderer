//
// voxels
//

#include "voxels.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>

using namespace std;

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
}

VoxelVolume::VoxelVolume(int xsize, int ysize, int zsize)
{
    int memsize;

    xdim = xsize;
    ydim = ysize;
    zdim = zsize;
    intsize = (sizeof(int) * 8); // because size of an integer is supposedly platform dependent, although typically 32 bits
    // will address individual bits in x dimension, so must be divisible by integer size
    xspan = (int) ceil((float) xdim / (float) intsize);
    xdim = xspan * intsize;

    memsize = xspan * ydim * zdim;
    voxgrid = new int[memsize];
    fill(false);
}

VoxelVolume::~VoxelVolume()
{
    if(voxgrid != NULL)
        delete [] voxgrid;
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

void VoxelVolume::getDim(int &dimx, int &dimy, int &dimz)
{
    dimx = xdim; dimy = ydim; dimz = zdim;
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