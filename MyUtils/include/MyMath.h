#pragma once

#include <cassert>
#include <iostream>

namespace MyMath
{
	constexpr const float PI = 3.14159265359f;

	struct Vec2
	{
		float x = 0;
		float y = 0;

		constexpr inline bool operator==(const Vec2& other) const
		{
			return	x == other.x &&
					y == other.y;
		}

		constexpr inline Vec2 operator+(const Vec2& other) const
		{
			return
			{
				x + other.x,
				y + other.y
			};
		}

		constexpr inline Vec2 operator-(const Vec2& other) const
		{
			return
			{
				x - other.x,
				y - other.y
			};
		}

		constexpr inline Vec2 operator-() const
		{
			return
			{
				-x,
				-y
			};
		}
	};

	// Following SDL's coordinate conventions for screen space coordinates.
	constexpr const Vec2 VEC2_ZERO = {0.0f, 0.0f};
	constexpr const Vec2 VEC2_RIGHT = {1.0f, 0.0f};
	constexpr const Vec2 VEC2_DOWN = {0.0f, 1.0f};
	constexpr const Vec2 VEC2_LEFT = -VEC2_RIGHT;
	constexpr const Vec2 VEC2_UP = -VEC2_DOWN;
	constexpr const Vec2 VEC2_ONE = VEC2_RIGHT + VEC2_DOWN;

	struct Vec3
	{
		float x = 0;
		float y = 0;
		float z = 0;

		constexpr inline bool operator==(const Vec3& other) const
		{
			return	x == other.x &&
					y == other.y &&
					z == other.z;
		}

		constexpr inline Vec3 operator+(const Vec3& other) const
		{
			return
			{
				x + other.x,
				y + other.y,
				z + other.z
			};
		}

		constexpr inline Vec3 operator-(const Vec3& other) const
		{
			return
			{
				x - other.x,
				y - other.y,
				z - other.z
			};
		}

		constexpr inline Vec3 operator-() const
		{
			return
			{
				-x,
				-y,
				-z
			};
		}
	};

	// Following the right-hand-rule for 3D coordinates: https://en.wikipedia.org/wiki/Right-hand_rule
	constexpr const Vec3 VEC3_ZERO =	{ 0.0f, 0.0f, 0.0f };
	constexpr const Vec3 VEC3_RIGHT =	{ 1.0f, 0.0f, 0.0f };
	constexpr const Vec3 VEC3_FRONT =	{ 0.0f, 1.0f, 0.0f };
	constexpr const Vec3 VEC3_UP =		{ 0.0f, 0.0f, 1.0f };
	constexpr const Vec3 VEC3_LEFT = -VEC3_RIGHT;
	constexpr const Vec3 VEC3_BACK = -VEC3_FRONT;
	constexpr const Vec3 VEC3_DOWN = -VEC3_UP;
	constexpr const Vec3 VEC3_ONE = VEC3_RIGHT + VEC3_FRONT + VEC3_UP;

	struct Vec4
	{
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 1; // 1 by default since that's what we usually need when dealing with 4x4 transform matrices.

		constexpr inline bool operator==(const Vec4& other) const
		{
			return	x == other.x &&
					y == other.y &&
					z == other.z &&
					w == other.w;
		}

		constexpr inline Vec4 operator+(const Vec4& other) const
		{
			return
			{
				x + other.x,
				y + other.y,
				z + other.z,
				w + other.w
			};
		}

		constexpr inline Vec4 operator-(const Vec4& other) const
		{
			return
			{
				x - other.x,
				y - other.y,
				z - other.z,
				w - other.w
			};
		}

		constexpr inline Vec4 operator-() const
		{
			return
			{
				-x,
				-y,
				-z,
				-w
			};
		}
	};

