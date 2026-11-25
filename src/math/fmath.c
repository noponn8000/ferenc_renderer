#include "fmath.h"
#include <stdint.h>
#include <math.h>

vec2 mat2vec2mul(mat2 a, vec2 v) {
    return (vec2) {
        v.x * a.a + v.y * a.b,
        v.x * a.c + v.y * a.d
    };
}

vec3 mat3vec3mul(mat3 a, vec3 v) {
    return (vec3) {
        v.x * a.a + v.y * a.b + v.z * a.c,
        v.x * a.d + v.y * a.e + v.z * a.f,
        v.x * a.g + v.y * a.h + v.z * a.i,
    };
}

vec4 mat4vec4mul(mat4 a, vec4 v) {
    return (vec4) {
        v.x * a.a + v.y * a.b + v.z * a.c + v.w * a.d,
        v.x * a.e + v.y * a.f + v.z * a.g + v.w * a.h,
        v.x * a.i + v.y * a.j + v.z * a.k + v.w * a.l,
        v.x * a.m + v.y * a.n + v.z * a.o + v.w * a.p,
    };
}

mat2 mat2mat2mul(mat2 a, mat2 b) {
    mat2 r;

    r.a = a.a * b.a + a.b * b.c;
    r.b = a.a * b.b + a.b * b.d;

    r.c = a.c * b.a + a.d * b.c;
    r.d = a.c * b.b + a.d * b.d;

    return r;
}

mat3 mat3mat3mul(mat3 a, mat3 b) {
    mat3 r;

    r.a = a.a * b.a + a.b * b.d + a.c * b.g;
    r.b = a.a * b.b + a.b * b.e + a.c * b.h;
    r.c = a.a * b.c + a.b * b.f + a.c * b.i;

    r.d = a.d * b.a + a.e * b.d + a.f * b.g;
    r.e = a.d * b.b + a.e * b.e + a.f * b.h;
    r.f = a.d * b.c + a.e * b.f + a.f * b.i;

    r.g = a.g * b.a + a.h * b.d + a.i * b.g;
    r.h = a.g * b.b + a.h * b.e + a.i * b.h;
    r.i = a.g * b.c + a.h * b.f + a.i * b.i;

    return r;
}

mat4 mat4mat4mul(mat4 a, mat4 b) {
    mat4 r;

    r.a = a.a*b.a + a.b*b.e + a.c*b.i  + a.d*b.m;
    r.b = a.a*b.b + a.b*b.f + a.c*b.j  + a.d*b.n;
    r.c = a.a*b.c + a.b*b.g + a.c*b.k  + a.d*b.o;
    r.d = a.a*b.d + a.b*b.h + a.c*b.l  + a.d*b.p;

    r.e = a.e*b.a + a.f*b.e + a.g*b.i  + a.h*b.m;
    r.f = a.e*b.b + a.f*b.f + a.g*b.j  + a.h*b.n;
    r.g = a.e*b.c + a.f*b.g + a.g*b.k  + a.h*b.o;
    r.h = a.e*b.d + a.f*b.h + a.g*b.l  + a.h*b.p;

    r.i = a.i*b.a + a.j*b.e + a.k*b.i  + a.l*b.m;
    r.j = a.i*b.b + a.j*b.f + a.k*b.j  + a.l*b.n;
    r.k = a.i*b.c + a.j*b.g + a.k*b.k  + a.l*b.o;
    r.l = a.i*b.d + a.j*b.h + a.k*b.l  + a.l*b.p;

    r.m = a.m*b.a + a.n*b.e + a.o*b.i  + a.p*b.m;
    r.n = a.m*b.b + a.n*b.f + a.o*b.j  + a.p*b.n;
    r.o = a.m*b.c + a.n*b.g + a.o*b.k  + a.p*b.o;
    r.p = a.m*b.d + a.n*b.h + a.o*b.l  + a.p*b.p;

    return r;
}

