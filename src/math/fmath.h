#pragma once

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct {
    float a; float b;
    float c; float d;
} mat2;

typedef struct {
    float a; float b; float c;
    float d; float e; float f;
    float g; float h; float i;
} mat3;

typedef struct {
    float a; float b; float c; float d;
    float e; float f; float g; float h;
    float i; float j; float k; float l;
    float m; float n; float o; float p;
} mat4;

mat2 mat2mat2mul(mat2 a, mat2 b);
vec2 mat2vec2mul(mat2 a, vec2 v);
mat2 mat2scale(mat2 a, float s);
mat2 mat2mat2add(mat2 a, mat2 b);

mat3 mat3mat3mul(mat3 a, mat3 b);
vec3 mat3vec3mul(mat3 a, vec3 v);
mat3 mat3scale(mat3 a, float s);
vec3 mat3vec3add(mat3 a, vec3 v);

mat4 mat4mat4mul(mat4 a, mat4 b);
vec4 mat4vec4mul(mat4 a, vec4 v);
mat4 mat4scale(mat4 a, float s);
mat4 mat4mat4add(mat4 a, mat4 b);

mat3 mat3rot(float rad);

vec2 vec2scale(vec2 v, float s);
vec3 vec3scale(vec3 v, float s);
vec4 vec4scale(vec4 v, float s);

vec2 vec2add(vec2 a, vec2 b);
vec3 vec3add(vec3 a, vec3 b);
vec4 vec4add(vec4 a, vec4 b);

vec2 vec2sub(vec2 a, vec2 b);
vec3 vec3sub(vec3 a, vec3 b);
vec4 vec4sub(vec4 a, vec4 b);

float vec3norm(vec3 v);

vec3 vec3dir(vec3 v);

float vec2dot(vec2 a, vec2 b);
float vec3dot(vec3 a, vec3 b);
float vec4dot(vec4 a, vec4 b);

vec3 vec3cross(vec3 a, vec3 b);
vec4 vec4cross(vec4 a, vec4 b);

float mat2det(mat2 a);
float mat3det(mat3 a);
float mat4det(mat4 a);

mat2 mat2inv(mat2 a);
mat3 mat3inv(mat3 a);
mat4 mat4inv(mat4 a);

#define vec2dc(v) (v).x, (v).y
#define vec3dc(v) (v).x, (v).y, (v).z
#define vec4dc(v) (v).x, (v).y, (v).z, (v).w

#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

