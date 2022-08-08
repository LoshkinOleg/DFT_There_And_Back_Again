#include <Application.h>

#include <string>
#include <cassert>
#include <iostream>
#include <array>

struct Vec2
{
	float x = 0;
	float y = 0;
};

struct Vec3
{
	float x = 0;
	float y = 0;
	float z = 0;
};

struct Vec4
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1; // 1 by default since that's what we usually need when dealing with 4x4 transform matrices.

	inline bool operator==(const Vec4& other) const
	{
		return	x == other.x &&
				y == other.y &&
				z == other.z &&
				w == other.w;
	}
};

struct Mat4x4
{
	float m00 = 1.0f; float m01 = 0.0f; float m02 = 0.0f; float m03 = 0.0f;
	float m10 = 0.0f; float m11 = 1.0f; float m12 = 0.0f; float m13 = 0.0f;
	float m20 = 0.0f; float m21 = 0.0f; float m22 = 1.0f; float m23 = 0.0f;
	float m30 = 0.0f; float m31 = 0.0f; float m32 = 0.0f; float m33 = 1.0f;

	inline bool operator==(const Mat4x4& other) const
	{
		return	m00 == other.m00 && m01 == other.m01 && m02 == other.m02 && m03 == other.m03 &&
				m10 == other.m10 && m11 == other.m11 && m12 == other.m12 && m13 == other.m13 &&
				m20 == other.m20 && m21 == other.m21 && m22 == other.m22 && m23 == other.m23 &&
				m30 == other.m30 && m31 == other.m31 && m32 == other.m32 && m33 == other.m33;
	}
};

inline Mat4x4 MatrixMultiplication(const Mat4x4 a, const Mat4x4 b)
{
	return
	{
		a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30,	a.m00* b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31,	a.m00* b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32,	a.m00* b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33,
		a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30,	a.m10* b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31,	a.m10* b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32,	a.m10* b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33,
		a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30,	a.m20* b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31,	a.m20* b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32,	a.m20* b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33,
		a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30,	a.m30* b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31,	a.m30* b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32,	a.m30* b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33
	};
}

inline Vec4 MatrixVectorMultiplication(const Mat4x4 a, const Vec4 b)
{
	return
	{
		a.m00 * b.x + a.m01 * b.y + a.m02 * b.z + a.m03 * b.w,
		a.m10 * b.x + a.m11 * b.y + a.m12 * b.z + a.m13 * b.w,
		a.m20 * b.x + a.m21 * b.y + a.m22 * b.z + a.m23 * b.w,
		a.m30 * b.x + a.m31 * b.y + a.m32 * b.z + a.m33 * b.w
	};
}

constexpr inline Mat4x4 OrthogonalProjectionMatrix(const float near, const float far, const float right, const float left, const float bottom, const float top)
{
	// Taken from: https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix

	return
	{
		2.0f/(right-left),	0.0f,				0.0f,				-(right+left)/(right-left),
		0.0f,				2.0f/(top-bottom),	0.0f,				-(top+bottom)/(top-bottom),
		0.0f,				0.0f,				-2.0f/(far-near),	-(far+near)/(far-near),
		0.0f,				0.0f,				0.0f,				1.0f
	};
}

void PrintVec(const Vec4& v)
{
	std::cout << "(" << std::to_string(v.x) << " ; " << std::to_string(v.y) << " ; " << std::to_string(v.z) << " ; " << std::to_string(v.w) << ")" << std::endl;
}

void PrintMat(const Mat4x4& m)
{
	std::cout << "(\n" << std::to_string(m.m00) << " ; " << std::to_string(m.m01) << " ; " << std::to_string(m.m02) << " ; " << std::to_string(m.m03) << ";\n";
	std::cout << std::to_string(m.m10) << " ; " << std::to_string(m.m11) << " ; " << std::to_string(m.m12) << " ; " << std::to_string(m.m13) << ";\n";
	std::cout << std::to_string(m.m20) << " ; " << std::to_string(m.m21) << " ; " << std::to_string(m.m22) << " ; " << std::to_string(m.m23) << ";\n";
	std::cout << std::to_string(m.m30) << " ; " << std::to_string(m.m31) << " ; " << std::to_string(m.m32) << " ; " << std::to_string(m.m33) << ";\n)" << std::endl;
}

