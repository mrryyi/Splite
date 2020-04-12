#pragma once

#include <cmath>
#include <fstream>
#include <strstream>

namespace graphics {

struct vec3d {
    float32 x = 0.0;
    float32 y = 0.0;
    float32 z = 0.0;
    float32 w = 1.0;
};

struct color3f {
    GLfloat r = { 1.0 }, g = { 1.0 }, b = { 1.0 };

    void operator=(const color3f& param) {
        r = param.r;
        g = param.g;
        b = param.b;
    }
};

struct triangle {
    vec3d p[3];

    color3f color;


};

struct mesh {
    std::vector<triangle> tris;

    bool8 load_from_object_file( std::string filename ) {
        
        std::ifstream f(filename);
        if (!f.is_open())
            return false;
        
        std::vector<vec3d> verts;
        char junk;

        while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

        return true;
    }
};

struct mat4x4 {
    float32 m[4][4] = { 0 };
};

vec3d vector_add( vec3d &v1, vec3d &v2) {
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3d vector_sub( vec3d &v1, vec3d &v2) {
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

vec3d vector_mul( vec3d &v, float32 k) {
    return { v.x * k, v.y * k, v.z * k };
}

vec3d vector_div( vec3d &v, float32 k) {
    return { v.x / k, v.y / k, v.z / k };
}

float32 vector_dot_product( vec3d &v1, vec3d &v2 ) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float32 vector_length( vec3d &v ) {
    return sqrtf( vector_dot_product( v, v ));
}

vec3d vector_normalize( vec3d &v ) {
    float32 l = vector_length( v );
    return { v.x / l, v.y / l, v.z / l };
}

vec3d vector_cross_product( vec3d &v1, vec3d &v2 ) {
    vec3d v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

vec3d vector_intersect_plane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd)
{
    plane_n = vector_normalize(plane_n);
    float plane_d = -vector_dot_product(plane_n, plane_p);
    float ad = vector_dot_product(lineStart, plane_n);
    float bd = vector_dot_product(lineEnd, plane_n);
    float t = (-plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd = vector_sub(lineEnd, lineStart);
    vec3d lineToIntersect = vector_mul(lineStartToEnd, t);
    return vector_add(lineStart, lineToIntersect);
}

vec3d matrix_multiply_vector( mat4x4 &m, vec3d &i) {
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    return v;
}

mat4x4 matrix_make_identity() {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 matrix_make_rotation_x( float32 fAngleRad ) {
    mat4x4 matrix;
    matrix.m[0][0] = 1;
    matrix.m[1][1] = cosf(fAngleRad * 0.5f);
    matrix.m[1][2] = sinf(fAngleRad * 0.5f);
    matrix.m[2][1] = -sinf(fAngleRad * 0.5f);
    matrix.m[2][2] = cosf(fAngleRad * 0.5f);
    matrix.m[3][3] = 1;
    return matrix;
}

mat4x4 matrix_make_rotation_y( float32 fAngleRad )
{
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][2] = sinf(fAngleRad);
    matrix.m[2][0] = -sinf(fAngleRad);
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 matrix_make_rotation_z( float32 fAngleRad ) {
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][1] = sinf(fAngleRad);
    matrix.m[1][0] = -sinf(fAngleRad);
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[2][2] = 1;
    matrix.m[3][3] = 1;
    return matrix;
}

mat4x4 matrix_make_translation( float32 x, float32 y, float32 z) {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

mat4x4 matrix_make_projection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    mat4x4 matrix;
    matrix.m[0][0] = fAspectRatio * fFovRad;
    matrix.m[1][1] = fFovRad;
    matrix.m[2][2] = fFar / (fFar - fNear);
    matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    return matrix;
}

mat4x4 matrix_multiply_matrix(mat4x4 &m1, mat4x4 &m2)
{
    mat4x4 matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
    return matrix;
}

mat4x4 matrix_point_at(vec3d &pos, vec3d &target, vec3d &up) {
    // Calculate new forward direction
    vec3d newForward = vector_sub(target, pos);
    newForward = vector_normalize(newForward);

    // Calculate new Up direction
    vec3d a = vector_mul(newForward, vector_dot_product(up, newForward));
    vec3d newUp = vector_sub(up, a);
    newUp = vector_normalize(newUp);

    // New Right direction is easy, its just cross product
    vec3d newRight = vector_cross_product(newUp, newForward);

    // Construct Dimensioning and Translation Matrix	
    mat4x4 matrix;
    matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
    return matrix;
}

mat4x4 matrix_quick_inverse(mat4x4 &m) // Only for Rotation/Translation Matrices
{
    mat4x4 matrix;
    matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
    matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
    matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
    matrix.m[3][3] = 1.0f;
    return matrix;
}




int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
{
    // Make sure plane normal is indeed normal
    plane_n = vector_normalize(plane_n);

    // Return signed shortest distance from point to plane, plane normal must be normalised
    auto dist = [&](vec3d &p)
    {
        vec3d n = vector_normalize(p);
        return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - vector_dot_product(plane_n, plane_p));
    };

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    vec3d* inside_points[3];  int nInsidePointCount = 0;
    vec3d* outside_points[3]; int nOutsidePointCount = 0;

    // Get signed distance of each point in triangle to plane
    float d0 = dist(in_tri.p[0]);
    float d1 = dist(in_tri.p[1]);
    float d2 = dist(in_tri.p[2]);

    if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
    else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
    if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
    else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
    if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
    else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

    // Now classify triangle points, and break the input triangle into 
    // smaller output triangles if required. There are four possible
    // outcomes...

    if (nInsidePointCount == 0)
    {
        // All points lie on the outside of plane, so clip whole triangle
        // It ceases to exist

        return 0; // No returned triangles are valid
    }

    if (nInsidePointCount == 3)
    {
        // All points lie on the inside of plane, so do nothing
        // and allow the triangle to simply pass through
        out_tri1 = in_tri;

        return 1; // Just the one returned original triangle is valid
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        // Triangle should be clipped. As two points lie outside
        // the plane, the triangle simply becomes a smaller triangle

        // Copy appearance info to new triangle
        out_tri1.color =  in_tri.color;

        // The inside point is valid, so keep that...
        out_tri1.p[0] = *inside_points[0];

        // but the two new points are at the locations where the 
        // original sides of the triangle (lines) intersect with the plane
        out_tri1.p[1] = vector_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
        out_tri1.p[2] = vector_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

        return 1; // Return the newly formed single triangle
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        // Triangle should be clipped. As two points lie inside the plane,
        // the clipped triangle becomes a "quad". Fortunately, we can
        // represent a quad with two new triangles

        // Copy appearance info to new triangles
        out_tri1.color =  in_tri.color;

        out_tri2.color =  in_tri.color;

        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = *inside_points[1];
        out_tri1.p[2] = vector_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the 
        // triangle and the plane, and the newly created point above
        out_tri2.p[0] = *inside_points[1];
        out_tri2.p[1] = out_tri1.p[2];
        out_tri2.p[2] = vector_intersect_plane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

        return 2; // Return two newly formed triangles which form a quad
    }
}

void get_color_by_lum(float32 lum, color3f& out_color ) {
    out_color.r = lum;
    out_color.g = lum;
    out_color.b = lum;
}


};