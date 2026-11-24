#include <stdint.h>
#include <math/fmath.h>

typedef struct {
	vec3* vertices;

	uint32_t* tris;

	vec3* normals;

	vec2* uvs;

	uint32_t* texture;
} Mesh;
