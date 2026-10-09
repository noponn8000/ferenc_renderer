#pragma once

typedef struct {
	int x;
	int y;
} Vector2i;

typedef struct {
	float x;
	float y;
} Vector2f;

Vector2i weightedSum(Vector2i a, Vector2f b, float weight);

// Collisions
bool pointInRect(Vector2i rPosition, Vector2i rSize, Vector2i pPosition);