// Following 3DTI's coordinate convention... really should have followed the mathematical right-hand convention instead... oh well.
constexpr const Vec3 FRONT = { 1.0f, 0.0f, 0.0f };
constexpr const Vec3 LEFT = { 0.0f, 1.0f, 0.0f };
constexpr const Vec3 UP = { 0.0f, 0.0f, 1.0f };

// Box volume used for orthogonal projection.
struct Box
{
	float back =	-1.0f; // -X
	float front =	 1.0f; // +X
	float right =	-1.0f; // -Y
	float left =	 1.0f; // +Y
	float bottom =	-1.0f; // -Z
	float top =		 1.0f; // +Z
};

// Triangles composing a cube with origin at 0,0,0 and side len of 2. CCW winding for front-facing triangles, CW for back-facing triangles.
constexpr const std::array<float, 3 * 3 * 2 * 6> CUBE =
{
	// Back face.
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,+1.0f,
	-1.0f,+1.0f,+1.0f,

	-1.0f,-1.0f,-1.0f,
	-1.0f,+1.0f,+1.0f,
	-1.0f,+1.0f,-1.0f,

	// Right face.
	-1.0f,-1.0f,-1.0f,
	+1.0f,-1.0f,-1.0f,
	+1.0f,-1.0f,+1.0f,

	-1.0f,-1.0f,-1.0f,
	+1.0f,-1.0f,+1.0f,
	-1.0f,-1.0f,+1.0f,

	// Front face.
	+1.0f,-1.0f,-1.0f,
	+1.0f,+1.0f,-1.0f,
	+1.0f,-1.0f,+1.0f,

	+1.0f,+1.0f,-1.0f,
	+1.0f,+1.0f,+1.0f,
	+1.0f,-1.0f,+1.0f,

	// Left face.
	+1.0f,+1.0f,-1.0f,
	-1.0f,+1.0f,-1.0f,
	-1.0f,+1.0f,+1.0f,

	+1.0f,+1.0f,-1.0f,
	-1.0f,+1.0f,+1.0f,
	+1.0f,+1.0f,+1.0f,

	// Bottom face.
	+1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	+1.0f,+1.0f,-1.0f,

	+1.0f,+1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,+1.0f,-1.0f,

	// Top face.
	-1.0f,+1.0f,+1.0f,
	-1.0f,-1.0f,+1.0f,
	+1.0f,-1.0f,+1.0f,

	-1.0f,+1.0f,+1.0f,
	+1.0f,-1.0f,+1.0f,
	+1.0f,+1.0f,+1.0f
};

inline Mat4x4 RotationMatrix(const float yaw, const float pitch, const float roll)
{
	// Taken from: https://en.wikipedia.org/wiki/Rotation_matrix

	const float cosa = std::cosf(yaw);
	const float sina = std::sinf(yaw);
	const float cosb = std::cosf(pitch);
	const float sinb = std::sinf(pitch);
	const float cosy = std::cosf(roll);
	const float siny = std::sinf(roll);

	return
	{
		cosb*cosy,		sina*sinb*cosy - cosa*siny,		cosa*sinb*cosy + sina*siny,		0.0f,
		cosb*siny,		sina*sinb*siny + cosa*cosy,		cosa*sinb*siny - sina*cosy,		0.0f,
		-sinb,			sina*cosb,						cosa*cosb,						0.0f,
		0.0f,			0.0f,							0.0f,							1.0f
	};
}

