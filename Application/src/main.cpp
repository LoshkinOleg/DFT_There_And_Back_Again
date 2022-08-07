#include <Application.h>

#include <string>
#include <cassert>
#include <iostream>

#include <imgui.h>

struct Vec2
{
	float v0 = 0;
	float v1 = 0;
};

struct Vec3
{
	float v0 = 0;
	float v1 = 0;
	float v2 = 0;
};

struct Vec4
{
	float v0 = 0;
	float v1 = 0;
	float v2 = 0;
	float v3 = 1;

	inline bool operator==(const Vec4& other) const
	{
		return	v0 == other.v0 &&
				v1 == other.v1 &&
				v2 == other.v2 &&
				v3 == other.v3;
	}
};

struct Mat3x3
{
	float m00 = 1.0f; float m01 = 0.0f; float m02 = 0.0f;
	float m10 = 0.0f; float m11 = 1.0f; float m12 = 0.0f;
	float m20 = 0.0f; float m21 = 0.0f; float m22 = 1.0f;
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
		a.m00 * b.v0 + a.m01 * b.v1 + a.m02 * b.v2 + a.m03 * b.v3,
		a.m10 * b.v0 + a.m11 * b.v1 + a.m12 * b.v2 + a.m13 * b.v3,
		a.m20 * b.v0 + a.m21 * b.v1 + a.m22 * b.v2 + a.m23 * b.v3,
		a.m30 * b.v0 + a.m31 * b.v1 + a.m32 * b.v2 + a.m33 * b.v3
	};
}

Mat4x4 OrthogonalProjectionMatrix(const float near, const float far, const float right, const float left, const float bottom, const float top)
{
	return
	{
		2.0f/(right-left),	0.0f,				0.0f,				-(right+left)/(right-left),
		0.0f,				2.0f/(top-bottom),	0.0f,				-(top+bottom)/(top-bottom),
		0.0f,				0.0f,				-2.0f/(far-near),	-(far+near)/(far-near),
		0.0f,				0.0f,				0.0f,				1.0f
	};
}

inline float Sine(const float sampleIdx, const float sampleRate, const float frequency)
{
	assert(sampleIdx >= 0.0f && sampleRate > 0.0f && frequency > 0.0f && frequency < sampleRate / 2.0f && "Invalid Sine() parameters.");
	return std::sinf(sampleIdx * (2.0f * 3.14159265359f / sampleRate) * frequency);
}

void PrintVec(const Vec4& v)
{
	std::cout << "(" << std::to_string(v.v0) << " ; " << std::to_string(v.v1) << " ; " << std::to_string(v.v2) << " ; " << std::to_string(v.v3) << ")" << std::endl;
}

void PrintMat(const Mat4x4& m)
{
	std::cout << "(\n" << std::to_string(m.m00) << " ; " << std::to_string(m.m01) << " ; " << std::to_string(m.m02) << " ; " << std::to_string(m.m03) << ";\n";
	std::cout << std::to_string(m.m10) << " ; " << std::to_string(m.m11) << " ; " << std::to_string(m.m12) << " ; " << std::to_string(m.m13) << ";\n";
	std::cout << std::to_string(m.m20) << " ; " << std::to_string(m.m21) << " ; " << std::to_string(m.m22) << " ; " << std::to_string(m.m23) << ";\n";
	std::cout << std::to_string(m.m30) << " ; " << std::to_string(m.m31) << " ; " << std::to_string(m.m32) << " ; " << std::to_string(m.m33) << ";\n)" << std::endl;
}

// Taken from: https://www.geeksforgeeks.org/window-to-viewport-transformation-in-computer-graphics-with-implementation/
// Function for window to viewport transformation
Vec2 WindowtoViewport(float x_w, float y_w, float x_wmax,
					   float y_wmax, float x_wmin, float y_wmin,
					  float x_vmax, float y_vmax, float x_vmin,
					   float y_vmin)
{
	// point on viewport
	float x_v, y_v;

	// scaling factors for x coordinate and y coordinate
	float sx, sy;

	// calculating Sx and Sy
	sx = (float)(x_vmax - x_vmin) / (x_wmax - x_wmin);
	sy = (float)(y_vmax - y_vmin) / (y_wmax - y_wmin);

	// calculating the point on viewport
	x_v = x_vmin + (float)((x_w - x_wmin) * sx);
	y_v = y_vmin + (float)((y_w - y_wmin) * sy);

	return {x_v, y_v};
}

void MyApp::Application::OnStart()
{
	// sdl_.RegisterImguiCallback([&]()
	// {
	// 	ImGui::Begin("Hello, world!");
	// 
	// 	ImGui::End();
	// });

	// 441 Hz sine at 8000 Hz sample rate.
	static std::vector<float> sine(8000, 0.0f);
	for (size_t i = 0; i < 8000; i++)
	{
		sine[i] = Sine(i, 8000, 441);
	}

	// auto& sound = audioEngine_.CreateSound(sine);
	// sound.Play();

	// Pos xyz: (1.0f, 0.33f, -0.33f), rotation xyz: (90°, 0°, 0°)
	// const static Mat4x4 modelMatrix =
	// {
	// 	1.0f,	 0.0f,	  0.0f,		 2.0f,
	// 	0.0f,	 0.0f,	 -1.0f,		 0.33f,
	// 	0.0f,	 1.0f,	  0.0f,		-0.33f,
	// 	0.0f,	 0.0f,	  0.0f,		 1.0f
	// };
	// y = 1 = down; x = 1 = left
	const static Mat4x4 modelMatrix =
	{
		1.0f,	 -1.0f,	  0.0f,			0.0f,
		0.7f,	 0.0f,	  -0.7f,		0.0f,
		0.7f,	 0.0f,	  0.7f,			0.0f,
		0.0f,	 0.0f,	  0.0f,			1.0f
	};

	const static Mat4x4 projMat = OrthogonalProjectionMatrix(0.01f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);

	static Mat4x4 viewMatrix{};
	
	sdl_.RegisterRenderCallback([&]()
	{
		const auto IsWithinBounds = [](const Vec4 p)->bool
		{
			return	p.v0 >= 0.0f && p.v0 <= 1.0f &&
					p.v1 >= 0.0f && p.v1 <= 1.0f &&
					p.v2 == 0.0f &&
					p.v3 == 1.0f;
		};

		for (size_t i = 0; i < 8000; i++)
		{
			// Vec4 point = {std::cosf(sine[i]), std::sinf(sine[i]), i / 8000.0f, 1.0f};
			// // PrintVec(point);
			// // PrintMat(modelMatrix);
			// point = MatrixVectorMultiplication(modelMatrix, point);
			// // PrintVec(point);
			// point = MatrixVectorMultiplication(viewMatrix, point);
			// point = MatrixVectorMultiplication(projMat, point);
			// Vec3 normalizedDeviceCoord = {point.v0 / point.v3, point.v1 / point.v3, point.v2 / point.v3};
			// Vec2 screenCoord = WindowtoViewport(normalizedDeviceCoord.v0, normalizedDeviceCoord.v1, 2.0f, 2.0f, -2.0f, -2.0f, 1.0f, 1.0f, 0.0f, 0.0f);
			
			// sdl_.RenderPoint(screenCoord.v0, screenCoord.v1);


		}
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