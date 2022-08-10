#pragma once

namespace MyMath
{
	constexpr const float PI = 3.14159265359f;
	constexpr const float E =  2.71828182845f;

	struct Vec2
	{
		float x = 0;
		float y = 0;
	};

	constexpr const Vec2 VEC2_ZERO = {};
	constexpr const Vec2 VEC2_ONE = {1.0f, 1.0f};
	constexpr const Vec2 VEC2_RIGHT = {1.0f, 0.0f};
	constexpr const Vec2 VEC2_DOWN = {0.0f, 1.0f};

	struct Vec3
	{
		float x = 0;
		float y = 0;
		float z = 0;
	};

	// Following 3DTI's coordinate convention... really should have followed the mathematical right-hand convention instead... oh well.
	constexpr const Vec3 VEC3_ZERO = {};
	constexpr const Vec3 VEC3_ONE = { 1.0f, 1.0f, 1.0f };
	constexpr const Vec3 VEC3_FRONT = { 1.0f, 0.0f, 0.0f };
	constexpr const Vec3 VEC3_LEFT = { 0.0f, 1.0f, 0.0f };
	constexpr const Vec3 VEC3_UP = { 0.0f, 0.0f, 1.0f };

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

	constexpr const Vec4 VEC4_ZERO = { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr const Vec4 VEC4_TRUE_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr const Vec4 VEC4_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr const Vec4 VEC4_FRONT = { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr const Vec4 VEC4_LEFT = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr const Vec4 VEC4_UP = { 0.0f, 0.0f, 1.0f, 1.0f };

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

		Mat4x4 Inverse() const
		{
			// Taken from: https://www.thecrazyprogrammer.com/2017/02/c-c-program-find-inverse-matrix.html

			float det = 0.0f;
			const float mat[4][4] =
			{
				{m00, m01, m02, m03},
				{m10, m11, m12, m13},
				{m20, m21, m22, m23},
				{m30, m31, m32, m33}
			};
			float imat[4][4] = {};

			for (size_t i = 0; i < 4; i++)
			{
				det += (mat[0][i] * (mat[1][(i + 1) % 3] * mat[2][(i + 2) % 3] - mat[1][(i + 2) % 3] * mat[2][(i + 1) % 3]));
			}

			for (size_t r = 0; r < 4; r++)
			{
				for (size_t c = 0; c < 4; c++)
				{
					imat[r][c] = ((mat[(c + 1) % 3][(r + 1) % 3] * mat[(c + 2) % 3][(r + 2) % 3]) - (mat[(c + 1) % 3][(r + 2) % 3] * mat[(c + 2) % 3][(r + 1) % 3])) / det;
				}
			}

			return
			{
				imat[0][0], imat[0][1], imat[0][2], imat[0][3],
				imat[1][0], imat[1][1], imat[1][2], imat[1][3],
				imat[2][0], imat[2][1], imat[2][2], imat[2][3],
				imat[3][0], imat[3][1], imat[3][2], imat[3][3]
			};
		}
	};

	constexpr const Mat4x4 MAT4_IDENTITY = Mat4x4{};

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
		// Rearranged version of: https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix

		assert(far > near && left > right && top > bottom && "Invalid bounds for an orthogonal projection matrix.");

		return
		{
			2.0f / (far - near),	0.0f,					0.0f,					-(far + near) / (far - near),
			0.0f,					2.0f / (left - right),	0.0f,					-(left + right) / (left - right),
			0.0f,					0.0f,					2.0f / (top - bottom),	-(top + bottom) / (top - bottom),
			0.0f,					0.0f,					0.0f,					1.0f
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

	constexpr const Box BOX_UNIT = {};

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

	inline Mat4x4 RotationMatrix(const Vec3 axis, const float rad)
	{
		// Taken from: https://en.wikipedia.org/wiki/Rotation_matrix

		const float cos = std::cosf(rad);
		const float sin = std::sinf(rad);
		const float mcos = 1.0f - cos;
		const float x = axis.x;
		const float y = axis.y;
		const float z = axis.z;

		return
		{
			cos+x*x*mcos,	x*y*mcos-z*sin,	x*z*mcos+y*sin,	0.0f,
			y*x*mcos+z*sin,	cos+y*y*mcos,	y*z*mcos-x*sin,	0.0f,
			z*x*mcos-y*sin,	z*y*mcos+x*sin,	cos+z*z*mcos,	0.0f,
			0.0f,			0.0f,			0.0f,			1.0f
		};
	}
}