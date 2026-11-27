#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <engine/engine.h>
#include <renderer/render.h>
#include <renderer/colors.h>
#include <math/fmath.h>
#include <obj/obj.h>
#include <utils/pbm_reader.h>
#include <utils/array.h>

// SDL state
SDL_Window   *win;
SDL_Renderer *ren;
SDL_Texture  *scrtex;
// framebuffer
uint32_t     *fb;
// Depth buffer
float		 *db;
// Shadow map
float 		 *sb;
SDL_AudioDeviceID dev;
bool running = true;

// Data
uint32_t* cube_tex;
uint16_t cube_tex_width;
uint16_t cube_tex_height;

#define N_MESHES 3
#define NVERTEXDATA 6
Mesh meshes[N_MESHES];
BlinnPhongMaterial materials[N_MESHES] = {
	{
    	// Albedo
    	{ 0.38, 0.57, 0.92 },
    	// Specular color
    	{ 0.9, 0.9, 0.9 },
    	// Shininess (phong exponent)
    	8.0,
    	// Ambient
    	0.1
    	
	},
	{
    	{ 0.32, 0.19, 0.72 },
    	{ 0.9, 0.9, 0.9 },
    	2.0,
    	0.2
	},
	{
    	{ 0.71, 0.18, 0.18 },
    	{ 0.9, 0.9, 0.9 },
    	16.0,
    	0.1
	}
};

mat4 meshTransforms[N_MESHES] = {
    {
		0.5, 0, 0, 2,
		0, 0.5, 0, 0,
		0, 0, 0.5, 2,
		0, 0, 0,   1
    },
    {
		8.0, 0, 0,  0.0,
		0, 2.0, 0, -3.0,
		0, 0, 8.0,  0.0,
		0, 0, 0,    1.0
    },
    {
		2.0, 0, 0, -2,
		0, 2.0, 0, 0,
		0, 0, 2.0, -2,
		0, 0, 0,   1
    },
};

// For rotating objects periodically
float theta;

// ---- Camera
// Perspective vs. ortohgraphic
bool perspective = true;
bool drawWireframe = false;
bool drawNormals = false;
bool drawShadows = false;

float lightRadius;

// Where the camera is looking at
vec3 target = { 0 };
// Distance from target
float distance = 12.0f;
// Rotation
float pitch = 0.0f;
float yaw = 0.0f;

float mouseXPrev = -1.0f;
float mouseYPrev = -1.0f;

const float MOUSE_SENSITIVITY = 3.5;
const float MAX_PITCH = 1.2;
const float NORMAL_LENGTH = 0.5;

// Orthographic view volume
// Assuming the view volume looks at the negative z-axis, with the up-direction being the positive y-axis
float l = -0.8f;
float r =  0.8f;
float b = -0.8f;
float t =  0.8f;
float n =  -1.0f;
float f = -100.0f;

// Scale factor canvas size -> window size
#define WINDOW_SCALE_FACTOR 1
#define CANVAS_HEIGHT 1024
#define CANVAS_WIDTH 1024

#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

typedef struct {
    mat4 lightProjection;
    const float* shadowMap;
    float shadowBias;
	vec3 albedo;
	vec3 highlight;
	vec3 lightDirection;
	vec3 eyePosition;
	float shininess;
	float ambient;
} BlinnPhongUniforms;

