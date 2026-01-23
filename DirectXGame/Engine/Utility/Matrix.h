#pragma once
#include "Operator.h"

struct Matrix4x4 final {

	static Matrix4x4 Identity() {
		return { 
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1 };
	}

	float m[4][4];

	Matrix4x4 Inverse() const {
		Matrix4x4 mat = *this;

		float d = mat.m[0][0] * mat.m[1][1] * mat.m[2][2] * mat.m[3][3]
			+ mat.m[0][0] * mat.m[1][2] * mat.m[2][3] * mat.m[3][1]
			+ mat.m[0][0] * mat.m[1][3] * mat.m[2][1] * mat.m[3][2]
			- mat.m[0][0] * mat.m[1][3] * mat.m[2][2] * mat.m[3][1]
			- mat.m[0][0] * mat.m[1][2] * mat.m[2][1] * mat.m[3][3]
			- mat.m[0][0] * mat.m[1][1] * mat.m[2][3] * mat.m[3][2]
			- mat.m[0][1] * mat.m[1][0] * mat.m[2][2] * mat.m[3][3]
			- mat.m[0][2] * mat.m[1][0] * mat.m[2][3] * mat.m[3][1]
			- mat.m[0][3] * mat.m[1][0] * mat.m[2][1] * mat.m[3][2]
			+ mat.m[0][3] * mat.m[1][0] * mat.m[2][2] * mat.m[3][1]
			+ mat.m[0][2] * mat.m[1][0] * mat.m[2][1] * mat.m[3][3]
			+ mat.m[0][1] * mat.m[1][0] * mat.m[2][3] * mat.m[3][2]
			+ mat.m[0][1] * mat.m[1][2] * mat.m[2][0] * mat.m[3][3]
			+ mat.m[0][2] * mat.m[1][3] * mat.m[2][0] * mat.m[3][1]
			+ mat.m[0][3] * mat.m[1][1] * mat.m[2][0] * mat.m[3][2]
			- mat.m[0][3] * mat.m[1][2] * mat.m[2][0] * mat.m[3][1]
			- mat.m[0][2] * mat.m[1][1] * mat.m[2][0] * mat.m[3][3]
			- mat.m[0][1] * mat.m[1][3] * mat.m[2][0] * mat.m[3][2]
			- mat.m[0][1] * mat.m[1][2] * mat.m[2][3] * mat.m[3][0]
			- mat.m[0][2] * mat.m[1][3] * mat.m[2][1] * mat.m[3][0]
			- mat.m[0][3] * mat.m[1][1] * mat.m[2][2] * mat.m[3][0]
			+ mat.m[0][3] * mat.m[1][2] * mat.m[2][1] * mat.m[3][0]
			+ mat.m[0][2] * mat.m[1][1] * mat.m[2][3] * mat.m[3][0]
			+ mat.m[0][1] * mat.m[1][3] * mat.m[2][2] * mat.m[3][0];

		return {
			(mat.m[1][1] * mat.m[2][2] * mat.m[3][3] + mat.m[1][2] * mat.m[2][3] * mat.m[3][1] + mat.m[1][3] * mat.m[2][1] * mat.m[3][2]
				- mat.m[1][3] * mat.m[2][2] * mat.m[3][1] - mat.m[1][2] * mat.m[2][1] * mat.m[3][3] - mat.m[1][1] * mat.m[2][3] * mat.m[3][2]) / d,
			-(mat.m[0][1] * mat.m[2][2] * mat.m[3][3] + mat.m[0][2] * mat.m[2][3] * mat.m[3][1] + mat.m[0][3] * mat.m[2][1] * mat.m[3][2]
				- mat.m[0][3] * mat.m[2][2] * mat.m[3][1] - mat.m[0][2] * mat.m[2][1] * mat.m[3][3] - mat.m[0][1] * mat.m[2][3] * mat.m[3][2]) / d,
			(mat.m[0][1] * mat.m[1][2] * mat.m[3][3] + mat.m[0][2] * mat.m[1][3] * mat.m[3][1] + mat.m[0][3] * mat.m[1][1] * mat.m[3][2]
				- mat.m[0][3] * mat.m[1][2] * mat.m[3][1] - mat.m[0][2] * mat.m[1][1] * mat.m[3][3] - mat.m[0][1] * mat.m[1][3] * mat.m[3][2]) / d,
			-(mat.m[0][1] * mat.m[1][2] * mat.m[2][3] + mat.m[0][2] * mat.m[1][3] * mat.m[2][1] + mat.m[0][3] * mat.m[1][1] * mat.m[2][2]
				- mat.m[0][3] * mat.m[1][2] * mat.m[2][1] - mat.m[0][2] * mat.m[1][1] * mat.m[2][3] - mat.m[0][1] * mat.m[1][3] * mat.m[2][2]) / d,

			-(mat.m[1][0] * mat.m[2][2] * mat.m[3][3] + mat.m[1][2] * mat.m[2][3] * mat.m[3][0] + mat.m[1][3] * mat.m[2][0] * mat.m[3][2]
				- mat.m[1][3] * mat.m[2][2] * mat.m[3][0] - mat.m[1][2] * mat.m[2][0] * mat.m[3][3] - mat.m[1][0] * mat.m[2][3] * mat.m[3][2]) / d,
			(mat.m[0][0] * mat.m[2][2] * mat.m[3][3] + mat.m[0][2] * mat.m[2][3] * mat.m[3][0] + mat.m[0][3] * mat.m[2][0] * mat.m[3][2]
				- mat.m[0][3] * mat.m[2][2] * mat.m[3][0] - mat.m[0][2] * mat.m[2][0] * mat.m[3][3] - mat.m[0][0] * mat.m[2][3] * mat.m[3][2]) / d,
			-(mat.m[0][0] * mat.m[1][2] * mat.m[3][3] + mat.m[0][2] * mat.m[1][3] * mat.m[3][0] + mat.m[0][3] * mat.m[1][0] * mat.m[3][2]
				- mat.m[0][3] * mat.m[1][2] * mat.m[3][0] - mat.m[0][2] * mat.m[1][0] * mat.m[3][3] - mat.m[0][0] * mat.m[1][3] * mat.m[3][2]) / d,
			(mat.m[0][0] * mat.m[1][2] * mat.m[2][3] + mat.m[0][2] * mat.m[1][3] * mat.m[2][0] + mat.m[0][3] * mat.m[1][0] * mat.m[2][2]
				- mat.m[0][3] * mat.m[1][2] * mat.m[2][0] - mat.m[0][2] * mat.m[1][0] * mat.m[2][3] - mat.m[0][0] * mat.m[1][3] * mat.m[2][2]) / d,

			(mat.m[1][0] * mat.m[2][1] * mat.m[3][3] + mat.m[1][1] * mat.m[2][3] * mat.m[3][0] + mat.m[1][3] * mat.m[2][0] * mat.m[3][1]
				- mat.m[1][3] * mat.m[2][1] * mat.m[3][0] - mat.m[1][1] * mat.m[2][0] * mat.m[3][3] - mat.m[1][0] * mat.m[2][3] * mat.m[3][1]) / d,
			-(mat.m[0][0] * mat.m[2][1] * mat.m[3][3] + mat.m[0][1] * mat.m[2][3] * mat.m[3][0] + mat.m[0][3] * mat.m[2][0] * mat.m[3][1]
				- mat.m[0][3] * mat.m[2][1] * mat.m[3][0] - mat.m[0][1] * mat.m[2][0] * mat.m[3][3] - mat.m[0][0] * mat.m[2][3] * mat.m[3][1]) / d,
			(mat.m[0][0] * mat.m[1][1] * mat.m[3][3] + mat.m[0][1] * mat.m[1][3] * mat.m[3][0] + mat.m[0][3] * mat.m[1][0] * mat.m[3][1]
				- mat.m[0][3] * mat.m[1][1] * mat.m[3][0] - mat.m[0][1] * mat.m[1][0] * mat.m[3][3] - mat.m[0][0] * mat.m[1][3] * mat.m[3][1]) / d,
			-(mat.m[0][0] * mat.m[1][1] * mat.m[2][3] + mat.m[0][1] * mat.m[1][3] * mat.m[2][0] + mat.m[0][3] * mat.m[1][0] * mat.m[2][1]
				- mat.m[0][3] * mat.m[1][1] * mat.m[2][0] - mat.m[0][1] * mat.m[1][0] * mat.m[2][3] - mat.m[0][0] * mat.m[1][3] * mat.m[2][1]) / d,

			-(mat.m[1][0] * mat.m[2][1] * mat.m[3][2] + mat.m[1][1] * mat.m[2][2] * mat.m[3][0] + mat.m[1][2] * mat.m[2][0] * mat.m[3][1]
				- mat.m[1][2] * mat.m[2][1] * mat.m[3][0] - mat.m[1][1] * mat.m[2][0] * mat.m[3][2] - mat.m[1][0] * mat.m[2][2] * mat.m[3][1]) / d,
			(mat.m[0][0] * mat.m[2][1] * mat.m[3][2] + mat.m[0][1] * mat.m[2][2] * mat.m[3][0] + mat.m[0][2] * mat.m[2][0] * mat.m[3][1]
				- mat.m[0][2] * mat.m[2][1] * mat.m[3][0] - mat.m[0][1] * mat.m[2][0] * mat.m[3][2] - mat.m[0][0] * mat.m[2][2] * mat.m[3][1]) / d,
			-(mat.m[0][0] * mat.m[1][1] * mat.m[3][2] + mat.m[0][1] * mat.m[1][2] * mat.m[3][0] + mat.m[0][2] * mat.m[1][0] * mat.m[3][1]
				- mat.m[0][2] * mat.m[1][1] * mat.m[3][0] - mat.m[0][1] * mat.m[1][0] * mat.m[3][2] - mat.m[0][0] * mat.m[1][2] * mat.m[3][1]) / d,
			(mat.m[0][0] * mat.m[1][1] * mat.m[2][2] + mat.m[0][1] * mat.m[1][2] * mat.m[2][0] + mat.m[0][2] * mat.m[1][0] * mat.m[2][1]
				- mat.m[0][2] * mat.m[1][1] * mat.m[2][0] - mat.m[0][1] * mat.m[1][0] * mat.m[2][2] - mat.m[0][0] * mat.m[1][2] * mat.m[2][1]) / d
		};

		return mat;
	}
	Matrix4x4 Transpose() const {
		Matrix4x4 mat = *this;
		Matrix4x4 ans;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				ans.m[i][j] = mat.m[j][i];
			}
		}
		return ans;
	}
};

