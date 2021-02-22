#include "perlin.h"
#include <vector>
#include <cmath>

using std::unique_ptr;

using namespace mycraft::perlin;

double mycraft::perlin::perlin3d(double x, double y, double z)
{

	auto xi = (int) x % 256;
	auto yi = (int) y % 256;
	auto zi = (int) z % 256;
	auto xf = std::fmod(x, 256) - xi;
	auto yf = std::fmod(y, 256) - yi;
	auto zf = std::fmod(z, 256) - zi;
	auto u = fade(xf);
	auto v = fade(yf);
	auto w = fade(zf);

	auto aaa = p[p[p[xi] + yi] + zi];
	auto aba = p[p[p[xi] + inc(yi)] + zi];
	auto aab = p[p[p[xi] + yi] + inc(zi)];
	auto abb = p[p[p[xi] + inc(yi)] + inc(zi)];
	auto baa = p[p[p[inc(xi)] + yi] + zi];
	auto bab = p[p[p[inc(xi)] + yi] + inc(zi)];
	auto bba = p[p[p[inc(xi)] + inc(yi)] + zi];
	auto bbb = p[p[p[inc(xi)] + inc(yi)] + inc(zi)];

	auto x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u);
	auto x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);
	auto y1 = lerp(x1, x2, v);
	x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1),
			u);
	auto y2 = lerp(x1, x2, v);
	return (lerp(y1, y2, w) + 1) / 2;
}

Image3DResult mycraft::perlin::perlin3d_image(int x, int y, int z,
		int cell_size, int layers)
{
	std::unique_ptr<double[]> tmp(new double[x * y * z * layers]);
	const auto a3 = y * z * layers;
	const auto a2 = z * layers;
	const auto a1 = layers;

	for (int l = 0; l < layers; l++)
	{
		for (int i = 0; i < x; i++)
		{
			for (int j = 0; j < y; j++)
			{
				for (int k = 0; k < z; k++)
				{
					auto amplitude = 1.0 / std::pow(2, l);
					auto frequency = std::pow(2, l);
					auto period = cell_size / frequency;
					tmp[i * a3 + j * a2 + k * a1 + l] = perlin3d(i / period,
							j / period, k / period) * amplitude;
				}
			}
		}
	}

	unique_ptr<double[]> result(new double[x * y * z]);
	for (int i = 0; i < x; i++)
		for (int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
				result[i * y * z + j * z + k] = 0;

	for (int l = 0; l < layers; l++)
	{
		for (int i = 0; i < x; i++)
		{
			for (int j = 0; j < y; j++)
			{
				for (int k = 0; k < z; k++)
				{
					result[i * y * z + j * z + k] += tmp[i * a3 + j * a2
							+ k * a1 + l];
				}
			}
		}
	}

	for (int i = 0; i < x; i++)
		for (int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
				result[i * y * z + j * z + k] /= layers;

	return result;
}

Image2DResult mycraft::perlin::perlin2d_image(int height, int width,
		int cell_size, int layers)
{
	std::unique_ptr<double[]> tmp(new double[height * width * layers]);
	const auto a2 = width * layers;

	for (int k = 0; k < layers; k++)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				auto amplitude = 1.0 / std::pow(2, k);
				auto frequency = std::pow(2, k);
				auto period = cell_size / frequency;
				tmp[i * a2 + j * layers + k] = perlin2d(i / period, j / period)
						* amplitude;
			}
		}
	}

	unique_ptr<double[]> result(new double[height * width]);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			result[i * width + j] = 0;

	for (int k = 0; k < layers; k++)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				result[i * width + j] += tmp[i * a2 + j * layers + k];
			}
		}
	}

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			result[i * width + j] /= layers;

	return result;
}
