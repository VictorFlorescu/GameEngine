#pragma once 
#include "raylib.h"
#include "raymath.h"

struct Transform
{
	Vector3 position = { 0.0f, 0.0f, 0.0f };
	Vector3 rotation = { 0.0f, 0.0f, 0.0f }; // Euler angles in degrees
	Vector3 scale = { 1.0f, 1.0f,1.0f };

	// Convenience helpers

	Matrix GetMatrix() const
	{
		Matrix t = MatrixTranslate(position.x, position.y, position.z);
		Matrix rx = MatrixRotateX(DEG2RAD * rotation.x);
		Matrix ry = MatrixRotateY(DEG2RAD * rotation.y);
		Matrix rz = MatrixRotateZ(DEG2RAD * rotation.z);
		Matrix s = MatrixScale(scale.x, scale.y, scale.z);

		return MatrixMultiply(MatrixMultiply(s, rz), MatrixMultiply(rx, ry), t);
	}

	Vector3 Forward() const
	{
		return Vector3Normalize
		(
			{
				sinf(DEG2RAD * rotation.y) * cosf(DEG2RAD * rotation.x),
				-sinf(DEG2RAD * rotation.x),
				-cosf(DEG2RAD * rotation.y) * cosf(DEG2RAD * rotation.x)
			}
		);
	}

	Vector3 Right() const
	{
		return Vector3Normalize(Vector3CrossProduct(Forward(), { 0.f, 1.f,0.f }));
	}
};