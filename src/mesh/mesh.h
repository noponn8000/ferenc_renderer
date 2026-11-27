#include <stdint.h>
#include <math/fmath.h>

typedef struct {
	vec3* vertices;

	vec3* normals;

	vec2* uvs;

	uint32_t* tris;

	uint32_t* tri_normals;

	uint32_t* tri_uvs;

	uint32_t* texture;
} Mesh;

typedef struct {
    vec3 albedo;
    vec3 specular;
    float shininess;
    float ambient;
} BlinnPhongMaterial;