void MyApp::Application::OnStart()
{
	sdl_.RegisterRenderCallback([&]()
	{
		constexpr const float PI = 3.14159265359f;
		constexpr const Box BOUNDS{ -2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f };
		constexpr const static Mat4x4 ORTHO_PROJ_MAT = OrthogonalProjectionMatrix(BOUNDS.back, BOUNDS.front, BOUNDS.right, BOUNDS.left, BOUNDS.bottom, BOUNDS.top);
		constexpr const static Mat4x4 VIEW_MAT{}; // Immobile view placed at origin with no rotation.

		// Start with a cube of scale 1.0, placed at origin and pitched slightly towards the viewer.
		const static Mat4x4 modelMat = RotationMatrix(0.0f, PI / 12, 0.0f);

		static float theta = 0.0f;
		const Mat4x4 rotation = RotationMatrix(0.0f, 0.0f, theta); // This frame's yaw rotation.

		// Draw triangles.
		for (size_t i = 0; i < CUBE.size() - 9; i += 9)
		{
			Vec4 pt0, pt1, pt2; // World position.
			// Single vertices.
			pt0 = {CUBE[i], CUBE[i + 1], CUBE[i + 2], 1.0f};
			pt1 = {CUBE[i + 3], CUBE[i + 4], CUBE[i + 5], 1.0f};
			pt2 = {CUBE[i + 6], CUBE[i + 7], CUBE[i + 8], 1.0f};

			// To model space.
			pt0 = MatrixVectorMultiplication(modelMat, pt0);
			pt1 = MatrixVectorMultiplication(modelMat, pt1);
			pt2 = MatrixVectorMultiplication(modelMat, pt2);

			// Rotate the cube around the Z axis.
			pt0 = MatrixVectorMultiplication(rotation, pt0);
			pt1 = MatrixVectorMultiplication(rotation, pt1);
			pt2 = MatrixVectorMultiplication(rotation, pt2);

			// To view space.
			pt0 = MatrixVectorMultiplication(VIEW_MAT, pt0);
			pt1 = MatrixVectorMultiplication(VIEW_MAT, pt1);
			pt2 = MatrixVectorMultiplication(VIEW_MAT, pt2);

			// To clip space.
			pt0 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt0);
			pt1 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt1);
			pt2 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt2);

			// Cull points outside the viewing volume.
			if (pt0.x < BOUNDS.back || pt0.x > BOUNDS.front ||
				pt0.y < BOUNDS.right || pt0.y > BOUNDS.left ||
				pt0.z < BOUNDS.bottom || pt0.z > BOUNDS.top) continue;
			if (pt1.x < BOUNDS.back || pt1.x > BOUNDS.front ||
				pt1.y < BOUNDS.right || pt1.y > BOUNDS.left ||
				pt1.z < BOUNDS.bottom || pt1.z > BOUNDS.top) continue;
			if (pt2.x < BOUNDS.back || pt2.x > BOUNDS.front ||
				pt2.y < BOUNDS.right || pt2.y > BOUNDS.left ||
				pt2.z < BOUNDS.bottom || pt2.z > BOUNDS.top) continue;

			// Perspective divide. Yields culled normal device coordinates.
			const Vec3 ndcPt0 =
			{
				pt0.x / pt0.w, pt0.y / pt0.w, pt0.z / pt0.w
			};
			const Vec3 ndcPt1 =
			{
				pt1.x / pt1.w, pt1.y / pt1.w, pt1.z / pt1.w
			};
			const Vec3 ndcPt2 =
			{
				pt2.x / pt2.w, pt2.y / pt2.w, pt2.z / pt2.w
			};
			assert(abs(ndcPt0.x) >= 0.0f && abs(ndcPt0.x) <= 1.0f &&
				   abs(ndcPt0.y) >= 0.0f && abs(ndcPt0.y) <= 1.0f &&
				   abs(ndcPt0.z) >= 0.0f && abs(ndcPt0.z) <= 1.0f && "Ndc coordinate lies outside the bounds of the viewing volume.");
			assert(abs(ndcPt1.x) >= 0.0f && abs(ndcPt1.x) <= 1.0f &&
				   abs(ndcPt1.y) >= 0.0f && abs(ndcPt1.y) <= 1.0f &&
				   abs(ndcPt1.z) >= 0.0f && abs(ndcPt1.z) <= 1.0f && "Ndc coordinate lies outside the bounds of the viewing volume.");
			assert(abs(ndcPt2.x) >= 0.0f && abs(ndcPt2.x) <= 1.0f &&
				   abs(ndcPt2.y) >= 0.0f && abs(ndcPt2.y) <= 1.0f &&
				   abs(ndcPt2.z) >= 0.0f && abs(ndcPt2.z) <= 1.0f && "Ndc coordinate lies outside the bounds of the viewing volume.");

			// Clip to window coords following "Viewport transform" section of: https://www.khronos.org/opengl/wiki/Viewport_Transform (note: adjusted to fit my coordinate convention). Yields Window coordinates.
			const Vec3 windowPt0 =
			{
				(1.0f - 0.0f)		* 0.5f * ndcPt0.x + 0 + (1.0f + 0.0f)		* 0.5f,
				sdl_.displaySize	* 0.5f * ndcPt0.y + 0 + sdl_.displaySize	* 0.5f,
				sdl_.displaySize	* 0.5f * ndcPt0.z + 0 + sdl_.displaySize	* 0.5f
			};
			const Vec3 windowPt1 =
			{
				(1.0f - 0.0f)		* 0.5f * ndcPt1.x + 0 + (1.0f + 0.0f)		* 0.5f,
				sdl_.displaySize	* 0.5f * ndcPt1.y + 0 + sdl_.displaySize	* 0.5f,
				sdl_.displaySize	* 0.5f * ndcPt1.z + 0 + sdl_.displaySize	* 0.5f
			};
			const Vec3 windowPt2 =
			{
				(1.0f - 0.0f)		* 0.5f * ndcPt2.x + 0 + (1.0f + 0.0f)		* 0.5f,
				sdl_.displaySize	* 0.5f * ndcPt2.y + 0 + sdl_.displaySize	* 0.5f,
				sdl_.displaySize	* 0.5f * ndcPt2.z + 0 + sdl_.displaySize	* 0.5f
			};
			assert(windowPt0.y >= 0.0f && windowPt0.y <= sdl_.displaySize &&
				   windowPt0.z >= 0.0f && windowPt0.z <= sdl_.displaySize && "Window point lies outside the screen's bounds.");
			assert(windowPt1.y >= 0.0f && windowPt1.y <= sdl_.displaySize &&
				   windowPt1.z >= 0.0f && windowPt1.z <= sdl_.displaySize && "Window point lies outside the screen's bounds.");
			assert(windowPt2.y >= 0.0f && windowPt2.y <= sdl_.displaySize &&
				   windowPt2.z >= 0.0f && windowPt2.z <= sdl_.displaySize && "Window point lies outside the screen's bounds.");

			// For us, +Y is right, for SDL, right is +X. For us, +Z is up, for SDL, up is -Y. Hence the conversions.
			const Vec2 screenPt0 = {windowPt0.y, sdl_.displaySize - windowPt0.z};
			const Vec2 screenPt1 = {windowPt1.y, sdl_.displaySize - windowPt1.z};
			const Vec2 screenPt2 = {windowPt2.y, sdl_.displaySize - windowPt2.z};
			constexpr const float RECT_WIDTH = 6.0f;

			// Draw vertices.
			sdl_.RenderFilledRect(screenPt0.x - RECT_WIDTH * 0.5f, screenPt0.x + RECT_WIDTH * 0.5f, screenPt0.y - RECT_WIDTH * 0.5f, screenPt0.y + RECT_WIDTH * 0.5f);
			sdl_.RenderFilledRect(screenPt1.x - RECT_WIDTH * 0.5f, screenPt1.x + RECT_WIDTH * 0.5f, screenPt1.y - RECT_WIDTH * 0.5f, screenPt1.y + RECT_WIDTH * 0.5f);
			sdl_.RenderFilledRect(screenPt2.x - RECT_WIDTH * 0.5f, screenPt2.x + RECT_WIDTH * 0.5f, screenPt2.y - RECT_WIDTH * 0.5f, screenPt2.y + RECT_WIDTH * 0.5f);

			// Draw edges.
			sdl_.RenderLine(screenPt0.x, screenPt0.y, screenPt1.x, screenPt1.y);
			sdl_.RenderLine(screenPt1.x, screenPt1.y, screenPt2.x, screenPt2.y);
			sdl_.RenderLine(screenPt2.x, screenPt2.y, screenPt0.x, screenPt0.y);
		}

		// Increment yaw rotation.
		theta += 0.0001f;
	});
}

void MyApp::Application::OnUpdate()
{

}

void MyApp::Application::OnShutdown()
{

}