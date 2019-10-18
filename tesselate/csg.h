#ifndef _CSG
#define _CSG
/**
 * @file
 *
 * Constructive Solid Geometry binary tree, with internal nodes representing binary set operations and leaf nodes as shape primitives.
 */

#include <vector>
#include <stdio.h>
#include <iostream>
#include "mesh.h"
#include "voxelrep.h"

/**
 * Different types of binary set operations on shapes
 */
enum class SetOp
{
    UNION,        ///< combine two shapes together
    INTERSECTION, ///< create a shape in the region where the arguments overlap
    DIFFERENCE,   ///< subtract second shape from the first
};

/**
 * Stages of evaluation of the CSG tree
 */
enum class SceneRep
{
    TREE,       ///< native csg tree representation
    VOXELS,     ///< voxelised volumetric representation
    ISOSURFACE, ///< final isosurface mesh representation
};

/// Base class for csg tree nodes
class SceneNode
{
public:
    // must have at least one virtual member to allow dynamic casting
    virtual ~SceneNode(){};
};

/// Inherited class for internal csg tree boolean set operation nodes
class OpNode: public SceneNode
{
public:
    SceneNode * left, * right;
    SetOp op;

    ~OpNode(){}
};

/// Inherited class for leaf csg tree shape nodes
class ShapeNode: public SceneNode
{
public:
    BaseShape * shape;

    ~ShapeNode(){ delete shape; }
};

/**
 * CSG Tree that can be evaluated to produce a volumetric representation.
 */
class Scene
{
private:
    SceneNode * csgroot;                        ///< root node of the csg tree
    GLfloat * col;                              ///< (r,g,b,a) colour
    cgp::Vector voldiag;                        ///< diagonal of scene bounding box in cm
    VoxelVolume vox;                            ///< voxel representation of scene

    float voxsidelen;                           ///< side length of a single voxel
    SceneRep rep;                               ///< which representation is current (tree, voxel, isosurface)
    Mesh voxmesh;                               ///< isosurface of voxel volume
    int percentDone = 0;

    /**
     * Generate triangle mesh geometry for OpenGL rendering of all leaf nodes. 
     * Does not capture set operations at all (except where all set operations are a union)
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool genVizRender(View * view, ShapeDrawData &sdd);

    /**
     * Generate triangle mesh geometry for OpenGL rendering of voxel structure.
     * Approximates voxel grid as a set of spheres. Extremely expensive to render but more accurate in
     * capturing set operations
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool genVoxRender(View * view, ShapeDrawData &sdd);

    /**
     * Apply a boolean set operator given two volumetric operands.
     * @param op            boolean set operation being applied (union, intersection or difference). Applied as leftarg = leftarg op rightarg
     * @param[out] leftarg  first voxel grid argument. Overwritten as the result for space reasons.
     * @param rightarg      second voxel grid argument.
     */
    void voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg);

    /**
     * Convert a CSG tree into a VoxelVolume by evaluating it with a recursive depth-first walk
     * @param root          root node of the CSG tree
     * @param[out] voxels   volumetric representation of the CSG tree
     */
    void voxWalk(SceneNode *root, VoxelVolume *voxels);

public:

    ShapeGeometry geom;         ///< triangle mesh geometry for scene
    ShapeNode * finalMesh = new ShapeNode();
    Mesh * accCube = new Mesh();

    /// Default constructor
    Scene();

    /// Destructor
    ~Scene();

    /// Reset CSG tree
    void clear();

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool bindGeometry(View * view, ShapeDrawData &sdd);

    /**
     * Access voxel volume associated with scene
     */
    VoxelVolume * getVox(){ return &vox; }

    /**
     * Access mesh extracted from voxel volume
     */
    Mesh * getMesh(){ return &voxmesh; }

    /**
     * convert csg tree into a voxel representation
     * @param voxlen    side length of an individual voxel
     */
    void voxelise(float voxlen);

    /**
     * convert voxel representation back into a mesh using marching cubes
     */
    void isoextract();

    /**
     * smooth extracted isosurface to improve on aliasing artefacts that result from marching cubes
     */
    void smooth();

    /**
     * apply free-form deformation to extracted isosurface
     * @param def   free-form deformation lattice
     */
    void deform(ffd * def);

    /**
     * create a sample csg tree to test different shapes and operators
     */
    void sampleScene();

    /**
     * create a sample csg tree to test intersection of sphere and cylinder
     */
    void intersectScene();

    /**
     * create a sample csg tree to load a mesh
     */
    void loadSTLScene(string filename);

    /**
     * create a sample csg tree to hold a sphere mesh
     */
    void sphereScene();

    /**
     * create a sample csg tree to test different shapes and operators. Expensive because it uses mesh point containment with the Bunny.
     */
    void expensiveScene(string filename);

    /**
    * Read grid from file into voxel volume
    * @param filename, len      name of local grid file and length of dimensions
    */
    void readGridVV(std::string filename, int len);

    /**
     * Voxel display scene based on new voxel read input
     */
    void displayVoxelScene(string filename);

    /**
     * Voxel scene based on new voxel read input
     */
    void voxelScene(string filename);

    /**
     * New test voxel scene based on new voxel read input
     */
    void newTestScene(string filename);

    /**
     * Different approach -> voxel scene based on new voxel read input
     */
    void anotherVoxelScene(string filename);

    /**
     * Voxel scene based on new voxel read input
     */
    void voxelMeshScene(string filename);

    /**
     * Shrink displayed model (unofficial teset version)
     */
    void testShrinkScene(bool normalSize = true);

    /**
     * Model A1 to 3D print
     */
    void pieceA1Scene(bool shrunk);

    /**
     * Model A2 to 3D print
     */
    void pieceA2Scene(bool shrunk);
};

#endif
