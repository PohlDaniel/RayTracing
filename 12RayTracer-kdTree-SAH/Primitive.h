#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Vector.h"
#include "Color.h"
#include "Hit.h"
#include "Ray.h"


class BBox{
public:
	BBox() : m_Pos(Vector3f(0, 0, 0)), m_Size(Vector3f(0, 0, 0)) {};
	BBox(Vector3f& a_Pos, Vector3f& a_Size) : m_Pos(a_Pos), m_Size(a_Size)  {};
	Vector3f& getPos() { return m_Pos; }
	Vector3f& getSize() { return m_Size; }

	inline void extend(Vector3f a)
	{
		m_Pos = Vector3f::Min(a, m_Pos);
		m_Size = Vector3f::Max(a, m_Size);
	}

	inline float getSurfaceArea()
	{
		float xDim = m_Size[0] - m_Pos[0];
		float yDim = m_Size[1] - m_Pos[1];
		float zDim = m_Size[2] - m_Pos[2];

		return 2 * (xDim*yDim + yDim*zDim + zDim*xDim);
	}


	bool intersect(const Ray &ray);

	Vector3f m_Pos, m_Size;
	float m_tmin, m_tmax;
};


/////////////////////////////////////////////////////////////////////////////

class Primitive {

	friend class Scene;
	friend class KDTree;
public:
	Primitive();

	Primitive(const Color &color, const Vector3f &normal);
	~Primitive();

	virtual void hit(const Ray& ray, Hit &hit) = 0;
	virtual void calcBounds() = 0;
	virtual void clip(int axis, float position, BBox& leftBoundingBox, BBox& rightBoundingBox);

	//const BBox& GetBounds();
	BBox& getBounds();

protected:
	bool bounds;
	BBox box;
	Color color;
	Vector3f normal;	
	bool  orientable;
	Matrix4f invT;
};
/////////////////////////////////////////////////////////////////////////////
class OrientablePrimitive : public Primitive {


public:

	OrientablePrimitive();
	OrientablePrimitive(const Color& color, const Vector3f &normal);
	~OrientablePrimitive();

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float);

};

/////////////////////////////////////////////////////////////////////////////

class Triangle :public OrientablePrimitive
{
public:
	
	Triangle();

	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3, Vector3f &normal);
	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3, Vector3f &normal, const Color& color);
	~Triangle();

	
	void hit(const Ray& ray, Hit &hit);
	void calcBounds();
	

private:
	
	Vector3f m_a;
	Vector3f m_b;
	Vector3f m_c;

};

//////////////////////////////////////////////////////////////////////////////////////////////////

class Sphere : public Primitive{
public:
	Sphere();
	Sphere(Vector3f& a_Centre, double a_Radius, Color color);
	~Sphere();

	void hit(const Ray& ray, Hit &hit);
	void calcBounds();

	Vector3f getNormal(Vector3f& a_Pos);
	
private:

	Vector3f m_Centre;						
	double m_SqRadius, m_Radius, m_RRadius;	
				
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Plane : public Primitive{

public:

	Plane();
	Plane(Vector3f normal, float distance, Color color);
	~Plane();

	void hit(const Ray &ray, Hit &hit);
	void calcBounds();

private:

	float distance;
};
//////////////////////////////////////////////////////////////////////////////////////////////////
class Torus : public OrientablePrimitive{

public:

	Torus();
	Torus(float a, float b, Color color);
	~Torus();

	void hit(const Ray &ray, Hit &hit);
	void calcBounds();

private:
	float a;
	float b;
};
//////////////////////////////////////////////////////////////////////////////////////////////////

class KDTree;
class Mesh :public OrientablePrimitive {


public:
	Mesh();
	~Mesh();

	std::vector<Triangle*>	triangles;
	void hit(const Ray& a_Ray, Hit &hit);
	void calcBounds();
	

	bool loadObject(const char* filename);
	bool loadObject(const char* filename, float scale);

	KDTree* m_KDTree;

private:

	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;
	
};



#endif