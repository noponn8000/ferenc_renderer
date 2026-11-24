#include "obj.h"
#include <utils/array.h>

Mesh FO_LoadObj(FILE* file) {
    Mesh mesh = { 0 };
	char line[1024];
	while (fgets(line, sizeof(line), file)) {
		if (line[0] == 'v' && line[1] == ' ') {
    		float x, y, z;
			if (sscanf(line + 3, "%f %f %f", &x, &y, &z) == 3) {
    			// Just vertex position
    			vec3 vertex = (vec3) { x, y, z };
    			arr_push(mesh.vertices, vertex);
			}
		} else if (line[0] == 'f' && line[1] == ' ') {
			// Face
            int a, b, c;
            if (sscanf(line + 3, "%d %d %d", &a, &b, &c) == 3) {
                arr_push(mesh.tris, (unsigned)(a - 1));
                arr_push(mesh.tris, (unsigned)(b - 1));
                arr_push(mesh.tris, (unsigned)(c - 1));
            }
		}
    }

	return mesh;
}
