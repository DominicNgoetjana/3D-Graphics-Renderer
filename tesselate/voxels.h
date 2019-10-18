#ifndef _VOXELVOLUME
#define _VOXELVOLUME
/**
 * @file
 *
 * VoxelVolume class for storing a 3d cuboid of binary voxel data
 */


#include <vector>
#include <stdio.h>
#include <iostream>
#include "vecpnt.h"

/**
 * A cuboid volume regularly subdivided into uniformly sized cubes (voxels). Bit packing is used to compress storage.
 */
class VoxelVolume
{
private:
    int * voxgrid;  ///< flattened voxel volume, bit packed to save memory
    int xdim;       ///< number of voxels in x dimension
    int ydim;       ///< number of voxels in y dimension
    int zdim;       ///< number of voxels in z dimension
    int xspan;      ///< number of integers used to represent xdim
    int intsize;    ///< size of an integer in bits

    cgp::Point origin;     ///< corner point in world space
    cgp::Vector diagonal;  ///< diagonal extent of the volume in world space
    cgp::Vector cell;      ///< diagonal extent of a single voxel cell

    /**
     * Convert from 3D position to voxgrid index, including the bit position
     * @param x, y, z   3D location, zero indexed
     * @param intidx   the word in the voxel grid within which the value is located
     * @param bitidx   the bit position within the located word
     * @retval @c true  if the 3D position is within the bounds of the grid,
     * @retval @c false otherwise.
     */
    bool flatten(int x, int y, int z, int &intidx, int &bitidx);

    /// Calculate the diagonal extent of a single cell and store internally
    void calcCellDiag();

public:

    /// Default constructor
    VoxelVolume();

    /**
     * Create voxel volume with specified dimensions. x dimension is padded to be divisible by 32
     * @param xsize, ysize, zsize      number of voxels in x, y, z dimensions
     * @param corner  origin position of the volume
     * @param diag     diagonal extent of the volume
     */
    VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag);

    /// Destructor
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
    * Get the span of a voxel in the x direction
    * @returns voxel x span
    */
    int getXSpan();

    /**
     * Set the dimensions of the voxel volume and allocate memory accordingly
     * @param dimx, dimy, dimz     number of voxels in x, y, z dimensions
     */
    void setDim(int dimx, int dimy, int dimz);

    /**
     * Getter for the placement and dimensions of the volume in 3d space
     * @param corner    bottom, front, left corner of the volume
     * @param diag      diagonal vector across the volume
     */
    void getFrame(cgp::Point &corner, cgp::Vector &diag);

    /**
     * Setter for the placement and dimensions of the volume in 3d space
     * @param corner    bottom, front, left corner of the volume
     * @param diag      diagonal vector across the volume
     */
    void setFrame(cgp::Point corner, cgp::Vector diag);

    /**
     * Set a single voxel element to either empty or occupied
     * @param x, y, z   3D location, zero indexed
     * @param setval    new voxel value, either empty (false) or occupied (true)
     * @retval true if the voxel is within volume bounds,
     * @retval false otherwise.
     */
    bool set(int x, int y, int z, bool setval);

    /**
     * Get the status of a single voxel element at the specified position
     * @param x, y, z   3D location, zero indexed
     * @retval true if the voxel is occupied,
     * @retval false if the voxel is empty.
     */
    bool get(int x, int y, int z);

    /**
     * Find the world-space position of the centre of a voxel
     * @param x, y, z   3D location, zero indexed
     * @returns voxel centre point 
     */
    cgp::Point getVoxelPos(int x, int y, int z);

    /**
     * Return the marching cubes vertex bit code for a voxel cell
     * (Required to shoehorn Bloyd's code into current framework - see http://paulbourke.net/geometry/polygonise/marchingsource.cpp)
     * @param x, y, z   3d index for the lower, front, left corner of the cell
     * @retval  bit code index for MC table
     */
    int getMCVertIdx(int x, int y, int z);

    /**
     * Return the marching cubes edge intersection bit code corresponding to a vertex bit code
     * (Required to shoehorn Bloyd's code into current framework - see http://paulbourke.net/geometry/polygonise/marchingsource.cpp)
     * @param vcode     vertex pattern bit code
     * @retval  edge bit code index for MC table
     */
    int getMCEdgeIdx(int vcode);

    /**
     * Return the marching cubes edge intersection corresponding to an edge bit position
     * (Required to shoehorn Bloyd's code into current framework - see http://paulbourke.net/geometry/polygonise/marchingsource.cpp)
     * @param ebit  bit position corresponding to a cube edge
     * @retval      position in unit cube of the intersection point
     */
    cgp::Point getMCEdgeXsect(int ebit);
};

#endif