uint32_t blinnPhongShader(const void* uniforms, const float* in) {
    const BlinnPhongUniforms* u = (const BlinnPhongUniforms*) uniforms;

    vec3 worldPos = { in[0], in[1], in[2] };
    vec3 normal   = vec3dir((vec3){ in[3], in[4], in[5] });

    float ndotl = vec3dot(normal, u->lightDirection);

    float lit = drawShadows ? 0.0f : 1.0f;

    if (drawShadows && ndotl > 0.0f) {
        vec4 lp = mat4vec4mul(u->lightProjection, (vec4){ worldPos.x, worldPos.y, worldPos.z, 1.0f });

        float bias = fmaxf(u->shadowBias * (1.0f - ndotl), u->shadowBias * 0.1f);
        float fragDepth = lp.z + bias;
                float occlusionDepth = FR_SampleDepth(u->shadowMap, SHADOW_WIDTH, SHADOW_HEIGHT, floorf(lp.x), floorf(lp.y));
                lit = (fragDepth < occlusionDepth) ? 0.0f : 1.0f;
    }

    vec3 full = blinn_phong(worldPos, u->lightDirection, u->eyePosition,
                               normal, u->shininess, u->ambient, u->albedo, u->highlight);
    vec3 ambient = vec3scale(u->albedo, u->ambient);
    vec3 rgb = vec3add(ambient, vec3scale(vec3sub(full, ambient), lit));

    return FR_RGBAToAGBR8888(rgb.x * 255, rgb.y * 255, rgb.z * 255, 255);
}