float vec3dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3norm(vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 vec3dir(vec3 v) {
    return vec3scale(v, 1.0/vec3norm(v));
}

vec3 vec3scale(vec3 v, float s) {
    return (vec3) {
        v.x * s,
        v.y * s,
        v.z * s
    };
}

vec3 vec3sub(vec3 a, vec3 b) {
    return (vec3) {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

vec3 vec3add(vec3 a, vec3 b) {
    return (vec3) {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

vec4 vec4scale(vec4 v, float s) {
    return (vec4) {
        v.x * s,
        v.y * s,
        v.z * s,
        v.w * s
    };
}

vec4 vec4add(vec4 a, vec4 b) {
    return (vec4) {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        a.w + b.w
    };
}

vec4 vec4sub(vec4 a, vec4 b) {
    return (vec4) {
        b.x - a.x,
        b.y - a.y,
        b.z - a.z,
        b.w - a.w
    };
}

vec3 vec3cross(vec3 a, vec3 b) {
	return (vec3) {
    	a.y * b.z - a.z * b.y,
    	a.z * b.x - a.x * b.z,
    	a.x * b.y - a.y * b.x
    };
}

mat4 mat4homrotx(float theta) {
	return (mat4) {
         cos(theta), 0, sin(theta), 0,
                  0, 1,          0, 0,
        -sin(theta), 0, cos(theta), 0,
                  0, 0,          0, 1
    };
}

mat4 mat4homroty(float theta) {
    return (mat4) {
        1, 0, 0, 0,
        0, cos(theta), -sin(theta), 0,
        0, sin(theta), cos(theta), 0,
        0, 0, 0, 1
    };
}

mat4 mat4homrotz(float theta) {
	return (mat4) {
		cos(theta), -sin(theta), 0, 0,
		sin(theta), cos(theta), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

mat4 mat4trans(vec3 trans) {
    return (mat4) {
        1, 0, 0, trans.x,
        0, 1, 0, trans.y,
        0, 0, 1, trans.z,
        0, 0, 0, 1
    };
}

mat4 mat4viewport(uint16_t canvas_w, uint16_t canvas_h) {
    float nx = (float) canvas_w;
    float ny = (float) canvas_h;
    return (mat4) {
    	nx / 2,    0, 0, (nx - 1) / 2,
    	     0, -ny/2, 0, (ny - 1) / 2,
    	     0,    0, 1, 			0,
    	     0,    0, 0, 			1
    };
}

mat4 mat4orthographic(float l, float r, float b, float t, float n, float f) {
	return (mat4) {
    	2/(r - l),       0,       0, -(r+l)/(r-l),
    	        0, 2/(t-b),       0, -(t+b)/(t-b),
    	        0,       0, 2/(n-f), -(n+f)/(n-f),
    	        0,       0,       0, 			1
	};
}

mat4 mat4camera(vec3 e, vec3 g, vec3 t) {
	vec3 w = vec3scale(g, -1.0); 
	vec3 u = vec3cross(t, w);
	vec3 v = vec3cross(w, u);

	return (mat4) {
		u.x, u.y, u.z, -vec3dot(u, e),
		v.x, v.y, v.z, -vec3dot(v, e),
		w.x, w.y, w.z, -vec3dot(w, e),
		0,   0,   0,   1
	};
}

mat4 mat4perspective(float n, float f) {
    return (mat4) {
    	n, 0, 0, 0,
    	0, n, 0, 0,
    	0, 0, n + f, -f * n,
    	0, 0, 1, 0
    };
}

float clampf(float x, float min, float max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

float lambertian(vec3 lightDirection, vec3 normal) {
    return clampf(vec3dot(lightDirection, normal), 0.0f, 1.0f);
}

float specular(vec3 viewDirection, vec3 lightDirection, vec3 normal, float shininess) {
    vec3 halfway = vec3dir(vec3add(lightDirection, viewDirection));
    float hdotl = clampf(vec3dot(normal, halfway), 0.0f, 1.0f);

    return powf(hdotl, shininess);
}

vec3 blinn_phong(vec3 fragmentPosition, vec3 lightPosition, vec3 eyePosition, vec3 normal, float shininess, float ambient, vec3 albedo, vec3 highlight) {
    vec3 viewDirection = vec3dir(vec3sub(eyePosition, fragmentPosition));
    vec3 lightDirection = vec3dir(vec3sub(lightPosition, fragmentPosition));

    float l = ambient + (1.0f - ambient) * lambertian(lightDirection, normal);
    float s = specular(viewDirection, lightDirection, normal, shininess);

	vec3 color = vec3add(
    	vec3scale(albedo, l),
    	vec3scale(highlight, s)
    );
	color.x = clampf(color.x, 0.0f, 1.0f);
	color.y = clampf(color.y, 0.0f, 1.0f);
	color.z = clampf(color.z, 0.0f, 1.0f);

	return color;
}
