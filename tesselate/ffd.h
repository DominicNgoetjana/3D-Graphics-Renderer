#ifndef _FFD
#define _FFD
/**
 * @file
 *
 * Free-form Deformation to warp vertices of a mesh. Uses a Bezier basis.
 */

#include <vector>
#include <stdio.h>
#include <iostream>
#include "renderer.h"

/**
 * Free-Form Deformation of geometric models. Supports Bezier bases with n=1,2,3
 * that can be set seperately for each dimension.
 */
class ffd
{
private:
    cgp::Point origin;      ///< bottom left front corner of lattice
    cgp::Vector diagonal;   ///< diagonal extent of lattice
    cgp::Point *** cp;      ///< dimx * dimy * dimz lattice of control points
    bool *** highlight;     ///< highlighting of control points to show selection
    int dimx;               ///< number of control points in x dimension
    int dimy;               ///< number of control points in y dimension
    int dimz;               ///< number of control points in z dimension

    /// Memory allocation of 3D array
    void alloc();

    /// Memory deallocation of 3D array
    void dealloc();

    /**
     * Check control point access to see if it is out of bounds
     * @param i, j, k   control point index [0..dimx-1,0..dimy-1,0..dimz-1] in lattice
     * @retval true if the index lies within the bounds of the lattice,
     * @retval false otherwise
     */
    bool inCPBounds(int i, int j, int k);


    /**
     * Evaluate a Bernstein polynomial for a given parameter value
     * @param t     parameter value, in [0,1]
     * @param i     control point index, in [0,n]
     * @param n     polynomial degree, in [1,3]
     * @return evaluated basis function, or 0 if any parameters are set incorrectly
     */
    float basis(float t, int i, int n);

public:

    ShapeGeometry geom;         ///< renderable version of non-active lattice
    ShapeGeometry activegeom;   ///< renderable version of active lattice (selected control points)

    /// Default constructor
    ffd();

    /**
     * Create FFD lattice with specified dimensions
     * @param xnum, ynum, znum  number of control point in x, y, z dimensions (2-4)
     * @param corner    origin position of the volume
     * @param diag      diagonal extent of the volume
     */
    ffd(int xnum, int ynum, int znum, cgp::Point corner, cgp::Vector diag);

    /// Destructor
    ~ffd(){ dealloc(); }

    /**
     * Reset all control points to their initial underformed positions. These are chosen to ensure that, by
     * linear precision, no deformation happens even if deform is called. In other words, the identity deformation.
     */
    void reset();

    /**
     * Obtain the dimensions of the ffd lattice
     * @param[out] numx, numy, numz     number of cp in x, y, z dimensions
     */
    void getDim(int &numx, int &numy, int &numz);

    /**
     * Set the dimensions of the ffd lattice and allocate memory accordingly
     * @param numx, numy, numz     number of cp in x, y, z dimensions
     */
    void setDim(int numx, int numy, int numz);

    /**
     * Getter for the placement and dimensions of the lattice in 3d space
     * @param[out] corner    bottom, front, left corner of the lattice
     * @param[out] diag      diagonal vector across the lattice
     */
    void getFrame(cgp::Point &corner, cgp::Vector &diag);

    /**
     * Setter for the placement and dimensions of the lattice in 3d space
     * @param corner    bottom, front, left corner of the lattice
     * @param diag      diagonal vector across the lattice
     */
    void setFrame(cgp::Point corner, cgp::Vector diag);

    /**
     * Highlight a selected control point in red
     * @param i, j, k   control point index [0..dimx-1,0..dimy-1,0..dimz-1] in lattice
     */
    void activateCP(int i, int j, int k);

    /**
     * Remove highlighting on a selected control point
     * @param i, j, k   control point index [0..dimx-1,0..dimy-1,0..dimz-1] in lattice
     */
    void deactivateCP(int i, int j, int k);

    /**
     * Set all control point highlighting off, so that control points have default colour
     */
    void deactivateAllCP();

    /**
     * Generate triangle mesh geometry for OpenGL rendering of control points.
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @param active    if true then draw highlighted cp, otherwise draw non-active cp
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool bindGeometry(View * view, ShapeDrawData &sdd, bool active);

    /**
     * Getter for control point positions
     * @param i, j, k   control point index [0..dimx-1,0..dimy-1,0..dimz-1] in lattice
     * @returns         control point position in 3d space (or origin if access is out of bounds)
     */
    cgp::Point getCP(int i, int j, int k);

    /**
     * Setter for control point positions
     * @param i, j, k   control point index [0..dimx-1,0..dimy-1,0..dimz-1] in lattice
     * @param pnt        control point position in 3d space
     */
    void setCP(int i, int j, int k, cgp::Point pnt);

    /**
     * Apply free-form deformation to a point by embedding it in an undistorted lattice and then applying the deformation indicated by new control point positions
     * @param[out] pnt  Point undergoing deformation
     */
    void deform(cgp::Point & pnt);
};

#endif