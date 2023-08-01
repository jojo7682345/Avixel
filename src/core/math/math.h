#pragma once

typedef struct Vec2f {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
} Vec2f;

typedef struct Vec3f {
	union {
		Vec2f xy;
		Vec2f rg;
		struct {
			struct Vec2f;
			union {
				float z;
				float b;
			};
		};
	};
} Vec3f;

typedef struct Vec4f {
	union {
		Vec3f xyz;
		Vec3f rgb;
		struct {
			struct Vec3f;
			union {
				float w;
				float a;
			};
		};
	};
} Vec4f;

typedef struct Mat4f {
	union {
		float m[4][4];
		struct {
			float m00;
			float m01;
			float m02;
			float m03;

			float m10;
			float m11;
			float m12;
			float m13;

			float m20;
			float m21;
			float m22;
			float m23;

			float m30;
			float m31;
			float m32;
			float m33;
		};
	};
}Mat4f;

void matrixSetIdentity(Mat4f* mat);
void matrixMultiply(Mat4f* restrict a, Mat4f* restrict b, Mat4f* restrict result);

void vectorMultiply(Mat4f* restrict mat, Vec4f* restrict vec, Vec4f* restrict res);
float vectorDot4f(Vec4f* restrict a, Vec4f*  restrict b);
float vectorDot3f(Vec3f* restrict a, Vec3f* restrict b);
float vectorDot2f(Vec2f* restrict a, Vec2f* restrict b);

