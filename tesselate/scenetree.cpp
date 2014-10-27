//
// SceneTree
//

#include "scenetree.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>

using namespace std;

GLfloat defaultCol[] = {0.243f, 0.176f, 0.75f, 1.0f};

ShapeNode::ShapeNode()
{
    ntype = NodeType::SHAPENODE;
    stype = ShapeType::EMPTY; // null shape
    xfm = glm::mat4(1.0f); // identity matrix
    col = defaultCol;
}

ShapeNode::ShapeNode(ShapeType type, glm::mat4 mat, GLfloat * colour)
{
    stype = type;
    ntype = NodeType::SHAPENODE;
    xfm = mat;
    col = colour;
}

void ShapeNode::genGeometry()
{
    geom.clear();
    geom.setColour(col);

    switch(stype)
    {
        case ShapeType::EMPTY: // no geometry
            break;
        case ShapeType::SPHERE:
            geom.genSphere(1.0f, 40, 40, xfm);
            break;
        case ShapeType::CYLINDER:
            geom.genCylinder(1.0f, 1.0f, 40, 40, xfm);
            break;
        default:
            break;
    }
}