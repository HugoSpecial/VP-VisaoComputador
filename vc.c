//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}


int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}



int vc_gray_negative(IVC* srcdst) {
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) {
		return 0;
	}
	if (channels != 1) {
		return 0;
	}

	//Inverter a Imagem Gray
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
			//data[pos] = srcdst->levels - data[pos];
		}
	}
	return 1;
}

// Gerar negativo da imagem RGB
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Inverte a imagem RGB
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	return 1;
}



// Converter de RGB para Gray
int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}

	return 1;
}


int vec_rgb_to_hsv(IVC* src, IVC* dst)
{
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 3 || dst->channels != 3)) return 0;

	int size = src->width * src->height * src->channels;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;

	for (int i = 0; i < size; i += 3)
	{
		float r = data_src[i] / 255.0f;
		float g = data_src[i + 1] / 255.0f;
		float b = data_src[i + 2] / 255.0f;


		float max = MAX3(r, g, b);
		float min = MIN3(r, g, b);
		float delta = max - min;
		float h = 0, s = 0, v = max;

		if (delta > 0)
		{
			if (max == r)
			{
				h = 60 * fmodf(((g - b) / delta), 6);
			}
			else if (max == g)
			{
				h = 60 * (((b - r) / delta) + 2);
			}
			else
			{
				h = 60 * (((r - g) / delta) + 4);
			}

			if (h < 0)
				h += 360;

			s = max == 0 ? 0 : (delta / max);
		}

		data_dst[i] = (unsigned char)(h / 2);          // Convertendo H para 0-255
		data_dst[i + 1] = (unsigned char)(s * 255);   // Convertendo S para 0-255
		data_dst[i + 2] = (unsigned char)(v * 255);   // Convertendo V para 0-255
	}

	return 1;
}



int vc_rgb_to_hsv(IVC* src, IVC* dst)
{
	if (src == NULL || dst == NULL)
	{
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 3 || dst->channels != 3))
	{
		return 0;
	}

	int width = src->width;
	int height = src->height;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int i = (y * width + x) * 3; // Índice do pixel na matriz 1D

			float r = data_src[i] / 255.0f;
			float g = data_src[i + 1] / 255.0f;
			float b = data_src[i + 2] / 255.0f;

			float max = MAX3(r, g, b);
			float min = MIN3(r, g, b);
			float delta = max - min;
			float h = 0, s = 0, v = max;

			if (delta > 0)
			{
				if (max == r)
				{
					h = 60 * fmodf(((g - b) / delta), 6);
				}
				else if (max == g)
				{
					h = 60 * (((b - r) / delta) + 2);
				}
				else
				{
					h = 60 * (((r - g) / delta) + 4);
				}

				if (h < 0)
					h += 360;

				s = max == 0 ? 0 : (delta / max);
			}

			data_dst[i] = (unsigned char)(h / 2.0f + 0.5f);     // H (0–179)
			data_dst[i + 1] = (unsigned char)(s * 255.0f + 0.5f);   // S (0–255)
			data_dst[i + 2] = (unsigned char)(v * 255.0f + 0.5f);   // V (0–255)
		}
	}

	return 1;
}


// hmin,hmax = [0, 360]; smin,smax = [0, 100]; vmin,vmax = [0, 100]
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 3)) return 0;

	int bytesperline = src->width * src->channels;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos = y * bytesperline + x * src->channels;

			// Obter os valores HSV da imagem de entrada
			float h = data_src[pos] * 2;     // H: 0-360 graus
			float s = data_src[pos + 1] / 255.0 * 100; // S: 0-100 %
			float v = data_src[pos + 2] / 255.0 * 100; // V: 0-100 %

			// Verificar se estÃ¡ dentro do intervalo
			if ((h >= hmin && h <= hmax) && (s >= smin && s <= smax) && (v >= vmin && v <= vmax)) {
				data_dst[y * dst->width + x] = 255; // Branco
			}
			else {
				data_dst[y * dst->width + x] = 0;   // Preto
			}
		}
	}

	return 1;
}


int vc_scale_gray_to_color_palette(IVC* src, IVC* dst)
{
	if (src == NULL || dst == NULL)
		return 0;
	if (src->width != dst->width || src->height != dst->height || src->channels != 1 || dst->channels != 3)
		return 0;

	int width = src->width;
	int height = src->height;
	unsigned char* gray_data = src->data;
	unsigned char* color_data = dst->data;

	int r, g, b;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int i = y * width + x;
			unsigned char gray = gray_data[i];

			if (gray >= 0 && gray < 64) {
				r = 255;
				g = gray * 4;
				b = 0;
			}
			else if (gray >= 64 && gray < 128) {
				r = 255 - (gray - 64) * 4;
				g = 255;
				b = 0;
			}
			else if (gray >= 128 && gray < 193) {
				r = 0;
				g = 255;
				b = (gray - 128) * 4;
			}
			else {
				r = 0;
				g = 255 - (gray - 193) * 4;
				b = 255;
			}

			color_data[i * 3] = b;
			color_data[i * 3 + 1] = g;
			color_data[i * 3 + 2] = r;
		}
	}

	return 1;
}

