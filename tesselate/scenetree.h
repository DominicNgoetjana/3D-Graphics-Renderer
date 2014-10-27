// SceneTree class for building up complex constructive solid geometry
// (c) James Gain, 2014

#ifndef _SCENEGRAPH
#define _SCENEGRAPH

#include <vector>
#include <stdio.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "renderer.h"

enum class NodeType
{
    OPNODE,         ///< operator internal node
    SHAPENODE       ///< shape leaf node
};

/**
 * Generic node in the SceneTree. Ancestor to OpNode and ShapeNode.
 */
class SceneNode
{
protected:
    glm::mat4 xfm;      ///< transformation matrix applied to this node
    NodeType ntype;     ///< node type - either operator or shape
    // could be done with dynamic casts but this is less messy

public:

    virtual ~SceneNode(){}
};

enum class OpType
{
    UNION,          ///< union set operation to combine children object
    DIFFERENCE,     ///< difference set operation to remove the volume of right child from the left child
    INTERSECTION    ///< intersection set operation where only common volume of the two children is retained
};

/**
 * CSG operator in the SceneTree. An internal node that represents union, difference or intersection.
 */
class OpNode: public SceneNode
{
private:
    OpType otype;               ///< CSG operator encoded by this node
    SceneNode * left, * right;  ///< left and right children of the operator, order is important

};

enum class ShapeType
{
    EMPTY,      ///< null object
    SPHERE,     ///< sphere of unit radius centered at the origin
    CYLINDER    ///< cylinder of unit radius and length aligned along the positive x-axis, with base at the origin
};

/**
 * Implicit shape in the SceneTree. An leaf node that represents either a sphere or cylinder.
 */
class ShapeNode: public SceneNode
{
private:
    ShapeType stype;    ///< shape encoded by this node
    ShapeGeometry geom; ///< triangle mesh geometry for shape
    GLfloat * col;      ///< (r,g,b,a) colour

public:

    ShapeNode();

    /**
     * Constructor
     * @param type   geometric shape represented by this node
     * @param mat    transformation matrix applied to the shape
     * @param colour shape colour
     */
    ShapeNode(ShapeType type, glm::mat4 mat, GLfloat * colour);

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     */
    void genGeometry();

    /// Getter for shape geometry
    ShapeGeometry * getGeometry(){ return &geom; }
};

class SceneTree
{
private:
    ShapeNode * root;

public:



};

#endif
