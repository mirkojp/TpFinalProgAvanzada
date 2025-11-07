#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct Pixel {
    unsigned char r, g, b;
};

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
                0.299f * r + 0.587f * g + 0.114f * b + 0.5f);
            int idx = i * 3;
            dst[idx] = dst[idx + 1] = dst[idx + 2] = gray;
        }
    }
}

void crossfade(const unsigned char* img1, const unsigned char* img2, unsigned char* result,
    int width, int height, int channels, float p) {
    for (int i = 0; i < width * height * channels; ++i) {
        result[i] = static_cast<unsigned char>(img1[i] * p + img2[i] * (1.0f - p));
    }
}

int main() {
    const std::string input_path = "images/input_color.png";
    const std::string output_dir = "frames_seq";
    fs::create_directories(output_dir);

    int width, height, channels;
    unsigned char* color_img = stbi_load(input_path.c_str(), &width, &height, &channels, 3);
    if (!color_img) {
        std::cerr << "Error loading image!\n";
        return 1;
    }
    channels = 3;

    std::vector<unsigned char> gray_img(width * height * 3);
    to_grayscale(color_img, gray_img.data(), width, height, channels);

    const int total_frames = 4 * 24; // 4 segundos a 24 FPS
    const float p_step = 1.0f / (total_frames - 1);

    auto start = std::chrono::high_resolution_clock::now();

    for (int f = 0; f < total_frames; ++f) {
        float p = f * p_step;
        std::vector<unsigned char> result(width * height * 3);
        crossfade(color_img, gray_img.data(), result.data(), width, height, 3, p);

        char filename[64];
        sprintf(filename, "%s/frame_%04d.png", output_dir.c_str(), f);
        stbi_write_png(filename, width, height, 3, result.data(), width * 3);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double time_sec = std::chrono::duration<double>(end - start).count();
    std::cout << "Sequential: " << total_frames << " frames in " << time_sec << " s\n";

    stbi_image_free(color_img);
    return 0;
}