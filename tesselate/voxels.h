// VoxelVolume class for storing a 3d cuboid of binary voxel data
// (c) James Gain, 2014

#ifndef _VOXELVOLUME
#define _VOXELVOLUME

#include <vector>
#include <stdio.h>
#include <iostream>
#include "vecpnt.h"

class VoxelVolume
{
private:
    int * voxgrid;  ///< flattened voxel volume, bit packed to save memory
    int xdim;       ///< number of voxels in x dimension
    int ydim;       ///< number of voxels in y dimension
    int zdim;       ///< number of voxels in z dimension
    int xspan;      ///< number of integers used to represent xdim
    int intsize;    ///< size of an integer in bits

    /**
     * Convert from 3D position to voxgrid index, including the bit position
     * @param x, y, z   3D location, zero indexed
     * @param intidx   the word in the voxel grid within which the occupance value is located
     * @param bitidx   the bit position within the located word
     * @retval @c true  if the 3D position is within the bounds of the grid,
     * @retval @c false otherwise.
     */
    bool flatten(int x, int y, int z, int &intidx, int &bitidx);

public:

    VoxelVolume();

    /**
     * Create voxel volume with specified dimensions. x dimension is padded to be divisible by 32
     * @param xsize, ysize, zsize      number of voxels in x, y, z dimensions
     */
    VoxelVolume(int xsize, int ysize, int zsize);

    ~VoxelVolume();

    /**
     * Delete voxel volume grid
     */
    void clear();

    /**
     * Set all voxel elements in volume to empty or occupied
     * @param setval    new value for all voxel elements, either empty (false) or occupied (true)
     */
    void fill(bool setval);

    /**
     * Obtain the dimensions of the voxel volume
     * @param dimx, dimy, dimz     number of voxels in x, y, z dimensions
     */
    void getDim(int &dimx, int &dimy, int &dimz);

    /**
     * Set the dimensions of the voxel volume and allocate memory accordingly
     * @param dimx, dimy, dimz     number of voxels in x, y, z dimensions
     */
    void setDim(int &dimx, int &dimy, int &dimz);

    /**
     * Set a single voxel element to either empty or occupied
     * @param x, y, z   3D location, zero indexed
     * @param setval    new voxel value, either empty (false) or occupied (true)
     * @retval @c true if the voxel is within volume bounds,
     * @retval @c false otherwise.
     */
    bool set(int x, int y, int z, bool setval);

    /**
     * Get the status of a single voxel element at the specified position
     * @param x, y, z   3D location, zero indexed
     * @retval @c true if the voxel is occupied,
     * @retval @c false if the voxel is empty.
     */
    bool get(int x, int y, int z);

    /**
     * Return the marching cubes vertex bit code for a voxel cell
     * @param x, y, z   3d index for the lower, front, left corner of the cell
     * @retval  bit code index for MC table
     */
    int getMCVertIdx(int x, int y, int z);

    /**
     * Return the marching cubes edge intersection bit code corresponding to a vertex bit code
     * @param vcode     vertex pattern bit code
     * @retval  edge bit code index for MC table
     */
    int getMCEdgeIdx(int vcode);

    /**
     * Return the marching cubes edge intersection corresponding to and edge bit position
     * @param ebit  bit position corresponding to a cube edge
     * @retval      position in unit cube of intersection
     */
    vpPoint getMCEdgeXsect(int ebit);
};

#endif