int vc_gray_to_binary(IVC* src, IVC* dst, int threshold) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1) || (dst->channels != 1)) return 0;

	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos = y * src->width + x; // Cálculo direto da posição

			if (data_src[pos] > threshold) {
				data_dst[pos] = 255;
			}
			else {
				data_dst[pos] = 0;
			}
		}
	}

	return 1;
}

int vc_gray_to_binary_global_mean(IVC* src, IVC* dst) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1) || (dst->channels != 1)) return 0;

	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;
	int N = src->height; 
	int M = src->width;  
	long sum = 0; 

	for (int y = 0; y < N; y++) {
		for (int x = 0; x < M; x++) {
			sum += data_src[y * M + x];
		}
	}
	int threshold = sum / (N * M);



	for (int y = 0; y < N; y++) {
		for (int x = 0; x < M; x++) {
			int pos = y * M + x;

			if (data_src[pos] > threshold) {
				data_dst[pos] = 255;
			}
			else {
				data_dst[pos] = 0;
			}
		}
	}

	return 1;
}

int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) ||
		(src->channels != 1) || (dst->channels != 1)) return 0;

	int width = src->width;
	int height = src->height;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;
	int half_kernel = kernel / 2;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int v_min = 255;
			int v_max = 0;

			// Percorre a vizinhança
			for (int ky = -half_kernel; ky <= half_kernel; ky++) {
				for (int kx = -half_kernel; kx <= half_kernel; kx++) {
					int ny = y + ky;
					int nx = x + kx;

					// Verifica se está dentro da imagem
					if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
						int pos = ny * width + nx;
						int value = data_src[pos];

						if (value < v_min) v_min = value;
						if (value > v_max) v_max = value;
					}
				}
			}

			// Calcula o threshold
			int threshold = (v_min + v_max) / 2;
			int pos = y * width + x;

			// Aplica a binarização
			if (data_src[pos] > threshold) {
				data_dst[pos] = 255;
			}
			else {
				data_dst[pos] = 0;
			}
		}
	}

	return 1;
}


//Operadores morfologicos
/*int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1) || (dst->channels != 1)) return 0;

	int width = src->width;
	int height = src->height;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;
	int half_kernel = kernel / 2;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int pos = y * width + x;
			int found_white = 0;

			// Percorre a vizinhança
			for (int ky = -half_kernel; ky <= half_kernel; ky++) {
				for (int kx = -half_kernel; kx <= half_kernel; kx++) {
					int ny = y + ky;
					int nx = x + kx;

					// Verifica se está dentro da imagem
					if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
						int neighbor_pos = ny * width + nx;
						if (data_src[neighbor_pos] == 255) {
							found_white = 1;
						}
					}
				}
			}

			// Aplica a dilatação
			data_dst[pos] = found_white ? 255 : 0;
		}
	}

	return 1;
}

int vc_binary_erode(IVC* src, IVC* dst, int kernel) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1) || (dst->channels != 1)) return 0;

	int width = src->width;
	int height = src->height;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;
	int half_kernel = kernel / 2;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int pos = y * width + x;
			int found_black = 0;

			// Percorre a vizinhança
			for (int ky = -half_kernel; ky <= half_kernel; ky++) {
				for (int kx = -half_kernel; kx <= half_kernel; kx++) {
					int ny = y + ky;
					int nx = x + kx;

					// Verifica se está dentro da imagem
					if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
						int neighbor_pos = ny * width + nx;
						if (data_src[neighbor_pos] == 0) {
							found_black = 1;
						}
					}
				}
			}

			// Aplica a erosão
			data_dst[pos] = found_black ? 0 : 255;
		}
	}

	return 1;
}
*/