float MyPreFrame(void* self) {
    memset(fb, 0xFF282828, CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint32_t)); // Clear framebuffer

	const float cospitch = cosf(pitch);
	const float sinpitch = sinf(pitch);
	const float cosyaw = cosf(yaw);
	const float sinyaw = sinf(yaw);
	// Eye position
	const vec3 e = {
    	target.x + distance * cospitch * sinyaw,
    	target.y + distance * sinpitch,
    	target.z + distance * cospitch * cosyaw
	};

	// Gaze direction
	const vec3 g = vec3dir(vec3sub(target, e));

	// View-up vector
	const vec3 up = {
    	0, 1, 0
	};

	// Camera matrix
	const mat4 m_cam = mat4camera(e, g, up);
	// Viewport matrix
	const mat4 m_vp = mat4viewport(CANVAS_WIDTH, CANVAS_HEIGHT);
	// Orthographic projection matrix
	const mat4 m_orth = mat4orthographic(l, r, b, t, n, f);
	// Perspective matrix
	const mat4 m_perspective = mat4perspective(n, f);

	mat4 m;
	if (perspective) {
    	// If perspective is enabled, add that to the matrix pipeline before the orthographic matrix
    	m = mat4mat4mul(m_perspective, m_cam);
	} else {
		m = m_cam;
	}
	m = mat4mat4mul(m_orth, m);
	m = mat4mat4mul(m_vp, m);

	const vec3 lightDirection = vec3dir((vec3) { 0.0f, 1.0f, 0.0f });
	const vec3 lightPosition = vec3scale(lightDirection, lightRadius);
	// Find a point on the near plane by going -n units away from the camera position in the direction of gaze
	const vec3 nearPlanePoint = vec3add(e, vec3scale(g, -n));

	// Clear depth buffer
	for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) {
    	db[i] = f;
    	sb[i] = f;
	}

	// Construct the projection matrix that renders depth from the light's POV
	mat4 lightProjection = mat4camera(lightPosition, vec3scale(lightDirection, -1.0f), (vec3) {0, 0, 1});
	lightProjection = mat4mat4mul(
    	mat4orthographic(-lightRadius, lightRadius, -lightRadius, lightRadius, 2.0f * lightRadius, 0.0f),
    	lightProjection);
	lightProjection = mat4mat4mul(mat4viewport(SHADOW_WIDTH, SHADOW_HEIGHT), lightProjection);

	// Struct that holds uniforms to pass into fragment shader
	BlinnPhongUniforms uniforms = {
		.lightProjection = lightProjection,
		.shadowBias = 0.1,
		.shadowMap = sb,
		.lightDirection = lightDirection,
		.eyePosition = e,
	};

	// Shadow map loop
	if (drawShadows) {
    	for (int mi = 0; mi < N_MESHES; mi++) {
        	Mesh mesh = meshes[mi];
        	mat4 m_model = meshTransforms[mi];
            for (size_t i = 0; i + 2 < arr_len(mesh.tris); i += 3) {
                uint32_t i0 = mesh.tris[i], i1 = mesh.tris[i + 1], i2 = mesh.tris[i + 2];

                vec4 p = (vec4){ vec3dc(mesh.vertices[i0]), 1.0f };
                vec4 q = (vec4){ vec3dc(mesh.vertices[i1]), 1.0f };
                vec4 r = (vec4){ vec3dc(mesh.vertices[i2]), 1.0f };

                // To world space
                p = mat4vec4mul(m_model, p);
                q = mat4vec4mul(m_model, q);
                r = mat4vec4mul(m_model, r);

                // To light space
                p = mat4vec4mul(lightProjection, p);
                q = mat4vec4mul(lightProjection, q);
                r = mat4vec4mul(lightProjection, r);

				// Draw the triangle's depth in the depth buffer
                FR_DrawTriangleDepth(sb, SHADOW_WIDTH, SHADOW_HEIGHT, vec3dc(p), vec3dc(q), vec3dc(r));
            }
    	}
	}

	for (int mi = 0; mi < N_MESHES; mi++) {
    	Mesh mesh = meshes[mi];
    	mat4 m_model = meshTransforms[mi];
		BlinnPhongMaterial material = materials[mi];

		uniforms.albedo = material.albedo;
		uniforms.highlight = material.specular;
		uniforms.shininess = material.shininess;
		uniforms.ambient = material.ambient;

        for (size_t i = 0; i + 2 < arr_len(mesh.tris); i += 3) {
            uint32_t i0 = mesh.tris[i], i1 = mesh.tris[i + 1], i2 = mesh.tris[i + 2];

            vec4 p = (vec4){ vec3dc(mesh.vertices[i0]), 1.0f };
            vec4 q = (vec4){ vec3dc(mesh.vertices[i1]), 1.0f };
            vec4 r = (vec4){ vec3dc(mesh.vertices[i2]), 1.0f };

            // To world space
            p = mat4vec4mul(m_model, p);
            q = mat4vec4mul(m_model, q);
            r = mat4vec4mul(m_model, r);

			// Get the normals and normalize them
            vec4 n0 = vec4dir(mat4vec4mul(m_model, (vec4){ vec3dc(mesh.normals[mesh.tri_normals[i]]), 0.0f }));
            vec4 n1 = vec4dir(mat4vec4mul(m_model, (vec4){ vec3dc(mesh.normals[mesh.tri_normals[i + 1]]), 0.0f }));
            vec4 n2 = vec4dir(mat4vec4mul(m_model, (vec4){ vec3dc(mesh.normals[mesh.tri_normals[i + 2]]), 0.0f }));

            // Vertex data for the triangle
            float tri[3 * NVERTEXDATA] = {
				p.x, p.y, p.z, n0.x, n0.y, n0.z,
				q.x, q.y, q.z, n1.x, n1.y, n1.z,
				r.x, r.y, r.z, n2.x, n2.y, n2.z,
            };

            float clipped[8 * NVERTEXDATA];
            int count = FR_ClipPolygon(tri, 3, clipped, NVERTEXDATA, vec3dc(g), vec3dc(nearPlanePoint));
            // Less than 3 vertices produced by clipping means that there is nothing to render
            if (count < 3) continue;

            // Project the clipped vertices
            vec4 s[8];
            for (int k = 0; k < count; k++) {
                const float *v = clipped + k * NVERTEXDATA;
                s[k] = mat4vec4mul(m, (vec4) { v[0], v[1], v[2], 1.0f });
				if (perspective) {
    				s[k].x /= s[k].w; s[k].y /= s[k].w; s[k].z /= s[k].w;
				}
            }

            // Fan-triangulate the clipped vertices and draw
            // Filled triangles (fan)
            for (int k = 1; k + 1 < count; k++) {
                FR_DrawTriangle3DFS(fb, db, CANVAS_WIDTH, CANVAS_HEIGHT,
                                    // Projected position
                                    vec4dc(s[0]), vec4dc(s[k]), vec4dc(s[k + 1]),
                                    // (packed) vertex data
                                    clipped, clipped + k * NVERTEXDATA, clipped + (k + 1) * NVERTEXDATA, NVERTEXDATA,
                                    &uniforms, blinnPhongShader);
            }

                             
            if (drawNormals) {
                for (int k = 0; k < count; k++) {
                    const float *v = clipped + k * NVERTEXDATA;
                    vec3 worldPos = { v[0], v[1], v[2] };
                    vec3 normal = vec3dir((vec3) { v[3], v[4], v[5] });
                    vec3 endpoint = vec3add(worldPos, vec3scale(normal, NORMAL_LENGTH));

                    // Omit normal endpoints that are beyond the near plane
                    if (vec3dot(g, vec3sub(endpoint, nearPlanePoint)) < 0.0f) continue;

					vec4 endpointHomo = { vec3dc(endpoint), 1.0f };
					endpointHomo = mat4vec4mul(m, endpointHomo);
					if (perspective) {
    					endpointHomo.x /= endpointHomo.w;
    					endpointHomo.y /= endpointHomo.w;
    					endpointHomo.z /= endpointHomo.w;
					}
					FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(s[k]), vec2dc(endpointHomo), RED);
                }
            }

            if (drawWireframe) {
                for (int k = 0; k < count; k++) {
                    int j = (k + 1) % count;
                    FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(s[k]), vec2dc(s[j]), GREEN);
                }
            }
    	}
    }
    
    SDL_Event event;
    Engine* eng = (Engine*) self;
    // Clear input buffer
    eng->input.event_counter = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEMOTION:
                eng->input.mouseX = (int) event.motion.x / WINDOW_SCALE_FACTOR;
                eng->input.mouseY = (int) event.motion.y / WINDOW_SCALE_FACTOR;
                break;
            case SDL_MOUSEBUTTONDOWN:
                eng->input.mouseButton[event.button.button - 1] = true;
                break;
            case SDL_MOUSEBUTTONUP:
                eng->input.mouseButton[event.button.button - 1] = false;
                break;
            case SDL_KEYDOWN:
                if (eng->input.event_counter < INPUT_BUFFER_SIZE) {
                    InputEvent ev = { event.key.keysym.sym, true };
                    eng->input.events[eng->input.event_counter++] = ev;
                }
                break;
            case SDL_KEYUP:
                if (eng->input.event_counter < INPUT_BUFFER_SIZE) {
                    InputEvent ev = { event.key.keysym.sym, false };
                    eng->input.events[eng->input.event_counter++] = ev;
                }
                break;
        }
    }

    static struct timespec t_i;
	struct timespec t_f;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_f);

	double start = (double)t_i.tv_sec + (double)t_i.tv_nsec / 1e9;
	double end = (double)t_f.tv_sec + (double)t_f.tv_nsec / 1e9;
	float dt = (float)(end - start);

	{
    	theta += dt;
	};

	if (t_i.tv_sec == 0 || dt > 0.1f) dt = 0.016f;
	t_i = t_f;
    
    return dt;
}

