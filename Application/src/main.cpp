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
		a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30,	a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31,	a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32,	a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33,
		a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30,	a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31,	a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32,	a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33,
		a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30,	a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31,	a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32,	a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33,
		a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30,	a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31,	a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32,	a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33
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
	// Taken from: https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix (note: using right - left since +Y is left)

	assert(far > near && left > right && top > bottom && "Invalid bounds for an orthogonal projection matrix.");

	return
	{
		2.0f / (far - near),	0.0f,				0.0f,				-(far + near) / (far - near),
		0.0f,				2.0f / (left - right),	0.0f,				-(left + right) / (left - right),
		0.0f,				0.0f,				2.0f / (top - bottom),	-(top + bottom) / (top - bottom),
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
	float back = -1.0f; // -X
	float front = 1.0f; // +X
	float right = -1.0f; // -Y
	float left = 1.0f; // +Y
	float bottom = -1.0f; // -Z
	float top = 1.0f; // +Z
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
		cosb * cosy,		sina * sinb * cosy - cosa * siny,		cosa * sinb * cosy + sina * siny,		0.0f,
		cosb * siny,		sina * sinb * siny + cosa * cosy,		cosa * sinb * siny - sina * cosy,		0.0f,
		-sinb,			sina * cosb,						cosa * cosb,						0.0f,
		0.0f,			0.0f,							0.0f,							1.0f
	};
}

inline float GenerateSine(const float n, const float sampleRate, const float frequency)
{
	constexpr const float PI = 3.14159265359f;

	return std::sinf((2.0f * PI * n / sampleRate) * frequency);
}

void MyApp::Application::OnStart()
{
	constexpr const size_t SAMPLE_RATE = 8000;
	constexpr const size_t FREQ = 441;

	static std::vector<float> sine(SAMPLE_RATE, 0.0f);
	for (size_t n = 0; n < sine.size(); n++)
	{
		sine[n] = GenerateSine(n, SAMPLE_RATE, FREQ);
	}

	sdl_.RegisterRenderCallback([&]()
	{
		constexpr const float PI = 3.14159265359f;
		constexpr const Box BOUNDS{ -2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f };
		constexpr const static Mat4x4 ORTHO_PROJ_MAT = OrthogonalProjectionMatrix(BOUNDS.back, BOUNDS.front, BOUNDS.right, BOUNDS.left, BOUNDS.bottom, BOUNDS.top);
		constexpr const static Mat4x4 VIEW_MAT{}; // Immobile view placed at origin with no rotation.

		// Start with a cube of scale 1.0, placed at origin and pitched slightly towards the viewer.
		static Mat4x4 modelMat{};
		modelMat.m23 = -5.0f;

		static float theta = 0.0f;
		const Mat4x4 rotation = RotationMatrix(0.0f, 0.0f, theta); // This frame's yaw rotation.

		// Draw sine.
		for (size_t n = 0; n < sine.size() - 2; n++)
		{
			constexpr const float SAMPLES_SPACING = 100.0f;

			Vec4 pt0, pt1; // World position.
			// Single vertices.
			pt0 = { std::cosf(sine[n]), std::sinf(sine[n]), SAMPLES_SPACING * float(n) / float(sine.size()), 1.0f };
			pt1 = { std::cosf(sine[n + 1]), std::sinf(sine[n + 1]), SAMPLES_SPACING * float(n + 1) / float(sine.size()), 1.0f };

			// To model space.
			pt0 = MatrixVectorMultiplication(modelMat, pt0);
			pt1 = MatrixVectorMultiplication(modelMat, pt1);

			// Rotate the cube around the Z axis.
			pt0 = MatrixVectorMultiplication(rotation, pt0);
			pt1 = MatrixVectorMultiplication(rotation, pt1);

			// To view space.
			pt0 = MatrixVectorMultiplication(VIEW_MAT, pt0);
			pt1 = MatrixVectorMultiplication(VIEW_MAT, pt1);

			// Cull points outside the viewing volume.
			if (pt0.x < BOUNDS.back || pt0.x > BOUNDS.front ||
				pt0.y < BOUNDS.right || pt0.y > BOUNDS.left ||
				pt0.z < BOUNDS.bottom || pt0.z > BOUNDS.top) continue;
			if (pt1.x < BOUNDS.back || pt1.x > BOUNDS.front ||
				pt1.y < BOUNDS.right || pt1.y > BOUNDS.left ||
				pt1.z < BOUNDS.bottom || pt1.z > BOUNDS.top) continue;

			// To clip space.
			pt0 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt0);
			pt1 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt1);

			assert(pt0.x >= -1.0f && pt0.x <= 1.0f &&
				   pt0.y >= -1.0f && pt0.y <= 1.0f &&
				   pt0.z >= -1.0f && pt0.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");
			assert(pt1.x >= -1.0f && pt1.x <= 1.0f &&
				   pt1.y >= -1.0f && pt1.y <= 1.0f &&
				   pt1.z >= -1.0f && pt1.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");

			const Vec3 ndcPt0 = {pt0.x, pt0.y, pt0.z};
			const Vec3 ndcPt1 = {pt1.x, pt1.y, pt1.z};

			// Clip to window coords following "Viewport transform" section of: https://www.khronos.org/opengl/wiki/Viewport_Transform (note: adjusted to fit my coordinate convention). Yields Window coordinates.
			const Vec2 windowPt0 =
			{
				sdl_.displaySize * 0.5f * -ndcPt0.y + 0 + sdl_.displaySize * 0.5f,
				sdl_.displaySize * 0.5f * ndcPt0.z + 0 + sdl_.displaySize * 0.5f
			};
			const Vec2 windowPt1 =
			{
				sdl_.displaySize * 0.5f * -ndcPt1.y + 0 + sdl_.displaySize * 0.5f,
				sdl_.displaySize * 0.5f * ndcPt1.z + 0 + sdl_.displaySize * 0.5f
			};
			assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl_.displaySize &&
				   windowPt0.y >= 0.0f && windowPt0.y <= sdl_.displaySize && "Window point lies outside the screen's bounds.");
			assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl_.displaySize &&
				   windowPt1.y >= 0.0f && windowPt1.y <= sdl_.displaySize && "Window point lies outside the screen's bounds.");

			const Vec2 screenPt0 = { windowPt0.x, sdl_.displaySize - windowPt0.y };
			const Vec2 screenPt1 = { windowPt1.x, sdl_.displaySize - windowPt1.y };

			// Draw vertices.
			sdl_.RenderLine(screenPt0.x, screenPt0.y, screenPt1.x, screenPt1.y);
		}

		// Increment yaw rotation.
		theta += 0.001f;
	});
}

void MyApp::Application::OnUpdate()
{

}

void MyApp::Application::OnShutdown()
{

}

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run();

	return 0;
}