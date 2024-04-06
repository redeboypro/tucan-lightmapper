#include "Lightmapper.h"

Vector3D::Vector3D() : X(0.0), Y(0.0), Z(0.0) {}

Vector3D::Vector3D(D64 x, D64 y, D64 z) : X(x), Y(y), Z(z) {}

D64 Vector3D::GetLength() const
{
	return std::sqrt(X * X + Y * Y + Z * Z);
}

D64 Vector3D::Dot(const Vector3D& a, const Vector3D& b)
{
	return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
}

Vector3D Vector3D::Normalize(const Vector3D& vector)
{
	const double magnitude = vector.GetLength();

	if (magnitude > KEPSILON)
	{
		const D64 multiplier = 1.0 / magnitude;
		return vector * multiplier;
	}

	return Vector3D();
}

Vector3D operator*(const Vector3D& vector, const D64& factor)
{
	return Vector3D(vector.X * factor, vector.Y * factor, vector.Z * factor);
}

TexCoord operator+(const TexCoord& a, const TexCoord& b)
{
	return TexCoord(a.U + b.U, a.V + b.V);
}

TexCoord::TexCoord() : U(0.0), V(0.0) {}

TexCoord::TexCoord(D64 u, D64 v) : U(u), V(v) {}

Lightmapper::Lightmapper(
	const std::vector<Triangle>& triangles,
	U32 width, U32 height,
	D64 ambientFactor,
	D64 bias,
	const Vector3D& lightDirection) :
	mTriangles(triangles),
	mPixelBuffer(width * width * 4, 255),
	Width(width), Height(height),
	AmbientFactor(ambientFactor),
	Bias(bias),
	LightDirection(lightDirection) {}

void Lightmapper::SetPixel(U32 x, U32 y, D64 r, D64 g, D64 b, D64 a)
{
	if (x < Width && y < Height)
	{
		U32 index = (x + y * Width) * 4;

		mPixelBuffer[index] = static_cast<UC>(r * 255);
		mPixelBuffer[index + 1] = static_cast<UC>(g * 255);
		mPixelBuffer[index + 2] = static_cast<UC>(b * 255);
		mPixelBuffer[index + 3] = static_cast<UC>(a * 255);
	}
}

Vertex::Vertex(const Vector3D& origin, const Vector3D& normal, const TexCoord& uv) {}

constexpr const Vertex& Triangle::operator[](size_t index) const
{
	constexpr const Vertex Triangle::*accessors[] = 
	{
		&Triangle::A,
		&Triangle::B,
		&Triangle::C,
	};

	return this->*accessors[index];
}

Triangle::Triangle(const Vertex& a, const Vertex& b, const Vertex& c) : 
	A(a), B(b), C(c),
	Origins({ a.Origin, b.Origin, c.Origin }),
	TexCoordinates({ a.UV, b.UV, c.UV }),
	Normals({ a.Normal, b.Normal, c.Normal }) {}