#include "3DData.h"

Vector3D operator*(const Vector3D& i, const Matrix4x4& m)
{
	Vector3D v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}
Vector3D operator*(const Matrix4x4& mat, const Vector3D& vec)
{
	return vec * mat;
}
Vector3D operator*(const Vector3D& m1, const float k)
{
	return { m1.x * k, m1.y * k, m1.z * k };
}
Vector3D operator/(const Vector3D& m1, const float k)
{
	return { m1.x / k, m1.y / k, m1.z / k };
}
Vector3D operator*(const Vector3D& v1, const Vector3D& v2)
{
	return { v1.x * v2.x, v1.y + v2.y, v1.z * v2.z, v1.w * v2.w };
}
Vector3D operator/(const Vector3D& v1, const Vector3D& v2)
{
	return { v1.x / v2.x, v1.y + v2.y, v1.z / v2.z, v1.w / v2.w };
}
Vector3D operator+(const Vector3D& m1, const Vector3D& m2)
{
	return { m1.x + m2.x, m1.y + m2.y, m1.z + m2.z };
}
Vector3D operator-(const Vector3D& m1, const Vector3D& m2)
{
	return { m1.x - m2.x, m1.y - m2.y, m1.z - m2.z };
}
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}

Vector3D Vector3D::planeIntersect(Vector3D& plane_p, Vector3D& plane_n, Vector3D& lineStart, Vector3D& lineEnd, float& t)
{
	plane_n = plane_n.normalized();
	float plane_d = -Vector3D::dotProduct(plane_n, plane_p);
	float ad = Vector3D::dotProduct(lineStart, plane_n);
	float bd = Vector3D::dotProduct(lineEnd, plane_n);
	t = (-plane_d - ad) / (bd - ad);
	Vector3D lineStartToEnd = lineEnd - lineStart;
	Vector3D lineToIntersect = lineStartToEnd * t;
	return lineStart + lineToIntersect;
}

