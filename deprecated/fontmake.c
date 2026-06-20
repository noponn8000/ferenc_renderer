#include "render.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Use stb_image to load the font.png file and convert it to AGBR8888 representation.
uint32_t* LoadFontTexture(int *tex_width, int *tex_height, char const *font_filename) {
    int x; int y;
    unsigned char* pixels = stbi_load(font_filename, &x, &y, NULL, 4);
    uint32_t* texture = malloc(sizeof(uint32_t) * x * y);

    for (int i = 0; i < x * y; i++) {
        int r = pixels[4 * i];
        int g = pixels[4 * i + 1];
        int b = pixels[4 * i + 2];
        int a = pixels[4 * i + 3];
        uint32_t color = FR_RGBAToAGBR8888(r, g, b, a);
        texture[i] = color;
    }

    stbi_image_free(pixels);

    *tex_width = x;
    *tex_height = y;

    return texture;
}

// Writes the uint32_t array to a C header file as a const along with some other font metadata.
// The font.h header is then to be included by code that requires the font texture and passed directly
// as pixel data to font rendering functions.
int main(void) {
	int x; int y;
	uint32_t* fonttex = LoadFontTexture(&x, &y, "font.png");
	// Open font file
	FILE *fp = fopen("font.h", "w");

	fprintf(fp, "#include <stdint.h>\n\n");
        fprintf(fp, "#define FONT_GLYPH_SET \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.!?-/{}\\\"\"\n");
	fprintf(fp, "#define FONT_TEX_WIDTH %d\n", x);
	fprintf(fp, "#define FONT_TEX_HEIGHT %d\n", y);
	fprintf(fp, "#define FONT_GLYPH_WIDTH 16\n");
	fprintf(fp, "#define FONT_GLYPH_HEIGHT 16\n");
	fprintf(fp, "#define FONT_N_GLYPHS 70\n\n");
	fprintf(fp, "const int lookup[128] = {\n");
	const char* glyphset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.!?-/{}\"";
	for (int c = 0; c < 128; c++) {
		char* p = strchr(glyphset, c);
		int i = (p == NULL) ? -1 : (p - glyphset);
		fprintf(fp, "%d, ", i);
	}
	fprintf(fp, "\n};\n");
	fprintf(fp, "const uint32_t fonttex[%d] = {\n", x * y);
	for (int j = 0; j < y; j++) {
		for (int i = 0; i < x; i++) {
			fprintf(fp, "0x%08X, ", fonttex[j * x + i]);		
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");

	free(fonttex);
	fclose(fp);
	return 0;
}