	// Following the right-hand-rule as well for 4D coordinates but distinguishing between "true" zero and unit vectors and "regular" versions of them that have the w component set to 1 since Vec4's are usually used in 4x4 matrices.
	constexpr const Vec4 VEC4_TRUE_ZERO =	{ VEC3_ZERO.x,	VEC3_ZERO.y,	VEC3_ZERO.z,	0.0f };
	constexpr const Vec4 VEC4_TRUE_RIGHT =	{ VEC3_RIGHT.x, VEC3_RIGHT.y,	VEC3_RIGHT.z,	0.0f };
	constexpr const Vec4 VEC4_TRUE_FRONT =	{ VEC3_FRONT.x, VEC3_FRONT.y,	VEC3_FRONT.z,	0.0f };
	constexpr const Vec4 VEC4_TRUE_UP =		{ VEC3_UP.x,	VEC3_UP.y,		VEC3_UP.z,		0.0f };
	constexpr const Vec4 VEC4_W_UNIT =		{ VEC3_ZERO.x,	VEC3_ZERO.y,	VEC3_ZERO.z,	1.0f };
	constexpr const Vec4 VEC4_ZERO =		 VEC4_TRUE_ZERO		+ VEC4_W_UNIT;
	constexpr const Vec4 VEC4_RIGHT =		 VEC4_TRUE_RIGHT	+ VEC4_W_UNIT;
	constexpr const Vec4 VEC4_FRONT =		 VEC4_TRUE_FRONT	+ VEC4_W_UNIT;
	constexpr const Vec4 VEC4_UP =			 VEC4_TRUE_UP		+ VEC4_W_UNIT;
	constexpr const Vec4 VEC4_TRUE_LEFT =	-VEC4_TRUE_RIGHT;
	constexpr const Vec4 VEC4_TRUE_BACK =	-VEC4_TRUE_FRONT;
	constexpr const Vec4 VEC4_TRUE_DOWN =	-VEC4_TRUE_UP;
	constexpr const Vec4 VEC4_LEFT =		 VEC4_TRUE_LEFT		+ VEC4_W_UNIT;
	constexpr const Vec4 VEC4_BACK =		 VEC4_TRUE_BACK		+ VEC4_W_UNIT;
	constexpr const Vec4 VEC4_DOWN =		 VEC4_TRUE_DOWN		+ VEC4_W_UNIT;

	struct Mat4x4
	{
		float m00 = 1.0f; float m01 = 0.0f; float m02 = 0.0f; float m03 = 0.0f;
		float m10 = 0.0f; float m11 = 1.0f; float m12 = 0.0f; float m13 = 0.0f;
		float m20 = 0.0f; float m21 = 0.0f; float m22 = 1.0f; float m23 = 0.0f;
		float m30 = 0.0f; float m31 = 0.0f; float m32 = 0.0f; float m33 = 1.0f;

		constexpr inline bool operator==(const Mat4x4& other) const
		{
			return	m00 == other.m00 && m01 == other.m01 && m02 == other.m02 && m03 == other.m03 &&
				m10 == other.m10 && m11 == other.m11 && m12 == other.m12 && m13 == other.m13 &&
				m20 == other.m20 && m21 == other.m21 && m22 == other.m22 && m23 == other.m23 &&
				m30 == other.m30 && m31 == other.m31 && m32 == other.m32 && m33 == other.m33;
		}

		constexpr Mat4x4 Inverse() const
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

	constexpr inline Vec4 MatrixVectorMultiplication(const Mat4x4 a, const Vec4 b)
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
			2.0f / (right - left),	0.0f,					0.0f,					-(right + left) / (right - left),
			0.0f,					2.0f / (far - near),	0.0f,					-(far + near) / (far - near),
			0.0f,					0.0f,					2.0f / (top - bottom),	-(top + bottom) / (top - bottom),
			0.0f,					0.0f,					0.0f,					1.0f
		};
	}

	inline void PrintVec(const Vec4& v)
	{
		std::cout << "(" << std::to_string(v.x) << " ; " << std::to_string(v.y) << " ; " << std::to_string(v.z) << " ; " << std::to_string(v.w) << ")" << std::endl;
	}

	inline void PrintMat(const Mat4x4& m)
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

	constexpr inline Mat4x4 MatrixMultiplication(const Mat4x4 a, const Mat4x4 b)
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