struct Matrix3x3 {

	static Matrix4x4 Identity() {
		return {
			1,0,0,
			0,1,0,
			0,0,1 };
	}

	float m[3][3];

	Matrix3x3 Inverse() const {
		Matrix3x3 mat = *this;

		float d = mat.m[0][0] * mat.m[1][1] * mat.m[2][2] + mat.m[0][1] * mat.m[1][2] * mat.m[2][0] + mat.m[0][2] * mat.m[1][0] * mat.m[2][1] -
			mat.m[0][2] * mat.m[1][1] * mat.m[2][0] - mat.m[0][1] * mat.m[1][0] * mat.m[2][2] - mat.m[0][0] * mat.m[1][2] * mat.m[2][1];

		Matrix3x3 ans = { mat.m[1][1] * mat.m[2][2] - mat.m[1][2] * mat.m[2][1] / d,
			-mat.m[0][1] * mat.m[2][2] + mat.m[0][2] * mat.m[2][1] / d,
			mat.m[0][1] * mat.m[1][2] - mat.m[0][2] * mat.m[1][1] / d,

			-mat.m[1][0] * mat.m[2][2] + mat.m[1][2] * mat.m[2][0] / d,
			mat.m[0][0] * mat.m[2][2] - mat.m[0][2] * mat.m[2][0] / d,
			-mat.m[0][0] * mat.m[1][2] + mat.m[0][2] * mat.m[1][0] / d,

			mat.m[1][0] * mat.m[2][1] - mat.m[1][1] * mat.m[2][0] / d,
			-mat.m[0][0] * mat.m[2][1] + mat.m[0][1] * mat.m[2][0] / d,
			mat.m[0][0] * mat.m[1][1] - mat.m[0][1] * mat.m[1][0] / d
		};

		return mat;
	}
};
