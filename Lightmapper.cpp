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

Vector3D& Vector3D::operator=(const Vector3D& value)
{
	X = value.X;
	Y = value.Y;
	Z = value.Z;
	return *this;
}

constexpr const D64& Vector3D::operator[](size_t index) const
{
	constexpr const D64 Vector3D::* accessors[] =
	{
		&Vector3D::X,
		&Vector3D::Y,
		&Vector3D::Z,
	};

	return this->*accessors[index];
}

Vector3D operator-(const Vector3D& value)
{
	return Vector3D(-value.X, -value.Y, -value.Z);
}

Vector3D operator+(const Vector3D& a, const Vector3D& b)
{
	return Vector3D(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
}

Vector3D operator-(const Vector3D& a, const Vector3D& b)
{
	return Vector3D(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
}

Vector3D operator*(const Vector3D& vector, const D64& factor)
{
	return Vector3D(vector.X * factor, vector.Y * factor, vector.Z * factor);
}

TexCoord operator+(const TexCoord& a, const TexCoord& b)
{
	return TexCoord(a.U + b.U, a.V + b.V);
}

TexCoord operator*(const TexCoord& vector, const D64& factor)
{
	return TexCoord(vector.U * factor, vector.V * factor);
}

TexCoord::TexCoord() : U(0.0), V(0.0) {}

TexCoord::TexCoord(D64 u, D64 v) : U(u), V(v) {}

bool TexCoord::operator<(const TexCoord& value) const
{
	return (U != value.U) ? (U < value.U) : (V < value.V);
}

bool TexCoord::operator==(const TexCoord& value) const
{
	return U == value.U && V == value.V;
}

TexCoord& TexCoord::operator=(const TexCoord& value)
{
	U = value.U;
	V = value.V;
	return *this;
}

TexCoord& TexCoord::operator+=(const TexCoord& value)
{
	U += value.U;
	V += value.V;
	return *this;
}

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
		U32 index = (x + (Height - y) * Width) * 4;

		mPixelBuffer[index] = static_cast<UC>(r * 255);
		mPixelBuffer[index + 1] = static_cast<UC>(g * 255);
		mPixelBuffer[index + 2] = static_cast<UC>(b * 255);
		mPixelBuffer[index + 3] = static_cast<UC>(a * 255);
	}
}

void Lightmapper::CalculateDiffuse()
{
	for (size_t i = 0; i < mTriangles.size(); i++)
		for (U32 y = 0; y < Height; y++)
			for (U32 x = 0; x < Width; x++)
			{
				TexCoord pixelUV = TexCoord(static_cast<D64>(x) / Width, static_cast<D64>(y) / Height);
				Triangle& triangle = mTriangles[i];

				if (PointInsideTexCoordinates(triangle.TexCoordinates, pixelUV))
				{
					D64 diffuse = std::clamp(std::max(Vector3D::Dot(triangle.A.Normal, -LightDirection), 0.0) + AmbientFactor, 0.0, 1.0);
					SetPixel(x, y, diffuse, diffuse, diffuse, 1.0f);
				}
			}
}

void Lightmapper::CastShadows()
{
	const size_t count = mTriangles.size();
	std::vector<TexCoord> projectedUV;

	for (size_t a = 0; a < count; a++)
		for (size_t b = 0; b < count; b++)
		{
			if (a == b)
				continue;

			Triangle& triangleA = mTriangles[a];
			Triangle& triangleB = mTriangles[b];

			if (TryGetProjectedUV(triangleA, triangleB, projectedUV))
				ShadeArea(projectedUV, triangleB);
		}
}

void Lightmapper::Encode(const std::string& fileName)
{
	lodepng::encode(fileName, mPixelBuffer, Width, Height);
}

Vertex::Vertex(const Vector3D& origin, const Vector3D& normal, const TexCoord& uv) : Origin(origin), Normal(normal), UV(uv) {}

D64 Triangle::Min(D64 a, D64 b, D64 c)
{
	return std::min(std::min(a, b), c);
}

D64 Triangle::Max(D64 a, D64 b, D64 c)
{
	return std::max(std::max(a, b), c);
}

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
	Normals({ a.Normal, b.Normal, c.Normal }),
	MinUV(Min(a.U, b.U, c.U), Min(a.V, b.V, c.V)),
	MaxUV(Max(a.U, b.U, c.U), Max(a.V, b.V, c.V)) {}

inline U32 Lightmapper::U2X(D64 u)
{
	return std::clamp(static_cast<U32>(u * Width), 0U, Width);
}

inline U32 Lightmapper::V2Y(D64 v)
{
	return std::clamp(static_cast<U32>(v * Height), 0U, Height);
}

void Lightmapper::ShadeArea(const std::vector<TexCoord>& projectedUV, const Triangle& triangle)
{
	U32 minX = U2X(triangle.MinUV.U);
	U32 minY = V2Y(triangle.MinUV.V);

	U32 maxX = U2X(triangle.MaxUV.U);
	U32 maxY = V2Y(triangle.MaxUV.V);

	for (U32 y = minY; y <= maxY; y++)
		for (U32 x = minX; x <= maxX; x++)
		{
			TexCoord pixelUV = TexCoord(static_cast<D64>(x) / Width, static_cast<D64>(y) / Height);

			if (PointInsideTexCoordinates(projectedUV, pixelUV) && PointInsideTexCoordinates(triangle.TexCoordinates, pixelUV))
				SetPixel(x, y, AmbientFactor, AmbientFactor, AmbientFactor, 1.0);
		}
}

