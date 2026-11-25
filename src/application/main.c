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
SDL_AudioDeviceID dev;
bool running = true;

// Data
uint32_t* cube_tex;
uint16_t cube_tex_width;
uint16_t cube_tex_height;

Mesh teapot;

// For rotating objects periodically
float theta;

// ---- Camera
// Perspective vs. ortohgraphic
bool perspective = true;
bool drawWireframe = false;
bool drawNormals = false;

// Where the camera is looking at
vec3 target = { 0 };
// Distance from target
float distance = 5.0f;
// Rotation
float pitch = 0.0f;
float yaw = 0.0f;

float mouseXPrev = -1.0f;
float mouseYPrev = -1.0f;

const float MOUSE_SENSITIVITY = 3.5;
const float MAX_PITCH = 1.2;
const float NORMAL_LENGTH = 3.0;

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
#define CANVAS_HEIGHT 800
#define CANVAS_WIDTH 800

typedef struct {
	vec3 albedo;
	vec3 highlight;
	vec3 lightPosition;
	vec3 eyePosition;
	float shininess;
	float ambient;
} BlinnPhongUniforms;

uint32_t blinnPhongShader(const void* uniforms, const float* in) {
	BlinnPhongUniforms* bp_uniforms = (BlinnPhongUniforms*) uniforms;

	vec3 worldPos = {
    	in[0],
    	in[1],
    	in[2]
	};

	vec3 normal = {
    	in[3],
    	in[4],
    	in[5]
	};
	normal = vec3dir(normal);

	vec3 rgb = blinn_phong(
    	worldPos, bp_uniforms->lightPosition, bp_uniforms->eyePosition,
    	normal, bp_uniforms->shininess, bp_uniforms->ambient, bp_uniforms->albedo, bp_uniforms->highlight
	);

	return FR_RGBAToAGBR8888(rgb.x * 255, rgb.y * 255, rgb.z * 255, 255);
}

