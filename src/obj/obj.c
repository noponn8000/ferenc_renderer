#include "obj.h"
#include <ctype.h>
#include <utils/array.h>

static long objToArrayIndex(long i, size_t count) {
    // Obj positions are one-indexed !
    if (i > 0) return i - 1;
    // Relative indices
    if (i < 0) return (long) count + i;
    // i == 0 means undefined
	return 0;
}

Mesh FO_LoadObj(FILE* file) {
    Mesh mesh = { 0 };
	char line[1024];
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == 'v' && line[1] == ' ') {
    		float x, y, z;
			if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3) {
    			// Just vertex position
    			vec3 vertex = (vec3) { x, y, z };
    			arr_push(mesh.vertices, vertex);
			}
		} else if (line[0] == 'v' && line[1] == 'n') {
    		vec3 n; 
    		if (sscanf(line + 2, "%f %f %f", &n.x, &n.y, &n.z) == 3) {
        		arr_push(mesh.normals, n);
    		}
    	} else if (line[0] == 'f' && line[1] == ' ') {
			// Face
			// Faces are defined as vertex_index/uv_index/normal_index
			// The latter two fields are optional
    		long v[32], t[32], n[32];
    		int count = 0;
    		char *p = line + 2;

			while (count < 32) {
    			// Skip whitespace
        		while (isspace(*p)) p++;

        		char *end;
        		long vertex, uv, normal;

        		vertex = strtol(p, &end, 10);
        		if (end == p) break;
        		p = end;

				uv = 0; normal = 0;
        		if (*p == '/') {
            		p++;
            		uv = strtol(p, &end, 10);
            		p = end;

            		if (*p == '/') {
                		p++;
                		normal = strtol(p, &end, 10);
                		p = end;
            		}
        		}

				v[count] = objToArrayIndex(vertex, arr_len(mesh.vertices));
				t[count] = objToArrayIndex(uv, arr_len(mesh.uvs));
				n[count] = objToArrayIndex(normal, arr_len(mesh.normals));

				count++;
			}

			// fan triangulate for now
			for (int i = 1; i + 1 < count; i++) {
    			arr_push(mesh.tris, v[0]);
    			arr_push(mesh.tris, v[i]);
    			arr_push(mesh.tris, v[i+1]);

    			arr_push(mesh.tri_normals, n[0]);
    			arr_push(mesh.tri_normals, n[i]);
    			arr_push(mesh.tri_normals, n[i + 1]);
			}
		}
    }

	return mesh;
}