bool Lightmapper::TryGetProjectedUV(const Triangle& triangleA, const Triangle& triangleB, std::vector<TexCoord>& projectedUV)
{
	Vertex planeBasicVertex = triangleB.A;
	Vector3D planeCenter = planeBasicVertex.Origin;
	Vector3D planeNormal = planeBasicVertex.Normal;

	D64 denominator0 = Vector3D::Dot(planeNormal, LightDirection);
	
	projectedUV.clear();

	if (std::abs(denominator0) <= KEPSILON)
		return false;

	size_t i = 0;
	for (size_t a = 0; a < 3; a++)
	{
		Vector3D aOrigin = triangleA[a].Origin;
		D64 t0 = Vector3D::Dot(planeCenter - aOrigin, planeNormal) / denominator0;

		Vector3D intersectionPoint;
		Vector3D barycentricCoordinates;

		if (t0 <= 0)
		{
			i++;

			if (i == 3)
				return false;

			for (size_t b = 0; b < 3; b++) 
			{
				if (a == b)
					continue;

				Vector3D bOrigin = triangleA[b].Origin;
				Vector3D bDirection = Vector3D::Normalize(aOrigin - bOrigin);

				D64 denominator1 = Vector3D::Dot(planeNormal, bDirection);

				if (std::abs(denominator1) <= KEPSILON)
					continue;

				D64 t1 = Vector3D::Dot(planeCenter - bOrigin, planeNormal) / denominator1;

				if (t1 < 0)
					continue;

				intersectionPoint = bOrigin + bDirection * t1;
				barycentricCoordinates = CalculateBarycentric(intersectionPoint, triangleB);
				projectedUV.push_back(CalculateUVFromBarycentric(barycentricCoordinates, triangleB));
			}

			continue;
		}

		intersectionPoint = aOrigin + LightDirection * t0;
		barycentricCoordinates = CalculateBarycentric(intersectionPoint, triangleB);
		projectedUV.push_back(CalculateUVFromBarycentric(barycentricCoordinates, triangleB));
	}

	if (i > 0)
		SortTexCoordinates(projectedUV);

	return projectedUV.size() >= 3;
}

bool Lightmapper::PointInsideTexCoordinates(const std::vector<TexCoord>& projectedUV, const TexCoord& point)
{
	bool hasPositive = false, hasNegative = false;
	const size_t count = projectedUV.size();

	for (size_t i = 0; i < count; i++)
	{
		TexCoord a = projectedUV[i];
		TexCoord b = projectedUV[(i + 1) % count];
		D64 direction = CalculateDirection(point, a, b);

		if (direction < -Bias)
			hasNegative = true;

		if (direction > Bias)
			hasPositive = true;

		if (hasPositive && hasNegative)
			return false;
	}

	return true;
}

void Lightmapper::SortTexCoordinates(std::vector<TexCoord>& projectedUV)
{
	TexCoord centroid = GetTexCoordinatesCentroid(projectedUV);

	std::map<TexCoord, D64> angles;
	for (const auto& vertex : projectedUV) 
	{
		D64 angle = std::atan2(vertex.V - centroid.V, vertex.U - centroid.U);
		if (angles.find(vertex) == angles.end())
			angles[vertex] = angle;
	}

	std::sort(projectedUV.begin(), projectedUV.end(), [&](const TexCoord& v1, const TexCoord& v2)
	{
		return angles[v1] < angles[v2];
	});
}

TexCoord Lightmapper::GetTexCoordinatesCentroid(std::vector<TexCoord>& projectedUV)
{
	TexCoord centroid;
	U32 n = 1 / projectedUV.size();

	for (const auto& vertex : projectedUV)
		centroid = centroid + vertex;

	return centroid * n;
}

D64 Lightmapper::CalculateDirection(const TexCoord& point, const TexCoord& a, const TexCoord& b)
{
	return (point.U - b.U) * (a.V - b.V) - (point.V - b.V) * (a.U - b.U);
}

Vector3D Lightmapper::CalculateBarycentric(const Vector3D& point, const Triangle& triangle)
{
	Vector3D a = triangle[1].Origin - triangle[0].Origin;
	Vector3D b = triangle[2].Origin - triangle[0].Origin;
	Vector3D c = point - triangle[0].Origin;

	D64 d00 = Vector3D::Dot(a, a);
	D64 d01 = Vector3D::Dot(a, b);
	D64 d11 = Vector3D::Dot(b, b);
	D64 d20 = Vector3D::Dot(c, a);
	D64 d21 = Vector3D::Dot(c, b);

	D64 denominator = d00 * d11 - d01 * d01;

	D64 v = (d11 * d20 - d01 * d21) / denominator;
	D64 w = (d00 * d21 - d01 * d20) / denominator;
	D64 u = 1 - v - w;

	return Vector3D(u, v, w);
}

TexCoord Lightmapper::CalculateUVFromBarycentric(const Vector3D& barycentricCoordinates, const Triangle& triangle)
{
	TexCoord uvCoordinates;
	for (size_t i = 0; i < 3; i++)
		uvCoordinates += triangle[i].UV * barycentricCoordinates[i];

	return uvCoordinates;
}