int vc_binary_erode(IVC* src, IVC* dst, int kernel) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1) || (dst->channels != 1)) return 0;

	int width = src->width;
	int height = src->height;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;
	int half_kernel = kernel / 2;

	memcpy(data_dst, data_src, width * height); // Copia original para evitar sobrescrita

	for (int y = half_kernel; y < height - half_kernel; y++) {
		for (int x = half_kernel; x < width - half_kernel; x++) {
			int pos = y * width + x;
			int found_black = 0;

			for (int ky = -half_kernel; ky <= half_kernel && !found_black; ky++) {
				for (int kx = -half_kernel; kx <= half_kernel; kx++) {
					int ny = y + ky;
					int nx = x + kx;
					int neighbor_pos = ny * width + nx;

					if (data_src[neighbor_pos] == 0) {
						found_black = 1;
						break; // Sai do loop assim que encontra um pixel preto
					}
				}
			}

			data_dst[pos] = found_black ? 0 : 255;
		}
	}

	return 1;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1) || (dst->channels != 1)) return 0;

	int width = src->width;
	int height = src->height;
	unsigned char* data_src = src->data;
	unsigned char* data_dst = dst->data;
	int half_kernel = kernel / 2;

	memcpy(data_dst, data_src, width * height); // Copia original para evitar sobrescrita

	for (int y = half_kernel; y < height - half_kernel; y++) {
		for (int x = half_kernel; x < width - half_kernel; x++) {
			int pos = y * width + x;
			int found_white = 0;

			for (int ky = -half_kernel; ky <= half_kernel && !found_white; ky++) {
				for (int kx = -half_kernel; kx <= half_kernel; kx++) {
					int ny = y + ky;
					int nx = x + kx;
					int neighbor_pos = ny * width + nx;

					if (data_src[neighbor_pos] == 255) {
						found_white = 1;
						break; // Sai do loop assim que encontra um pixel branco
					}
				}
			}

			data_dst[pos] = found_white ? 255 : 0;
		}
	}

	return 1;
}

int vc_binary_open(IVC* src, IVC* dst, int kernel) {

	IVC* imagefun[2];

	//erosao
	if (vc_binary_erode(src, dst, kernel) == 1) {
		printf("Erosao feita com sucesso\n");
	}
	else
	{
		printf("Erro na erosao\n");
	}

	imagefun[0] = vc_image_new(src->width, src->height, src->channels, src->levels);
	if (imagefun[0] == NULL)
	{
		printf("ERROR -> vc_image_new(): \n\nMemory allocation failed!\n");
		vc_image_free(imagefun[0]);
		getchar();
		return 0;
	}

	//dilatacao
	if (vc_binary_dilate(dst, imagefun[0], kernel) == 1) {
		printf("Dilatacao feita com sucesso\n");
	}
	else
	{
		printf("Erro na dilatacao\n");
	}

	dst = imagefun[0];

	return 1;
}

int vc_binary_close(IVC* src, IVC* dst, int kernel) {

	IVC* imagefun[2];

	//erosao
	if (vc_binary_dilate(src, dst, kernel) == 1) {
		printf("Dilatacao feita com sucesso\n");
	}
	else
	{
		printf("Erro na dilatacao\n");
	}

	imagefun[0] = vc_image_new(src->width, src->height, src->channels, src->levels);
	if (imagefun[0] == NULL)
	{
		printf("ERROR -> vc_image_new(): \n\nMemory allocation failed!\n");
		vc_image_free(imagefun[0]);
		getchar();
		return 0;
	}

	//dilatacao
	if (vc_binary_erode(dst, imagefun[0], kernel) == 1) {
		printf("erosao feita com sucesso\n");
	}
	else
	{
		printf("Erro na erosao\n");
	}

	dst = imagefun[0];

	return 1;
}


// Blobs e Etiquetagem
//int vc_binary_blob_labelling(IVC* src, IVC* dst) {
//	if (!src || !dst || !src->data || !dst->data) return -1;
//
//	// Copiar os dados da imagem fonte para a imagem destino (considerando bytesperline)
//	memcpy(dst->data, src->data, src->bytesperline * src->height);
//
//	int label = 1;
//
//	for (int y = 1; y < dst->height - 1; y++) { // Evita acessar fora da memória
//		for (int x = 1; x < dst->width - 1; x++) {
//			int posX = y * dst->bytesperline + x;
//			int posA = (y - 1) * dst->bytesperline + (x - 1);
//			int posB = (y - 1) * dst->bytesperline + x;
//			int posC = (y - 1) * dst->bytesperline + (x + 1);
//			int posD = y * dst->bytesperline + (x - 1);
//
//			if (dst->data[posX] == 255) {
//				int min_label = 255;
//
//				// Encontrar o menor rótulo vizinho
//				if (dst->data[posA] > 0 && dst->data[posA] < min_label) min_label = dst->data[posA];
//				if (dst->data[posB] > 0 && dst->data[posB] < min_label) min_label = dst->data[posB];
//				if (dst->data[posC] > 0 && dst->data[posC] < min_label) min_label = dst->data[posC];
//				if (dst->data[posD] > 0 && dst->data[posD] < min_label) min_label = dst->data[posD];
//
//				if (min_label == 255) { // Nenhum vizinho rotulado
//					dst->data[posX] = label;
//					label++;
//					if (label > 254) label = 254; // Evita estouro
//				}
//				else {
//					dst->data[posX] = min_label;
//				}
//			}
//			else {
//				dst->data[posX] = 0;
//			}
//		}
//	}
//
//	return 1; // Sucesso
//}

