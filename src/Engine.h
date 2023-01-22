#include "CreateTriangle.h"
#include <fstream>
#include <strstream>
#include <iostream>
#include <math.h>
#include <array>
#include <GLFW/glfw3.h>
struct vec3D
{
    float x;
    float y;
    float z;
    float w = 1;
};

struct triangle
{
    std::array<vec3D, 3> p;
    Color col;
};

struct mesh
{
    std::vector<triangle> tris;

    bool LoadFromObjectFile(std::string fileName)
    {   

        std::ifstream f(fileName);
        if (!f.is_open())
            return false;

        std::vector<vec3D> verts;
        int i = 0;
        std::string line;
        while (std::getline(f, line))
        {
            std::strstream s;
            s << line;
            char junk;
            if (line[0] == 'v') {
                vec3D v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }
            if (line[0] == 'f') {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
            }
            i++;
        }
        return true;
    }
};

struct mat4x4
{   
    // float m[4][4] =  {0};
	std::array<std::array<float, 4>, 4> m= {0};
};



class Engine{
    private:
        
public:
    Triangles TriangleRenderer;
    Engine(GLFWwindow** WindowInput);
    void OnUpdate(float fElapsedTime, std::array<float, 3> offset, float Speed);
    void OnImGUIRender();

private:
    float fYaw;
    float fTheta;
	ScreenCoord coord;
    vec3D vCamera;
    vec3D vLookDir;
    mesh meshCube;
    GLFWwindow** window;
    mat4x4 matProj;
    triangle triProjected;

    vec3D Matrix_MultiplyVector(vec3D &i, mat4x4 &m)
    {
        vec3D v;
        v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
        v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
        v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
        v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
        return v;
    }
    mat4x4 Matrix_Identity()
    {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }
    mat4x4 Matrix_MakeRotationX(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[1][2] = sinf(fAngleRad);
		matrix.m[2][1] = -sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationY(float fAngleRad)
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

	mat4x4 Matrix_MakeRotationZ(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][1] = sinf(fAngleRad);
		matrix.m[1][0] = -sinf(fAngleRad);
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeTranslation(float x, float y, float z)
	{
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

	mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
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

	mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}
    mat4x4 Matrix_PointAt(vec3D &pos, vec3D &target, vec3D &up)
    {
        vec3D newForward = Vector_Sub(target, pos);
        newForward = Vector_Normalise(newForward);

        vec3D a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
        vec3D newUp = Vector_Sub(up, a);
        newUp = Vector_Normalise(newUp);

        vec3D newRight = Vector_CrossProduct(newUp, newForward);
		mat4x4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;

	}

	mat4x4 Matrix_QuickInverse(mat4x4 &m) // Only for Rotation/Translation Matrices
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
    vec3D Vector_IntersectPlane(vec3D &plane_p, vec3D &plane_n, vec3D &lineStart, vec3D &lineEnd)
	{
		plane_n = Vector_Normalise(plane_n);
		float plane_d = -Vector_DotProduct(plane_n, plane_p);
		float ad = Vector_DotProduct(lineStart, plane_n);
		float bd = Vector_DotProduct(lineEnd, plane_n);
		float t = (-plane_d - ad) / (bd - ad);
		vec3D lineStartToEnd = Vector_Sub(lineEnd, lineStart);
		vec3D lineToIntersect = Vector_Mul(lineStartToEnd, t);
		return Vector_Add(lineStart, lineToIntersect);
	}
    int Triangle_ClipAgainstPlane(vec3D plane_p, vec3D plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
	{
		// Make sure plane normal is indeed normal
		plane_n = Vector_Normalise(plane_n);

		// Return signed shortest distance from point to plane, plane normal must be normalised
		auto dist = [&](vec3D &p)
		{
			vec3D n = Vector_Normalise(p);
			return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
		};

		// Create two temporary storage arrays to classify points either side of plane
		// If distance sign is positive, point lies on "inside" of plane
		vec3D* inside_points[3];  int nInsidePointCount = 0;
		vec3D* outside_points[3]; int nOutsidePointCount = 0;

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
			out_tri1.col =  in_tri.col;

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];

			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

			return 1; // Return the newly formed single triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			out_tri1.col =  in_tri.col;

			out_tri2.col =  in_tri.col;

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

			// The second triangle is composed of one of he inside points, a
			// new point determined by the intersection of the other side of the 
			// triangle and the plane, and the newly created point above
			out_tri2.p[0] = *inside_points[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

			return 2; // Return two newly formed triangles which form a quad
		}

		throw std::runtime_error(__func__);
	}
    vec3D Vector_Add(vec3D &v1, vec3D &v2)
    {
        return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    }
    vec3D Vector_Sub(vec3D &v1, vec3D &v2)
    {
        return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    }
    vec3D Vector_Div(vec3D &v1, float k)
    {
        return {v1.x /k, v1.y / k, v1.z /k};
    }
    vec3D Vector_Mul(vec3D &v1, float k)
    {
        return {v1.x * k, v1.y * k, v1.z * k};
    }
    float Vector_DotProduct(vec3D &v1, vec3D &v2)
    {
        return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
    }
    float Vector_Length(vec3D &v)
    {
        return sqrtf(Vector_DotProduct(v, v));
    }
    vec3D Vector_Normalise(vec3D &v)
    {   
        float l = Vector_Length(v);
        return {v.x / l , v.y / l, v.z / l};
    }
    vec3D Vector_CrossProduct(vec3D &v1, vec3D &v2)
    {   
        vec3D v;
        v.x = v1.y * v2.z - v1.z * v2.y;
        v.y = v1.z * v2.x - v1.x * v2.z;
        v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
    }
};