void MyPostFrame(void* self, RenderContext rctx, AudioContext actx) {
    Engine* engine = (Engine*) self;

    for (int i = 0; i < engine->input.event_counter; i++) {
        if (engine->input.events[i].down) {
            if (engine->input.events[i].key == '+') {
                distance -= 0.5;
            } else if (engine->input.events[i].key == '-') {
                distance += 0.5;
            } else if (engine->input.events[i].key == 'p') {
                perspective = !perspective;
            } else if (engine->input.events[i].key == 'w') {
                drawWireframe = !drawWireframe;
            } else if (engine->input.events[i].key == 'n') {
                drawNormals = !drawNormals;
            } else if (engine->input.events[i].key == 's') {
                drawShadows = !drawShadows;
            }
        }
    }

	if (engine->input.mouseButton[0]) {
		float dx = ((float) engine->input.mouseX - mouseXPrev) / (CANVAS_WIDTH * WINDOW_SCALE_FACTOR);
		float dy = ((float) engine->input.mouseY - mouseYPrev) / (CANVAS_HEIGHT * WINDOW_SCALE_FACTOR);

		yaw -= dx * MOUSE_SENSITIVITY;
		pitch += dy * MOUSE_SENSITIVITY;

		if (pitch > MAX_PITCH) {
    		pitch = MAX_PITCH; 
		} else if (pitch < -MAX_PITCH) {
    		pitch = -MAX_PITCH;
		}
    }
	mouseXPrev = engine->input.mouseX;
	mouseYPrev = engine->input.mouseY;

    // Upload framebuffer to SDL Texture
    void *tex_pixels;
    int pitch;
    SDL_LockTexture(scrtex, NULL, &tex_pixels, &pitch);
    
    uint8_t *dst = (uint8_t *)tex_pixels;
    uint8_t *src = (uint8_t *)fb;
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        memcpy(dst, src, CANVAS_WIDTH * sizeof(uint32_t));
        dst += pitch;
        src += CANVAS_WIDTH * sizeof(uint32_t);
    }
    SDL_UnlockTexture(scrtex);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, scrtex, NULL, NULL);
    SDL_RenderPresent(ren);

    SDL_QueueAudio(
        dev,
        actx.output,
        actx.streams[0].n_samples * sizeof(float)
    );
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    win = SDL_CreateWindow("Engine Driver", SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH * WINDOW_SCALE_FACTOR, CANVAS_HEIGHT * WINDOW_SCALE_FACTOR,
                           SDL_WINDOW_BORDERLESS);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    scrtex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
    fb = calloc(CANVAS_WIDTH * CANVAS_HEIGHT, sizeof(uint32_t));
    db = calloc(CANVAS_WIDTH * CANVAS_HEIGHT, sizeof(float));
    sb = calloc(SHADOW_WIDTH * SHADOW_HEIGHT, sizeof(float));

    Engine engine = FE_InitEngine(MyPreFrame, MyPostFrame);
    RenderContext rctx = { fb, CANVAS_WIDTH, CANVAS_HEIGHT };

	FILE* texFile = fopen("res/cube.ppm", "r");
	if (texFile == NULL) {
    	exit(1);
	}
    cube_tex = readPPM(texFile, &cube_tex_width, &cube_tex_height);
    printf("Texture dims: %u, %u\n", cube_tex_width, cube_tex_height);
    if (cube_tex == NULL) {
        exit(1);
    }

	FILE *file = fopen("res/teapot.obj", "r");
	if (file == NULL) {
    	exit(1);
	}
	meshes[0] = FO_LoadObj(file);
	fclose(file);

	file = fopen("res/floor.obj", "r");
	meshes[1] = FO_LoadObj(file);
	fclose(file);

	file = fopen("res/monkey.obj", "r");
	if (file == NULL) {
    	exit(1);
	}
	meshes[2] = FO_LoadObj(file);
	fclose(file);

    vec3 v_min = { -f, -f, -f };
    vec3 v_max = { f, f, f };
    for (size_t i = 0; i < N_MESHES; i++) {
        Mesh mesh = meshes[i];
        for (size_t j = 0; j < arr_len(mesh.vertices); j++) {
			vec4 v = mat4vec4mul(meshTransforms[i], (vec4) { vec3dc(mesh.vertices[j]), 1.0f });
			v_min.x = fminf(v_min.x, v.x);
			v_min.y = fminf(v_min.y, v.y);
			v_min.z = fminf(v_min.z, v.z);
			v_max.x = fmaxf(v_max.x, v.x);
			v_max.y = fmaxf(v_max.y, v.y);
			v_max.z = fmaxf(v_max.z, v.z);
        }
    }
    lightRadius = vec3norm(vec3sub(v_max, v_min)) / 2.0f;
    printf("%f\n", lightRadius);

    while (running) {
        FE_Loop(&engine, rctx, (AudioContext) {0});
    }

    // Cleanup
    FE_DestroyEngine(&engine);

    free(fb);
    free(db);
    free(sb);
    free(cube_tex);
    return 0;
}
