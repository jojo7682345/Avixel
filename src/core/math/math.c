#include "math.h"
#include <stdlib.h>
#include <memory.h>

void matrixSetIdentity(Mat4f* mat) {
	memset(mat->m, 0, sizeof(Mat4f));
	mat->m00 = 1;
	mat->m11 = 1;
	mat->m22 = 1;
	mat->m33 = 1;
}

void matrixMultiply(Mat4f* restrict a, Mat4f* restrict b, Mat4f* restrict result) {
	result->m00 = a->m00 * b->m00 + a->m01 * b->m10 + a->m02 * b->m20 + a->m03 * b->m30;
	result->m01 = a->m00 * b->m01 + a->m01 * b->m11 + a->m02 * b->m21 + a->m03 * b->m31;
	result->m02 = a->m00 * b->m02 + a->m01 * b->m12 + a->m02 * b->m22 + a->m03 * b->m32;
	result->m03 = a->m00 * b->m03 + a->m01 * b->m13 + a->m02 * b->m23 + a->m03 * b->m33;

	result->m10 = a->m10 * b->m00 + a->m11 * b->m10 + a->m12 * b->m20 + a->m13 * b->m30;
	result->m11 = a->m10 * b->m01 + a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31;
	result->m12 = a->m10 * b->m02 + a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32;
	result->m13 = a->m10 * b->m03 + a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33;

	result->m20 = a->m20 * b->m00 + a->m21 * b->m10 + a->m22 * b->m20 + a->m23 * b->m30;
	result->m21 = a->m20 * b->m01 + a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31;
	result->m22 = a->m20 * b->m02 + a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32;
	result->m23 = a->m20 * b->m03 + a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33;

	result->m30 = a->m30 * b->m00 + a->m31 * b->m10 + a->m32 * b->m20 + a->m33 * b->m30;
	result->m31 = a->m30 * b->m01 + a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31;
	result->m32 = a->m30 * b->m02 + a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32;
	result->m33 = a->m30 * b->m03 + a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33;
}

void vectorMultiply(Mat4f* restrict mat, Vec4f* restrict vec, Vec4f* restrict res) {
	res->x = mat->m00 * vec->x + mat->m01 * vec->y + mat->m02 * vec->z + mat->m03 * vec->w;
	res->y = mat->m10 * vec->x + mat->m11 * vec->y + mat->m12 * vec->z + mat->m13 * vec->w;
	res->z = mat->m20 * vec->x + mat->m21 * vec->y + mat->m22 * vec->z + mat->m23 * vec->w;
	res->w = mat->m30 * vec->x + mat->m31 * vec->y + mat->m32 * vec->z + mat->m33 * vec->w;
}

float vectorDot4f(Vec4f* restrict a, Vec4f* restrict b) {
	return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

float vectorDot3f(Vec3f* restrict a, Vec3f* restrict b) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

float vectorDot2f(Vec2f* restrict a, Vec2f* restrict b) {
	return a->x * b->x + a->y * b->y;
}
