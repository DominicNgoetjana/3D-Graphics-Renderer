// file: vecpnt.cpp
// author: James Gain
// project: Interactive Sculpting (1997+)
// notes: Basic vector and point arithmetic library. Inlined for efficiency.
// changes: included helpful geometry routines (2006)

#include "vecpnt.h"
#include <stdio.h>
#include <iostream>

using namespace std;

void Plane::formPlane(vpPoint pnt, Vector norm)
{
    n = norm;
    n.normalize();
    d = -1.0f * (n.i * pnt.x + n.j * pnt.y + n.k * pnt.z);
}

bool Plane::formPlane(vpPoint * tri)
{
    Vector v1, v2, zero;

    v1.diff(tri[0], tri[1]);
    v2.diff(tri[0], tri[2]);
    v1.normalize();
    v2.normalize();
    n.cross(v1, v2);
    zero = Vector(0.0f, 0.0f, 0.0f);
    if(n == zero)
        return false;
    n.normalize();
    d = -1.0f * (n.i * tri[0].x + n.j * tri[0].y + n.k * tri[0].z);
    return true;
}

bool Plane::rayPlaneIntersect(vpPoint start, Vector dirn, float & tval)
{
    Vector svec;
    double num, den;

    // this approach can be numerically unstable for oblique intersections
    svec.pntconvert(start);

    num = (double) (d + svec.dot(n));
    den = (double) dirn.dot(n);
    if(den == 0.0f) // dirn parallel to plane
        if(num == 0.0f) // ray lies in plane
            tval = 0.0f;
        else
            return false;
    else
        tval = (float) (-1.0f * num/den);
    return true;
}

bool Plane::rayPlaneIntersect(vpPoint start, Vector dirn, vpPoint & intersect)
{
    Vector svec, dvec;
    double num, den;
    float tval;

    svec.pntconvert(start);

    num = (double) (d + svec.dot(n));
    den = (double) dirn.dot(n);
    if(den == 0.0f) // dirn parallel to plane
        if(num == 0.0f) // ray lies in plane
            tval = 0.0f;
        else
            return false;
    else
        tval = (float) (-1.0f * num/den);
    dvec = dirn;
    dvec.mult(tval);
    dvec.pntplusvec(start, &intersect);
    return true;
}

bool Plane::side(vpPoint pnt)
{
    return ((pnt.x * n.i + pnt.y * n.j + pnt.z * n.k + d) >= 0.0f);
}

float Plane::dist(vpPoint pnt)
{
    return fabs(pnt.x * n.i + pnt.y * n.j + pnt.z * n.k + d);
}

float Plane::height(vpPoint pnt)
{
    // B.y = -A.x - C.z - D
    return (n.i * pnt.x + n.k * pnt.z + d) / (n.j * -1.0f);
}

void Plane::projectPnt(vpPoint pnt, vpPoint * proj)
{
    Vector svec;
    float tval;

    svec.pntconvert(pnt);
    tval = -1.0f * (d + svec.dot(n));
    svec = n;
    svec.mult(tval);
    svec.pntplusvec(pnt, proj);
}

void rayPointDist(vpPoint start, Vector dirn, vpPoint query, float &tval, float &dist)
{
    float den;
    vpPoint closest;
    Vector closevec;

    den = dirn.sqrdlength();
    if(den == 0.0f) // not a valid line segment
        dist = -1.0f;
    else
    {
        // get parameter value of closest point
        tval = dirn.i * (query.x - start.x) + dirn.j * (query.y - start.y) + dirn.k * (query.z - start.z);
        tval /= den;

        // find closest point on line
        closevec = dirn;
        closevec.mult(tval);
        closevec.pntplusvec(start, &closest);
        closevec.diff(query, closest);
        dist = closevec.length();
    }
}

void clamp(float & t)
{
    if(t > 1.0f+minuszero)
        t = 1.0f+minuszero;
    if(t < 0.0f)
        t = 0.0f;
}