float Vector3D::dotProduct(Vector3D& v1, Vector3D& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
Vector3D Vector3D::crossProduct(Vector3D& v1, Vector3D& v2)
{
	Vector3D v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}
float Vector3D::length()
{
	return Vector::qSqrt(dotProduct(*this, *this), 1);
}
void Vector3D::normalize()
{
	float l = length();
	x /= l;
	y /= l;
	z /= l;
}
Vector3D Vector3D::normalized()
{
	float l = length();
	return { x / l, y / l, z / l };
}

Matrix4x4 Matrix4x4::makeIdentity()
{
	Matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}
Matrix4x4 Matrix4x4::makeRotationX(float xAngleRadians)
{
	Matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(xAngleRadians);
	matrix.m[1][2] = sinf(xAngleRadians);
	matrix.m[2][1] = -sinf(xAngleRadians);
	matrix.m[2][2] = cosf(xAngleRadians);
	matrix.m[3][3] = 1.0f;
	return matrix;
}
Matrix4x4 Matrix4x4::makeRotationY(float yAngleRadians)
{
	Matrix4x4 matrix;
	matrix.m[0][0] = cosf(yAngleRadians);
	matrix.m[0][2] = sinf(yAngleRadians);
	matrix.m[2][0] = -sinf(yAngleRadians);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(yAngleRadians);
	matrix.m[3][3] = 1.0f;
	return matrix;
}
Matrix4x4 Matrix4x4::makeRotationZ(float zAngleRadians)
{
	Matrix4x4 matrix;
	matrix.m[0][0] = cosf(zAngleRadians);
	matrix.m[0][1] = sinf(zAngleRadians);
	matrix.m[1][0] = -sinf(zAngleRadians);
	matrix.m[1][1] = cosf(zAngleRadians);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}
Matrix4x4 Matrix4x4::makeTranslation(float x, float y, float z)
{
	Matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}
Matrix4x4 Matrix4x4::makeProjectionMatrix(float fovDegrees, float aspectRatio, float near, float far)
{
	float fovRad = 1.0f / tanf(Rotation::toRadians(fovDegrees * 0.5f));
	Matrix4x4 matrix;
	matrix.m[0][0] = aspectRatio * fovRad;
	matrix.m[1][1] = fovRad;
	matrix.m[2][2] = far / (far - near);
	matrix.m[3][2] = (-far * near) / (far - near);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}
Matrix4x4 Matrix4x4::makePointedAt(Vector3D& pos, Vector3D& target, Vector3D& up)
{
	Vector3D newForward = target - pos;
	newForward = newForward.normalized();

	Vector3D a = newForward * Vector3D::dotProduct(up, newForward);
	Vector3D newUp = up - a;
	newUp = newUp.normalized();

	Vector3D newRight = Vector3D::crossProduct(newUp, newForward);

	Matrix4x4 matrix;
	matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;
}
Matrix4x4 Matrix4x4::qInverse()
{

	Matrix4x4 matrix;
	matrix.m[0][0] = m[0][0]; matrix.m[0][1] = m[1][0]; matrix.m[0][2] = m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m[0][1]; matrix.m[1][1] = m[1][1]; matrix.m[1][2] = m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m[0][2]; matrix.m[2][1] = m[1][2]; matrix.m[2][2] = m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m[3][0] * matrix.m[0][0] + m[3][1] * matrix.m[1][0] + m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m[3][0] * matrix.m[0][1] + m[3][1] * matrix.m[1][1] + m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m[3][0] * matrix.m[0][2] + m[3][1] * matrix.m[1][2] + m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

float Triangle::clipAgainstPlane(Vector3D plane_p, Vector3D plane_n, Triangle& in_tri, Triangle& out_tri1, Triangle& out_tri2)
{
	plane_n.normalize();

	auto dist = [&](Vector3D& p)
	{
		Vector3D n = p.normalized();
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector3D::dotProduct(plane_n, plane_p));
	};

	Vector3D* inside_points[3];  int nInsidePointCount = 0;
	Vector3D* outside_points[3]; int nOutsidePointCount = 0;
	Vector2D* inside_tex[3]; int nInsideTexCount = 0;
	Vector2D* outside_tex[3]; int nOutsideTexCount = 0;

	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; inside_tex[nInsideTexCount++] = &in_tri.t[0]; }
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[0]; outside_tex[nOutsideTexCount++] = &in_tri.t[0];
	}
	if (d1 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri.p[1]; inside_tex[nInsideTexCount++] = &in_tri.t[1];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[1];  outside_tex[nOutsideTexCount++] = &in_tri.t[1];
	}
	if (d2 >= 0) {
		inside_points[nInsidePointCount++] = &in_tri.p[2]; inside_tex[nInsideTexCount++] = &in_tri.t[2];
	}
	else {
		outside_points[nOutsidePointCount++] = &in_tri.p[2];  outside_tex[nOutsideTexCount++] = &in_tri.t[2];
	}

	if (nInsidePointCount == 0)
		return 0;

	if (nInsidePointCount == 3)
	{
		out_tri1 = in_tri;
		return 1;
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		out_tri1.col = in_tri.col;

		out_tri1.p[0] = *inside_points[0];
		out_tri1.t[0] = *inside_tex[0];

		float t;
		out_tri1.p[1] = Vector3D::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.t[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.t[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.t[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

		out_tri1.p[2] = Vector3D::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
		out_tri1.t[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.t[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.t[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;

		return 1;
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		out_tri1.col = in_tri.col;

		out_tri2.col = in_tri.col;

		out_tri1.p[0] = *inside_points[0];
		out_tri1.p[1] = *inside_points[1];
		out_tri1.t[0] = *inside_tex[0];
		out_tri1.t[1] = *inside_tex[1];

		float t;
		out_tri1.p[2] = Vector3D::planeIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.t[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
		out_tri1.t[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
		out_tri1.t[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

		out_tri2.p[0] = *inside_points[1];
		out_tri2.t[0] = *inside_tex[1];
		out_tri2.p[1] = out_tri1.p[2];
		out_tri2.t[1] = out_tri1.t[2];
		out_tri2.p[2] = Vector3D::planeIntersect(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
		out_tri2.t[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
		out_tri2.t[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
		out_tri2.t[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;
		return 2; // Return two newly formed triangles which form a quad
	}
}

bool Mesh::loadFromOBJ(std::string fileName, bool hasTexture)
{
	using namespace std;
	ifstream f(fileName);
	if (!f.is_open())
		return false;

	vector<Vector3D> verts;
	vector<Vector2D> texs;

	while (!f.eof()) {
		char line[MAX_OBJ_LINE_CHARS];
		char junk;
		f.getline(line, MAX_OBJ_LINE_CHARS);
		stringstream s;

		s << line;
		if (line[0] == 'v') {
			if (line[1] == 't') {
				Vector2D v;
				s >> junk >> junk >> v.u >> v.v;
				texs.push_back(v);
			}
			else {
				Vector3D v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
		}
		if (!hasTexture) {
			if (line[0] == 'f') {
				int face[3] = { 0 };
				s >> junk >> face[0] >> face[1] >> face[2];
				int v1 = face[0] - 1;
				int v2 = face[1] - 1;
				int v3 = face[2] - 1;
				Triangle fTri = { verts[v1 < 0 ? 0 : v1], verts[v2 < 0 ? 0 : v2], verts[v3 < 0 ? 0 : v3] };
				tris.push_back(fTri);
			}
		}
		else {
			if (line[0] == 'f') {
				s >> junk;

				string tokens[6];
				int nTokenCount = -1;

				while (!s.eof())
				{
					char c = s.get();
					if (c == ' ' || c == '/')
						nTokenCount++;
					else
						tokens[nTokenCount].append(1, c);
				}

				tokens[nTokenCount].pop_back();

				tris.push_back({ verts[stoi(tokens[0]) - 1], verts[stoi(tokens[2]) - 1], verts[stoi(tokens[4]) - 1],
					texs[stoi(tokens[1]) - 1], texs[stoi(tokens[3]) - 1], texs[stoi(tokens[5]) - 1] });
			}
		}
	}
	return true;
}

Vector3D Camera::lookVector()
{
	Vector3D target = { 0, 0, 1 };
	Vector3D up = { 0, 1, 0 };
	Matrix4x4 mRotation = Matrix4x4::makeRotationX(rotation.x) * Matrix4x4::makeRotationY(rotation.y) * Matrix4x4::makeRotationZ(rotation.z);
	target = (target * mRotation);
	return target;
}