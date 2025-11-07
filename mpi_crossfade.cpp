#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdio>
#include <cmath>

namespace fs = std::filesystem;

/* -------------------------------------------------------------
   Conversión a escala de grises (ejecutada solo en el rank 0)
   ------------------------------------------------------------- */
void to_grayscale(const unsigned char* src, unsigned char* dst,
    int width, int height, int channels)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = y * width + x;
            unsigned char r = src[i * channels];
            unsigned char g = src[i * channels + 1];
            unsigned char b = src[i * channels + 2];
            unsigned char gray = static_cast<unsigned char>(
                0.299 * r + 0.587 * g + 0.114 * b + 0.5);
            int idx = i * 3;
            dst[idx] = dst[idx + 1] = dst[idx + 2] = gray;
        }
    }
}

/* -------------------------------------------------------------
   Cross-fade entre dos imágenes (p en [0,1])
   ------------------------------------------------------------- */
void crossfade(const unsigned char* img1, const unsigned char* img2, unsigned char* result,
    int width, int height, int channels, float p) {
    for (int i = 0; i < width * height * channels; ++i) {
        result[i] = static_cast<unsigned char>(img1[i] * p + img2[i] * (1.0f - p));
    }
}

/* -------------------------------------------------------------
   MAIN
   ------------------------------------------------------------- */
int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const std::string input_path = "images/input_color.png";
    const std::string out_dir = "frames_mpi";

    const int total_frames = 96;                 // 4 s * 24 fps
    const float p_step = 1.0f / (total_frames - 1);

    int width = 0, height = 0, channels = 0;
    unsigned char* color_img = nullptr, * gray_img = nullptr;

    /* -------------------------------------------------------------
       Carga de la imagen y generación de la versión gris (solo rank 0)
       ------------------------------------------------------------- */
    if (rank == 0) {
        color_img = stbi_load(input_path.c_str(), &width, &height, &channels, 3);
        if (!color_img) {
            std::cerr << "ERROR: No se pudo cargar la imagen: " << input_path << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        channels = 3;

        gray_img = new unsigned char[width * height * 3];
        to_grayscale(color_img, gray_img, width, height, channels);
    }

    /* -------------------------------------------------------------
       Broadcast de dimensiones y datos a todos los procesos
       ------------------------------------------------------------- */
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        color_img = new unsigned char[width * height * 3];
        gray_img = new unsigned char[width * height * 3];
    }

    MPI_Bcast(color_img, width * height * 3, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(gray_img, width * height * 3, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    /* -------------------------------------------------------------
       Distribución de frames entre procesos
       ------------------------------------------------------------- */
    int frames_per_proc = total_frames / size;
    int start_frame = rank * frames_per_proc;
    int end_frame = (rank == size - 1) ? total_frames : start_frame + frames_per_proc;

    double start_time = MPI_Wtime();

    /* Cada proceso crea su propia carpeta de salida (evita colisiones) */
    std::string local_dir = out_dir + "/rank" + std::to_string(rank);
    fs::create_directories(local_dir);

    for (int f = start_frame; f < end_frame; ++f) {
        float p = f * p_step;

        std::vector<unsigned char> result(width * height * 3);
        crossfade(color_img, gray_img, result.data(), width, height, 3, p);

        char filename[256];
        std::snprintf(filename, sizeof(filename),
            "%s/frame_%04d.png", local_dir.c_str(), f);
        stbi_write_png(filename, width, height, 3, result.data(), width * 3);
    }

    double end_time = MPI_Wtime();

    /* -------------------------------------------------------------
       Rank 0 muestra tiempo total (solo una vez)
       ------------------------------------------------------------- */
    if (rank == 0) {
        std::cout << "MPI (P=" << size << "): "
            << (end_time - start_time) << " s\n";
    }

    /* -------------------------------------------------------------
       Limpieza
       ------------------------------------------------------------- */
    if (rank == 0) {
        stbi_image_free(color_img);
        delete[] gray_img;
    }
    else {
        delete[] color_img;
        delete[] gray_img;
    }

    MPI_Finalize();
    return 0;
}