float MyPreFrame(void* self) {
    memset(fb, 0xFFFBFBFB, CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(uint32_t)); // Clear framebuffer

	float cospitch = cosf(pitch);
	float sinpitch = sinf(pitch);
	float cosyaw = cosf(yaw);
	float sinyaw = sinf(yaw);
	// Eye position
	vec3 e = {
    	target.x + distance * cospitch * sinyaw,
    	target.y + distance * sinpitch,
    	target.z + distance * cospitch * cosyaw
	};

	// Gaze direction
	vec3 g = vec3dir(vec3sub(target, e));

	// View-up vector
	vec3 up = {
    	0, 1, 0
	};

	// Construct orthonormal basis from g, up
	vec3 w = vec3scale(g, -1.0); 
	vec3 u = vec3cross(up, w);
	vec3 v = vec3cross(w, u);

	// camera transform matrix
	mat4 m_cam = {
		u.x, u.y, u.z, -vec3dot(u, e),
		v.x, v.y, v.z, -vec3dot(v, e),
		w.x, w.y, w.z, -vec3dot(w, e),
		0,   0,   0,   1
	};

	// viewport matrix
	// canonical view volume -> screen space
	float nx = CANVAS_WIDTH; float ny = CANVAS_HEIGHT;
	mat4 m_vp = {
		nx / 2,    0, 0, (nx - 1) / 2,
		     0, -ny/2, 0, (ny - 1) / 2,
		     0,    0, 1, 			0,
		     0,    0, 0, 			1
	};

    mat4 m_model = (mat4) {
		0.05, 0, 0, 0,
		0, 0.05, 0, 0,
		0, 0, 0.05, 0,
		0, 0, 0, 	1
    };

	mat4 m_orth = {
    	2/(r - l),       0,       0, -(r+l)/(r-l),
    	        0, 2/(t-b),       0, -(t+b)/(t-b),
    	        0,       0, 2/(n-f), -(n+f)/(n-f),
    	        0,       0,       0, 			1
	};

	mat4 m_perspective = {
    	n, 0, 0, 0,
    	0, n, 0, 0,
    	0, 0, n + f, -f * n,
    	0, 0, 1, 0
	};

	mat4 m;
	// Perspective
	if (perspective) {
    	m = mat4mat4mul(m_perspective, m_cam);
	} else {
		m = m_cam;
	}
	m = mat4mat4mul(m_orth, m);
	m = mat4mat4mul(m_vp, m);

	// Clear depth buffer
	for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++) db[i] = f;

	if (true) {
		BlinnPhongUniforms uniforms = {
    		.albedo = (vec3) { 0.81, 0.40, 0.47 },
    		.highlight = (vec3) { 1.0, 1.0, 1.0 },
    		.lightPosition = { 3.0, 5.0, 0.0 },
    		.eyePosition = e,
    		.shininess = 8,
    		.ambient = 0.3
		};

        for (size_t i = 0; i + 2 < arr_len(teapot.tris); i += 3) {
            uint32_t i0 = teapot.tris[i], i1 = teapot.tris[i + 1], i2 = teapot.tris[i + 2];

            vec4 p = (vec4){ vec3dc(teapot.vertices[i0]), 1.0f };
            vec4 q = (vec4){ vec3dc(teapot.vertices[i1]), 1.0f };
            vec4 r = (vec4){ vec3dc(teapot.vertices[i2]), 1.0f };

            // To world space
            p = mat4vec4mul(m_model, p);
            q = mat4vec4mul(m_model, q);
            r = mat4vec4mul(m_model, r);

            vec4 n0 = mat4vec4mul(m_model, (vec4){ vec3dc(teapot.normals[i0]), 0.0f });
            vec4 n1 = mat4vec4mul(m_model, (vec4){ vec3dc(teapot.normals[i1]), 0.0f });
            vec4 n2 = mat4vec4mul(m_model, (vec4){ vec3dc(teapot.normals[i2]), 0.0f });

            vec4 pp, qp, rp;
            if (drawNormals) {
                pp = vec4add(p, vec4scale(n0, NORMAL_LENGTH));
                qp = vec4add(q, vec4scale(n1, NORMAL_LENGTH));
                rp = vec4add(r, vec4scale(n2, NORMAL_LENGTH));
            }

			float v0[6] = { p.x, p.y, p.z, n0.x, n0.y, n0.z };
			float v1[6] = { q.x, q.y, q.z, n1.x, n1.y, n1.z };
			float v2[6] = { r.x, r.y, r.z, n2.x, n2.y, n2.z };


            p = mat4vec4mul(m, p);
            q = mat4vec4mul(m, q);
            r = mat4vec4mul(m, r);
            if (drawNormals) {
                pp = mat4vec4mul(m, pp);
                qp = mat4vec4mul(m, qp);
                rp = mat4vec4mul(m, rp);
            }

            if (perspective) {
                p.x /= p.w; p.y /= p.w; p.z /= p.w;
                q.x /= q.w; q.y /= q.w; q.z /= q.w;
                r.x /= r.w; r.y /= r.w; r.z /= r.w;
                if (drawNormals) {
                    pp.x /= pp.w; pp.y /= pp.w; pp.z /= pp.w;
                    qp.x /= qp.w; qp.y /= qp.w; qp.z /= qp.w;
                    rp.x /= rp.w; rp.y /= rp.w; rp.z /= rp.w;
                }
            }

            FR_DrawTriangle3DFS(fb, db, CANVAS_WIDTH, CANVAS_HEIGHT, vec4dc(p), vec4dc(q), vec4dc(r),
                                v0, v1, v2, 6,
                                &uniforms, blinnPhongShader
            );
                                

            if (drawNormals) {
                FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(p), vec2dc(pp), RED);
                FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(q), vec2dc(qp), RED);
                FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(r), vec2dc(rp), RED);
            }

            if (drawWireframe) {
                FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(p), vec2dc(q), GREEN);
                FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(q), vec2dc(r), GREEN);
                FR_DrawLine(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(r), vec2dc(p), GREEN);
            }
        }
    } else if (false) {
		// Spinning cube
        vec3 cube_vertices[24] = {
            // front (+z)
            {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1},
            // top (+y)
            {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}, { 1, 1,-1},
            // bottom (-y)
            {-1,-1,-1}, { 1,-1,-1}, { 1,-1, 1}, {-1,-1, 1},
            // left (-x)
            {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1}, {-1, 1,-1},
            // right (+x)
            { 1,-1,-1}, { 1, 1,-1}, { 1, 1, 1}, { 1,-1, 1},
            // back (-z)
            {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}, { 1,-1,-1},
        };

        int cube_tris[] = {
             0,  1,  2,    0,  2,  3,   // front
             4,  5,  6,    4,  6,  7,   // top
             8,  9, 10,    8, 10, 11,   // bottom
            12, 13, 14,   12, 14, 15,   // left
            16, 17, 18,   16, 18, 19,   // right
            20, 21, 22,   20, 22, 23,   // back
        };

        vec2 cube_uvs[] = {
            // front
            {0.25f, 0.66666667f}, {0.5f,  0.66666667f}, {0.5f,  0.33333333f}, {0.25f, 0.33333333f},
            // top
            {0.25f, 0.0f},        {0.25f, 0.33333333f}, {0.5f,  0.33333333f}, {0.5f,  0.0f},
            // bottom
            {0.25f, 1.0f},        {0.5f,  1.0f},        {0.5f,  0.66666667f}, {0.25f, 0.66666667f},
            // left
            {0.0f,  0.66666667f}, {0.25f, 0.66666667f}, {0.25f, 0.33333333f}, {0.0f,  0.33333333f},
            // right
            {0.75f, 0.66666667f}, {0.75f, 0.33333333f}, {0.5f,  0.33333333f}, {0.5f,  0.66666667f},
            // back
            {1.0f,  0.66666667f}, {1.0f,  0.33333333f}, {0.75f, 0.33333333f}, {0.75f, 0.66666667f},
        };

        uint32_t vertex_colors[] = {
            BROWN, BROWN, BROWN, BROWN, GRAY, GRAY, GRAY, GRAY,
        };


    	for (size_t i = 0; i < (size_t) sizeof(cube_tris) / sizeof(int); i += 3) {
        	vec4 p = { vec3dc(cube_vertices[cube_tris[i]]), 1 };
        	vec4 q = { vec3dc(cube_vertices[cube_tris[i + 1]]), 1 };
        	vec4 r = { vec3dc(cube_vertices[cube_tris[i + 2]]), 1 };

			// Model to world
    		p = mat4vec4mul(m_model, p);
    		q = mat4vec4mul(m_model, q);
    		r = mat4vec4mul(m_model, r);

    		p = mat4vec4mul(m, p);
    		q = mat4vec4mul(m, q);
    		r = mat4vec4mul(m, r);

    		// Perspective - leave w alone for interpolation
    		if (perspective) {
        		p.x /= p.w; p.y /= p.w; p.z /= p.w;
        		q.x /= q.w; q.y /= q.w; q.z /= q.w;
        		r.x /= r.w; r.y /= r.w; r.z /= r.w;
    		}

    		//FR_DrawTriangle3DVC(fb, db, CANVAS_WIDTH, CANVAS_HEIGHT, vec4dc(p), vec4dc(q), vec4dc(r), vertex_colors[cube_tris[i]], vertex_colors[cube_tris[i + 1]], vertex_colors[cube_tris[i + 2]]);
    		FR_DrawTriangle3DUV(fb, db, CANVAS_WIDTH, CANVAS_HEIGHT, vec4dc(p), vec4dc(q), vec4dc(r),
								vec2dc(cube_uvs[cube_tris[i]]), vec2dc(cube_uvs[cube_tris[i + 1]]), vec2dc(cube_uvs[cube_tris[i + 2]]),
    		                    cube_tex, cube_tex_width, cube_tex_height,
    		                    FR_SampleNearest
    		);
    	}
	}

	// 2D homogenous example, barycentric coords
	if (false) {
    	int tx = 400 + 100 * sin(theta); int ty = 400 + 100 * sin(theta);
    	vec3 v1 = { -100, -100, 1 };
    	vec3 v2 = { 100, -100, 1 };
    	vec3 v3 = { 0, 100, 1 };

    	mat3 rotate_translate = {
        	cos(theta), -sin(theta), tx,
        	sin(theta), cos(theta),  ty,
    		0,			0,			 1
    	};

    	v1 = mat3vec3mul(rotate_translate, v1);
    	v2 = mat3vec3mul(rotate_translate, v2);
    	v3 = mat3vec3mul(rotate_translate, v3);
    	
        FR_DrawCircleFill(fb, CANVAS_WIDTH, CANVAS_HEIGHT, 300, 300, 50.0f, MAGENTA);
        FR_DrawTriangle2DVC(fb, CANVAS_WIDTH, CANVAS_HEIGHT, vec2dc(v1), vec2dc(v2), vec2dc(v3), RED, GREEN, BLUE);
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
	teapot = FO_LoadObj(file);

    while (running) {
        FE_Loop(&engine, rctx, (AudioContext) {0});
    }

    // Cleanup
    FE_DestroyEngine(&engine);

    free(fb);
    free(db);
    free(cube_tex);
    return 0;
}
