#pragma once
#include <stdexcept>
#include <vector>
#include <cmath>
#include "lodepng.h"

using UC = unsigned char;
using D64 = double;
using U32 = unsigned;

const D64 KEPSILON = 0.000001;

struct Vector3D
{
public:
	D64 X, Y, Z;

	Vector3D();;
	Vector3D(D64 x, D64 y, D64 z);;

	D64 GetLength() const;

	static D64 Dot(const Vector3D& a, const Vector3D& b);;
	static Vector3D Normalize(const Vector3D& vector);;

	friend Vector3D operator*(const Vector3D& vector, const D64& factor);;
};

struct TexCoord
{
public:
	D64 U, V;

	TexCoord();;
	TexCoord(D64 u, D64 v);;

	friend TexCoord operator+(const TexCoord& a, const TexCoord& b);;
};

struct Vertex
{
public:
	Vector3D Origin;
	Vector3D Normal;
	TexCoord UV;

	Vertex(const Vector3D& origin, const Vector3D& normal, const TexCoord& uv);

	D64& X = Origin.X;
	D64& Y = Origin.Y;
	D64& Z = Origin.Z;

	D64& U = UV.U;
	D64& V = UV.V;
};

struct Triangle
{
public:
	const Vertex A, B, C;
	const TexCoord MinUV, MaxUV;

	const std::vector<Vector3D> Origins;
	const std::vector<TexCoord> TexCoordinates;
	const std::vector<Vector3D> Normals;

	constexpr const Vertex& operator[](size_t index) const;

	Triangle(const Vertex& a, const Vertex& b, const Vertex& c);
};

class Lightmapper
{
private:
	std::vector<UC> mPixelBuffer;
	std::vector<Triangle> mTriangles;

public:
	const U32 Width, Height;
	const D64 AmbientFactor;
	const D64 Bias;
	const Vector3D LightDirection;

	Lightmapper(
		const std::vector<Triangle>& triangles,
		U32 width, U32 height,
		D64 ambientFactor,
		D64 bias,
		const Vector3D& lightDirection);

	void SetPixel(U32 x, U32 y, D64 r, D64 g, D64 b, D64 a);
};