// Etiquetagem de blobs
// src		: Imagem binária de entrada
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. É necessário libertar posteriormente esta memória.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].yc = sumy / MAX2(blobs[i].area, 1);
		blobs[i].xc = sumx / MAX2(blobs[i].area, 1);
	}

	return 1;
}





int vc_gray_histogram_equalization(IVC* src, IVC* dst) {
	if (src == NULL || dst == NULL || src->data == NULL || dst->data == NULL) {
		return 0; // Erro: imagens inválidas
	}

	int n[256] = { 0 };      // Histograma
	float pdf[256];   // Probabilidade de ocorrência
	float cdf[256];   // Função de distribuição acumulada (CDF)
	unsigned char lut[256];   // Look-up table para transformação de níveis de cinza
	int total_pixels = src->width * src->height;

	// Passo 1: Calcular o histograma
	for (int i = 0; i < total_pixels; n[src->data[i++]]++);

	// Passo 2: Calcular a PDF (Probabilidade de ocorrência)
	for (int i = 0; i < 256; i++) {
		pdf[i] = (float)n[i] / total_pixels;
	}

	// Passo 3: Calcular a CDF (Distribuição acumulada)
	cdf[0] = pdf[0];
	for (int i = 1; i < 256; i++) {
		cdf[i] = cdf[i - 1] + pdf[i];
	}

	// Encontrar cdfmin (primeiro valor de CDF diferente de zero)
	float cdfmin = 0.0;
	for (int i = 0; i < 256; i++) {
		if (cdf[i] > 0) {
			cdfmin = cdf[i];
			break;
		}
	}

	// Passo 4: Construir a LUT (Look-Up Table)
	for (int i = 0; i < 256; i++) {
		lut[i] = (unsigned char)(((cdf[i] - cdfmin) / (1 - cdfmin)) * (256 - 1));
	}

	// Passo 5: Aplicar a transformação nos pixels da imagem de entrada
	for (int i = 0; i < total_pixels; i++) {
		dst->data[i] = lut[src->data[i]];
	}

	return 1; // Sucesso
}





int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th)
{
	int x, y;
	long int pos;
	int gx, gy;
	float g;

	// Verificações básicas
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1 || dst->channels != 1)) return 0;

	for (y = 1; y < src->height - 1; y++)
	{
		for (x = 1; x < src->width - 1; x++)
		{
			// Posição central da janela 3x3
			pos = y * src->width + x;

			// Cálculo de Gx
			gx =
				-src->data[(y - 1) * src->width + (x - 1)] + src->data[(y - 1) * src->width + (x + 1)] +
				-src->data[y * src->width + (x - 1)] + src->data[y * src->width + (x + 1)] +
				-src->data[(y + 1) * src->width + (x - 1)] + src->data[(y + 1) * src->width + (x + 1)];

			// Cálculo de Gy
			gy =
				src->data[(y - 1) * src->width + (x - 1)] + src->data[(y - 1) * src->width + x] + src->data[(y - 1) * src->width + (x + 1)] -
				src->data[(y + 1) * src->width + (x - 1)] - src->data[(y + 1) * src->width + x] - src->data[(y + 1) * src->width + (x + 1)];

			// Magnitude do gradiente
			g = sqrt((float)(gx * gx + gy * gy));

			// Limiarização
			dst->data[pos] = (g > th) ? 255 : 0;
		}
	}

	return 1;
}


int vc_gray_edge_sobel(IVC* src, IVC* dst, float th)
{
	int x, y;
	long int pos;
	int gx, gy;
	float g;

	// Verificações básicas
	if (src == NULL || dst == NULL) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != 1 || dst->channels != 1)) return 0;

	for (y = 1; y < src->height - 1; y++)
	{
		for (x = 1; x < src->width - 1; x++)
		{
			pos = y * src->width + x;

			// Gx Sobel
			gx =
				-src->data[(y - 1) * src->width + (x - 1)] + src->data[(y - 1) * src->width + (x + 1)] +
				-2 * src->data[y * src->width + (x - 1)] + 2 * src->data[y * src->width + (x + 1)] +
				-src->data[(y + 1) * src->width + (x - 1)] + src->data[(y + 1) * src->width + (x + 1)];

			// Gy Sobel
			gy =
				src->data[(y - 1) * src->width + (x - 1)] + 2 * src->data[(y - 1) * src->width + x] + src->data[(y - 1) * src->width + (x + 1)] -
				src->data[(y + 1) * src->width + (x - 1)] - 2 * src->data[(y + 1) * src->width + x] - src->data[(y + 1) * src->width + (x + 1)];

			// Magnitude do gradiente
			g = sqrt((float)(gx * gx + gy * gy));

			// Limiarização
			dst->data[pos] = (g > th) ? 255 : 0;
		}
	}

	return 1;
}
