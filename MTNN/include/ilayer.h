#pragma once

#include "imatrix.h"

#define CNN_LAYER_INPUT 0
#define CNN_LAYER_CONVOLUTION 1
#define CNN_LAYER_PERCEPTRONFULLCONNECTIVITY 2
//#define CNN_LAYER_PERCEPTRONLOCALCONNECTIVITY 3 todo?
#define CNN_LAYER_MAXPOOL 4
#define CNN_LAYER_SOFTMAX 5
#define CNN_LAYER_OUTPUT 6

#define CNN_FUNC_LINEAR 0
#define CNN_FUNC_LOGISTIC 1
#define CNN_FUNC_BIPOLARLOGISTIC 2
#define CNN_FUNC_TANH 3
#define CNN_FUNC_TANHLECUN 4
#define CNN_FUNC_RELU 5
#define CNN_FUNC_RBM 6

#define CNN_BIAS_NONE 0
#define CNN_BIAS_CONV 1
#define CNN_BIAS_PERCEPTRON 2

#define CNN_DATA_FEATURE_MAP 0
#define CNN_DATA_WEIGHT_GRAD 1
#define CNN_DATA_BIAS_GRAD 2
#define CNN_DATA_WEIGHT_MOMENT 3
#define CNN_DATA_BIAS_MOMENT 4
#define CNN_DATA_WEIGHT_HESSIAN 5
#define CNN_DATA_BIAS_HESSIAN 6
#define CNN_DATA_ACT 7

template <size_t r, size_t c, size_t kernel_r, size_t kernel_c, size_t s, bool use_pad> struct conv_helper_funcs
{
	static Matrix2D<float, (use_pad ? r : (r - kernel_r) / s + 1), (use_pad ? c : (c - kernel_c) / s + 1)> convolve(Matrix2D<float, r, c>& input, Matrix2D<float, kernel_r, kernel_c>& kernel);
	static void back_prop_kernel(Matrix2D<float, r, c> input, Matrix2D<float, (use_pad ? r : (r - kernel_r) / s + 1), (use_pad ? c : (c - kernel_c) / s + 1)> output, Matrix2D<float, kernel_r, kernel_c> kernel_gradient);
	static void back_prop_kernel_hessian(Matrix2D<float, r, c>& input, Matrix2D<float, (use_pad ? r : (r - kernel_r) / s + 1), (use_pad ? c : (c - kernel_c) / s + 1)>& output, Matrix2D<float, kernel_r, kernel_c>& kernel_hessian, float gamma);
	static Matrix2D<float, r, c> convolve_back(Matrix2D<float, (use_pad ? r : (r - kernel_r) / s + 1), (use_pad ? c : (c - kernel_c) / s + 1)> input, Matrix2D<float, kernel_r, kernel_c> kernel);
	static Matrix2D<float, r, c> convolve_back_weights_hessian(Matrix2D<float, r, c>& input, Matrix2D<float, kernel_r, kernel_c>& kernel);
};

template<size_t r, size_t c, size_t kernel_r, size_t kernel_c, size_t s> struct
conv_helper_funcs<r, c, kernel_r, kernel_c, s, false>
{
	static Matrix2D<float, (r - kernel_r) / s + 1, (c - kernel_c) / s + 1> convolve(Matrix2D<float, r, c>& input, Matrix2D<float, kernel_r, kernel_c>& kernel)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		constexpr size_t out_r = (r - kernel_r) / s + 1;
		constexpr size_t out_c = (c - kernel_c) / s + 1;
		Matrix2D<float, out_r, out_c> output = { 0 };

		for (size_t i = N; i < (r - N); i += s)//change focus of kernel
		{
			for (size_t j = M; j < (c - M); j += s)
			{
				//iterate over kernel
				float sum = 0;
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						sum += input.at(i - n, j - m) * kernel.at(N - n, N - m);
				output.at((i - N) / s, (j - N) / s) = sum;
			}
		}
		return output;
	}

	static void back_prop_kernel(Matrix2D<float, r, c> input, Matrix2D<float, (r - kernel_r) / s + 1, (c - kernel_c) / s + 1> output, Matrix2D<float, kernel_r, kernel_c> kernel_gradient)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		constexpr size_t out_r = (r - kernel_r) / s + 1;
		constexpr size_t out_c = (c - kernel_c) / s + 1;

		size_t i_0 = 0;
		size_t j_0 = 0;

		//change focus of kernel
		for (size_t i = N; i < (r - N); i += s)
		{
			for (size_t j = M; j < (c - M); j += s)
			{
				//iterate over kernel
				float sum = 0;
				float out = output.at(i_0, j_0);
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						kernel_gradient.at(N - n, M - m) += input.at(i - n, j - m) * out;
				++j_0;
			}
			j_0 = 0;
			++i_0;
		}
	}

	static void back_prop_kernel_hessian(Matrix2D<float, r, c>& input, Matrix2D<float, (r - kernel_r) / s + 1, (c - kernel_c) / s + 1>& output, Matrix2D<float, kernel_r, kernel_c>& kernel_hessian, float gamma)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		constexpr size_t out_r = (r - kernel_r) / s + 1;
		constexpr size_t out_c = (c - kernel_c) / s + 1;

		size_t i_0 = 0;
		size_t j_0 = 0;

		//change focus of kernel
		for (size_t i = N; i < (r - N); i += s)
		{
			for (size_t j = M; j < (c - M); j += s)
			{
				//iterate over kernel
				float sum = 0;
				float out = output.at(i_0, j_0);
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						kernel_hessian.at(N - n, M - m) += gamma * input.at(i - n, j - m) * input.at(i - n, j - m) * out;
				j_0++;
			}
			j_0 = 0;
			++i_0;
		}
	}

	static Matrix2D<float, r, c> convolve_back(Matrix2D<float, (r - kernel_r) / s + 1, (c - kernel_c) / s + 1> input, Matrix2D<float, kernel_r, kernel_c> kernel)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		Matrix2D<float, r, c> output = { 0 };

		size_t times_across = 0;
		size_t times_down = 0;

		for (size_t i = N; i < (r - N); i += s)
		{
			for (size_t j = M; j < (c - M); j += s)
			{
				//find all possible ways convolved size_to
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						output.at(i - n, j - m) += kernel.at(N - n, M - m) * input.at(times_down, times_across);
				++times_across;
			}
			times_across = 0;
			++times_down;
		}
		return output;
	}

	static Matrix2D<float, r, c> convolve_back_weights_hessian(Matrix2D<float, (r - kernel_r) / s + 1, (c - kernel_c) / s + 1>& input, Matrix2D<float, kernel_r, kernel_c>& kernel)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		Matrix2D<float, r, c> output = { 0 };

		size_t times_across = 0;
		size_t times_down = 0;

		for (size_t i = N; i < (r - N); i += s)
		{
			for (size_t j = M; j < (c - M); j += s)
			{
				//find all possible ways convolved size_to
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						output.at(i - n, j - m) += kernel.at(N - n, M - m) * kernel.at(N - n, M - m) * input.at(times_down, times_across);
				++times_across;
			}
			times_across = 0;
			++times_down;
		}
		return output;
	}
};

template<size_t r, size_t c, size_t kernel_r, size_t kernel_c, size_t s> struct conv_helper_funcs<r, c, kernel_r, kernel_c, s, true>
{
	static Matrix2D<float, r, c> convolve(Matrix2D<float, r, c>& input, Matrix2D<float, kernel_r, kernel_c>& kernel)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		constexpr size_t out_r = r;
		constexpr size_t out_c = c;
		Matrix2D<float, out_r, out_c> output = { 0 };

		//change focus of kernel
		for (size_t i = 0; i < r; i += s)
		{
			for (size_t j = 0; j < c; j += s)
			{
				//iterate over kernel
				float sum = 0;
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						sum += kernel.at(N - n, N - m) * (i < 0 || i >= r || j < 0 || j >= c ? 0 : input.at(i - n, j - m));
				output.at((i - N) / s, (j - N) / s) = sum;
			}
		}
		return output;
	}

	static void back_prop_kernel(Matrix2D<float, r, c> input, Matrix2D<float, r, c> output, Matrix2D<float, kernel_r, kernel_c> kernel_gradient)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		constexpr size_t out_r = r;
		constexpr size_t out_c = c;

		size_t i_0 = 0;
		size_t j_0 = 0;

		//change focus of kernel
		for (size_t i = 0; i < r; i += s)
		{
			for (size_t j = 0; j < c; j += s)
			{
				//iterate over kernel
				float sum = 0;
				float out = output.at(i_0, j_0);
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						kernel_gradient.at(N - n, M - m) += out * (i < 0 || i >= r || j < 0 || j >= c ? 0 : input.at(i - n, j - m));
				++j_0;
			}
			j_0 = 0;
			++i_0;
		}
	}

	static void back_prop_kernel_hessian(Matrix2D<float, r, c>& input, Matrix2D<float, r, c>& output, Matrix2D<float, kernel_r, kernel_c>& kernel_hessian, float gamma)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		constexpr size_t out_r = (r - kernel_r) / s + 1;
		constexpr size_t out_c = (c - kernel_c) / s + 1;

		size_t i_0 = 0;
		size_t j_0 = 0;

		//change focus of kernel
		for (size_t i = 0; i < r; i += s)
		{
			for (size_t j = 0; j < c; j += s)
			{
				//iterate over kernel
				float sum = 0;
				float out = output.at(i_0, j_0);
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						kernel_hessian.at(N - n, M - m) += gamma * out * (i < 0 || i >= r || j < 0 || j >= c ? 0 : input.at(i - n, j - m) * input.at(i - n, j - m));
				j_0++;
			}
			j_0 = 0;
			++i_0;
		}
	}

	static Matrix2D<float, r, c> convolve_back(Matrix2D<float, r, c> input, Matrix2D<float, kernel_r, kernel_c> kernel)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		Matrix2D<float, r, c> output = { 0 };

		size_t times_across = 0;
		size_t times_down = 0;

		for (size_t i = 0; i < r; i += s)
		{
			for (size_t j = 0; j < c; j += s)
			{
				//find all possible ways convolved size_to
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						output.at(i - n, j - m) += kernel.at(N - n, M - m) * (i < 0 || i >= r || j < 0 || j >= c ? 0 : input.at(times_down, times_across));
				++times_across;
			}
			times_across = 0;
			++times_down;
		}
		return output;
	}

	static Matrix2D<float, r, c> convolve_back_weights_hessian(Matrix2D<float, r, c>& input, Matrix2D<float, kernel_r, kernel_c>& kernel)
	{
		int N = (kernel_r - 1) / 2;
		int M = (kernel_c - 1) / 2;
		Matrix2D<float, r, c> output = { 0 };

		size_t times_across = 0;
		size_t times_down = 0;

		for (size_t i = 0; i < r; i += s)
		{
			for (size_t j = 0; j < c; j += s)
			{
				//find all possible ways convolved size_to
				for (int n = N; n >= -N; --n)
					for (int m = M; m >= -M; --m)
						output.at(i - n, j - m) += kernel.at(N - n, M - m) * kernel.at(N - n, M - m) * (i < 0 || i >= r || j < 0 || j >= c ? 0 : input.at(times_down, times_across));
				++times_across;
			}
			times_across = 0;
			++times_down;
		}
		return output;
	}
};

//helper functions class
template<size_t feature, size_t row, size_t col> class Layer_Functions
{
public:
	static void chain_activations(FeatureMaps<feature, row, col>& fm, FeatureMaps<feature, row, col>& o_fm, size_t activation)
	{
		for (int f = 0; f < feature; ++f)
			for (int i = 0; i < row; ++i)
				for (int j = 0; j < col; ++j)
					fm[f].at(i, j) *= activation_derivative(o_fm[f].at(i, j), activation);
	}

	static void chain_second_activations(FeatureMaps<feature, row, col>& fm, FeatureMaps<feature, row, col>& o_fm, size_t activation)
	{
		for (int f = 0; f < feature; ++f)
			for (int i = 0; i < row; ++i)
				for (int j = 0; j < col; ++j)
					fm[f].at(i, j) *= activation_second_derivative(o_fm[f].at(i, j), activation);
	}

	static inline float activate(float value, size_t activation)
	{
		if (activation == CNN_FUNC_LINEAR)
			return value;
		else if (activation == CNN_FUNC_LOGISTIC || activation == CNN_FUNC_RBM)
			return value < 5 && value > -5 ? (1 / (1 + exp(-value))) : (value >= 5 ? 1.0f : 0.0f);
		else if (activation == CNN_FUNC_BIPOLARLOGISTIC)
			return value < 5 && value > -5 ? ((2 / (1 + exp(-value))) - 1) : (value >= 5 ? 1.0f : -1.0f);
		else if (activation == CNN_FUNC_TANH)
			return value < 5 && value > -5 ? tanh(value) : (value >= 5 ? 1.0f : -1.0f);
		else if (activation == CNN_FUNC_TANHLECUN)
			return value < 5 && value > -5 ? 1.7159f * tanh(0.66666667f * value) : ((value >= 5 ? 1.7159f : -1.7159f));
		else if (activation == CNN_FUNC_RELU)
			return value > 0 ? value : 0;
	}

	static inline float activation_derivative(float value, size_t activation)
	{
		if (activation == CNN_FUNC_LINEAR)
			return 1;
		else if (activation == CNN_FUNC_LOGISTIC || activation == CNN_FUNC_RBM)
			return value * (1 - value);
		else if (activation == CNN_FUNC_BIPOLARLOGISTIC)
			return (1 + value) * (1 - value) / 2;
		else if (activation == CNN_FUNC_TANH)
			return 1 - value * value;
		else if (activation == CNN_FUNC_TANHLECUN)
			return (0.66666667f / 1.7159f * (1.7159f + value) * (1.7159f - value));
		else if (activation == CNN_FUNC_RELU)
			return value > 0 ? 1.0f : 0.0f;
	}

	static inline float activation_second_derivative(float value, size_t activation)
	{
		if (activation == CNN_FUNC_LINEAR)
			return 0;
		else if (activation == CNN_FUNC_LOGISTIC || activation == CNN_FUNC_RBM)
			return value * (1 - value) * (1 - 2 * value);
		else if (activation == CNN_FUNC_BIPOLARLOGISTIC)
			return (1 + value) * (1 - value) * value / 2;
		else if (activation == CNN_FUNC_TANH)
			return (2 * ((value * value) - 1) * value);
		else if (activation == CNN_FUNC_TANHLECUN)
			return (2 * (0.66666667f * 0.66666667f) / (1.7159f * 1.7159f) * (value + 1.7159f) * (value - 1.7159f) * (value));
		else if (activation == CNN_FUNC_RELU)
			return 0;
	}

	template<size_t f = feat, size_t r = row, size_t c = col>
	static inline void stochastic_sample(FeatureMaps<f, r, c>& data)
	{
		if (activation == CNN_FUNC_RBM)
			for (size_t f = 0; f < f; ++f)
				for (size_t i = 0; i < r; ++i)
					for (size_t j = 0; j < c; ++j)
						data[f].at(i, j) = ((rand() * 1.0f) / RAND_MAX < data[f].at(i, j)) ? 1 : 0;
	}
};

template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding>
class ConvolutionLayer : private Layer_Functions<features, rows, cols>
{
public:
	ConvolutionLayer() = default;

	ConvolutionLayer(float rand_max, float rand_min) //todo: for weight initialization, don't screw up weights with every reinitialization?
	{
		for (size_t k = 0; k < out_features * features; ++k)
		{
			biases.at(k) = Matrix2D<float, 1, 1>(.1f, 0.0f);
			biases_hessian.at(k) = Matrix2D<float, 1, 1>({ 0 });
			weights_hessian.at(k) = Matrix2D<float, kernel_size, kernel_size>(0, 0);
			weights.at(k) = Matrix2D<float, kernel_size, kernel_size>(rand_max, rand_min);
		}

		if (activation_function == CNN_FUNC_RBM)
		{
			for (size_t f = 0; f < features; ++f)
				generative_biases[f] = Matrix2D<float, rows, cols>(.1f, 0.0f);
		}
	}

	~ConvolutionLayer() = default;

	static void feed_forwards(FeatureMaps<out_features, (use_padding ? rows : (rows - kernel_size) / stride + 1), (use_padding ? cols : (cols - kernel_size) / stride + 1)>& output)
	{
		constexpr size_t out_rows = use_padding ? rows : (rows - kernel_size) / stride + 1;
		constexpr size_t out_cols = use_padding ? cols : (cols - kernel_size) / stride + 1;

		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			//sum the kernels
			for (size_t f = 0; f < features; ++f)
			{
				add<float, out_rows, out_cols>(output[f_0],
					conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::convolve(feature_maps[f], weights[f_0 * features + f]));
				if (use_biases)
					for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
						for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
							output[f_0].at(i_0, j_0) += biases[f_0 * features + f].at(0, 0);
			}

			if (activation_function != CNN_FUNC_LINEAR)
			{
				for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
					for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
						output[f_0].at(i_0, j_0) = activate(output[f_0].at(i_0, j_0), activation);
			}
		}
	}

	static void feed_backwards(FeatureMaps<out_features, use_padding ? rows : (rows - kernel_size) / stride + 1, use_padding ? cols : (cols - kernel_size) / stride + 1>& input)
	{
		for (size_t f = 0; f < features; ++f)
		{
			for (size_t f_0 = 0; f_0 < out_features; ++f_0)
			{
				add<float, rows, cols>(feature_maps[f],
					conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::convolve_back(input[f_0], weights[f_0 * features + f]));
			}

			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					if (use_biases && activation_function == CNN_FUNC_RBM)
						feature_maps[f].at(i, j) += generative_biases[f].at(i, j);
					feature_maps[f].at(i, j) = activate(feature_maps[f].at(i, j), activation_function);
				}
			}
		}
	}

	static void wake_sleep(float& learning_rate, size_t markov_iterations, bool use_dropout)
	{
		constexpr size_t out_rows = use_padding ? rows : (rows - kernel_size) / stride + 1;
		constexpr size_t out_cols = use_padding ? cols : (cols - kernel_size) / stride + 1;

		//find difference via gibbs sampling
		FeatureMaps<features, rows, cols> original = { 0 };
		for (size_t f = 0; f < features; ++f)
			original[f] = feature_maps[f].clone();

		FeatureMaps<out_features, out_rows, out_cols> discriminated = { 0 };
		FeatureMaps<out_features, out_rows, out_cols> reconstructed = { 0 };

		//Sample, but don't "normalize" second time
		feed_forwards(discriminated);
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
			reconstructed[f_0] = discriminated[f_0].clone();
		stochastic_sample<out_features, out_rows, out_cols>(reconstructed);
		feed_backwards(reconstructed);
		if (!mean_field)
			stochastic_sample(feature_maps);
		feed_forwards(reconstructed);
		for (size_t its = 1; its < markov_iterations; ++its)
		{
			stochastic_sample<out_features, out_rows, out_cols>(reconstructed);
			feed_backwards(reconstructed);
			if (!mean_field)
				stochastic_sample(feature_maps);
			feed_forwards(reconstructed);
		}

		constexpr size_t N = (kernel_size - 1) / 2;

		if (!mean_field)
			stochastic_sample<out_features, out_rows, out_cols>(discriminated);

		//adjust weights
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t f = 0; f < features; ++f)
			{
				size_t i = 0;
				size_t j = 0;

				if (!use_padding)
				{
					i = N;
					j = N;
				}

				for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
				{
					for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
					{
						for (int n = N; n >= -N; --n)
						{
							for (int m = N; m >= -N; --m)
							{
								float delta_weight = reconstructed[f_0].at(i_0, j_0) * feature_maps[f].at(i, j) - discriminated[f_0].at(i_0, j_0) * original[f].at(i, j);
								weights[f_0 * features + f].at(N - n, N - m) += -learning_rate * delta_weight;
							}
						}
						j += stride;
					}
					j = use_padding ? 0 : N;
					i += stride;
				}
			}

			//adjust hidden biases
			if (use_biases)
				for (size_t i_0 = 0; i_0 < biases[f_0].rows(); ++i_0)
					for (size_t j_0 = 0; j_0 < biases[f_0].cols(); ++j_0)
						biases[f_0].at(i_0, j_0) += -learning_rate * (reconstructed[f_0].at(i_0, j_0) - discriminated[f_0].at(i_0, j_0));
		}

		//adjust visible biases
		if (use_biases && activation_function == CNN_FUNC_RBM)
			for (size_t f = 0; f < features; ++f)
				for (size_t i = 0; i < rows; ++i)
					for (size_t j = 0; j < cols; ++j)
						generative_biases[f].at(i, j) += -learning_rate * (feature_maps[f].at(i, j) - original[f].at(i, j));
	}

	static void back_prop(size_t previous_layer_activation, FeatureMaps<out_features, (use_padding ? rows : (rows - kernel_size) / stride + 1), (use_padding ? cols : (cols - kernel_size) / stride + 1)>& deriv, bool online, float learning_rate, bool use_hessian, float mu, bool use_momentum, float momentum_term, bool use_l2_weight_decay, bool include_biases_decay, float weight_decay_factor)
	{
		constexpr size_t out_rows = use_padding ? rows : (rows - kernel_size) / stride + 1;
		constexpr size_t out_cols = use_padding ? cols : (cols - kernel_size) / stride + 1;

		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
		{
			temp[f] = feature_maps[f].clone();
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = 0;
		}

		//adjust gradients and update features
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t f = 0; f < features; ++f)
			{
				//update deltas
				add<float, rows, cols>(feature_maps[f],
					conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::convolve_back(deriv[f_0], weights[f_0 * features + f]));

				//adjust the gradient
				conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::back_prop_kernel(temp[f], deriv[f_0], weights_gradient[f_0 * features + f]);

				//L2 weight decay
				if (use_l2_weight_decay && online)
					for (size_t i = 0; i < kernel_size; ++i)
						for (size_t j = 0; j < kernel_size; ++j)
							weights_gradient[f_0 * features + f].at(i, j) += weights[f_0 * features + f].at(i, j);

				//apply hessian adjustment
				if (use_hessian && online)
					for (size_t i = 0; i < kernel_size; ++i)
						for (size_t j = 0; j < kernel_size; ++j)
							weights_gradient[f_0 * features + f].at(i, j) /= (weights_hessian[f_0 * features + f].at(i, j) + mu);

				if (use_biases)
				{
					//normal derivative
					for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
						for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
							biases_gradient[f_0 * features + f].at(0, 0) += deriv[f_0].at(i_0, j_0);

					//l2 weight decay
					if (use_l2_weight_decay && include_biases_decay && online)
						biases_gradient[f_0 * features + f].at(0, 0) += 2 * weight_decay_factor * biases[f_0 * features + f].at(0, 0);

					//apply hessian adjustment
					if (use_hessian && online)
						biases_gradient[f_0 * features + f].at(0, 0) /= (biases_hessian[f_0 * features + f].at(0, 0) + mu);
				}

				//update for online
				if (use_momentum && online)
				{
					for (size_t i = 0; i < kernel_size; ++i)
					{
						for (size_t j = 0; j < kernel_size; ++j)
						{
							weights[f_0 * features + f].at(i, j) += -learning_rate * (weights_gradient[f_0 * features + f].at(i, j)
								+ momentum_term * weights_momentum[f_0 * features + f].at(i, j));
							weights_momentum[f_0 * features + f].at(i, j) = momentum_term * weights_momentum[f_0 * features + f].at(i, j)
								+ weights_gradient[f_0 * features + f].at(i, j);
							weights_gradient[f_0 * features + f].at(i, j) = 0;
						}
					}

					if (use_biases)
					{
						biases[f_0 * features + f].at(0, 0) += -learning_rate * (biases_gradient[f_0 * features + f].at(0, 0)
							+ momentum_term * biases_momentum[f_0 * features + f].at(0, 0));
						biases_momentum[f_0 * features + f].at(0, 0) = momentum_term * biases_momentum[f_0 * features + f].at(0, 0)
							+ biases_gradient[f_0 * features + f].at(0, 0);
						biases_gradient[f_0 * features + f].at(0, 0) = 0;
					}
				}

				else if (online)
				{
					for (size_t i = 0; i < kernel_size; ++i)
					{
						for (size_t j = 0; j < kernel_size; ++j)
						{
							weights[f_0 * features + f].at(i, j) += -learning_rate * weights_gradient[f_0 * features + f].at(i, j);
							weights_gradient[f_0 * features + f].at(i, j) = 0;
						}
					}

					if (use_biases)
					{
						biases[f_0 * features + f].at(0, 0) += -learning_rate * biases_gradient[f_0 * features + f].at(0, 0);
						biases_gradient[f_0 * features + f].at(0, 0) = 0;
					}
				}
			}
		}

		//apply derivatives
		chain_activations(feature_maps, temp, previous_layer_activation);
	}

	static void back_prop_second(size_t previous_layer_activation, FeatureMaps<out_features, (use_padding ? rows : (rows - kernel_size) / stride + 1), (use_padding ? cols : (cols - kernel_size) / stride + 1)>& deriv, FeatureMaps<out_features, (use_padding ? rows : (rows - kernel_size) / stride + 1), (use_padding ? cols : (cols - kernel_size) / stride + 1)>& deriv_first_in, FeatureMaps<features, rows, cols>& deriv_first_out, bool use_first_deriv, float gamma)
	{
		constexpr size_t out_rows = use_padding ? rows : (rows - kernel_size) / stride + 1;
		constexpr size_t out_cols = use_padding ? cols : (cols - kernel_size) / stride + 1;

		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
		{
			temp[f] = feature_maps[f].clone();
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = 0;
		}

		//adjust by 1 - gamma
		for (size_t d = 0; d < features * out_features; ++d)
			for (size_t i = 0; i < kernel_size; ++i)
				for (size_t j = 0; j < kernel_size; ++j)
					weights_hessian[d].at(i, j) *= (1 - gamma);
		if (use_biases)
			for (size_t d = 0; d < features * out_features; ++d)
				biases_hessian.at(d).at(0, 0) *= (1 - gamma);

		//adjust gradients and update features
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			//update hessian
			for (size_t f = 0; f < features; ++f)
			{
				conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::back_prop_kernel_hessian(feature_maps[f], deriv[f_0],
					weights_hessian[f_0 * features + f], gamma);

				if (use_biases)
					for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
						for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
							biases_hessian[f_0 * features + f].at(0, 0).at(0, 0) += gamma * deriv[f_0].at(i_0, j_0);
			}
		}

		//update deltas, feed backwards except w^2
		for (size_t f = 0; f < features; ++f)
		{
			for (size_t f_0 = 0; f_0 < out_features; ++f_0)
			{
				add<float, rows, cols>(feature_maps[f],
					conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::convolve_back_weights_hessian(deriv[f_0], weights[f_0 * features + f]));
				if (use_first_deriv)
				{
					add<float, rows, cols>(deriv_first_out[f],
						conv_helper_funcs<rows, cols, kernel_size, kernel_size, stride, use_padding>::convolve_back(deriv_first_in[f_0], weights[f_0 * features + f]));
				}
			}
		}

		//apply derivatives
		for (size_t f = 0; f < features; ++f)
		{
			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					float deriv_temp = activation_derivative(temp[f].at(i, j), activation_function);
					feature_maps[f].at(i, j) *= deriv_temp * deriv_temp;

					if (use_first_deriv)
					{
						feature_maps[f].at(i, j) += deriv_first_out[f].at(i, j) * activation_second_derivative(temp[f].at(i, j), activation_function);
						deriv_first_out[f].at(i, j) *= deriv_temp;
					}
				}
			}
		}
	}

	static constexpr size_t type = CNN_LAYER_CONVOLUTION;
	static constexpr size_t activation = activation_function;

	static bool mean_field;

	static FeatureMaps<features, rows, cols> feature_maps;
	static FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> biases;
	static FeatureMaps<out_features * features, kernel_size, kernel_size> weights;

	static FeatureMaps<((use_biases && activation_function == CNN_FUNC_RBM) ? features : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? rows : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? cols : 0)> generative_biases;
	static FeatureMaps<out_features * features, kernel_size, kernel_size> weights_hessian;
	static FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> biases_hessian;

	static FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> biases_gradient;
	static FeatureMaps<out_features * features, kernel_size, kernel_size> weights_gradient;

	static FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> biases_momentum;
	static FeatureMaps<out_features * features, kernel_size, kernel_size> weights_momentum;

	static FeatureMaps<features, rows, cols> activations_mean;
	static FeatureMaps<features, rows, cols> activations_variance;
};

//initialize static todo add custom
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> bool ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::mean_field = false;
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<features, rows, cols> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::feature_maps = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::biases = { 0, .1f };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<out_features * features, kernel_size, kernel_size> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::weights = { -.1f, .1f };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<((use_biases && activation_function == CNN_FUNC_RBM) ? features : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? rows : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? cols : 0)> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::generative_biases = { 0, .1f };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<out_features * features, kernel_size, kernel_size> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::weights_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::biases_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::biases_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<out_features * features, kernel_size, kernel_size> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::weights_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<(use_biases ? out_features * features : 0), (use_biases ? 1 : 0), (use_biases ? 1 : 0)> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::biases_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<out_features * features, kernel_size, kernel_size> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::weights_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<features, rows, cols> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::activations_mean = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t kernel_size, size_t stride, size_t out_features, size_t activation_function, bool use_biases, bool use_padding> FeatureMaps<features, rows, cols> ConvolutionLayer<index, features, rows, cols, kernel_size, stride, out_features, activation_function, use_biases, use_padding>::activations_variance = { 0 };

template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases>
class PerceptronFullConnectivityLayer : private Layer_Functions<features, rows, cols>
{
public:
	PerceptronFullConnectivityLayer()
	{
		if (use_biases)
			for (size_t k = 0; k < out_features; ++k)
				biases[k] = Matrix2D<float, out_rows, out_cols>(.1f, 0.0f);

		//gaussian distributed
		for (size_t i = 0; i < out_rows * out_cols * out_features; ++i)
			for (size_t j = 0; j < rows * cols * features; ++j)
				weights[0].at(i, j) = sqrt(-2 * log(1.0f * (rand() + 1) / (RAND_MAX))) * sin(2 * 3.14152f * rand() / RAND_MAX) *.1f;

		if (activation == CNN_FUNC_RBM && use_biases)
			for (size_t f = 0; f < features; ++f)
				generative_biases[f] = Matrix2D<float, rows, cols>(.1f, 0.0f);
	}

	PerceptronFullConnectivityLayer(float rand_max, float rand_min)
	{
		if (use_biases)
			for (size_t k = 0; k < out_features; ++k)
				biases[k] = Matrix2D<float, out_rows, out_cols>(.1f, 0.0f);

		weights[0] = Matrix2D<float, out_rows * out_cols * out_features, rows * cols * features>(rand_max, rand_min);

		if (activation == CNN_FUNC_RBM && use_biases)
			for (size_t f = 0; f < features; ++f)
				generative_biases[f] = Matrix2D<float, rows, cols>(.1f, 0.0f);
	}

	~PerceptronFullConnectivityLayer() = default;

	static void feed_forwards(FeatureMaps<out_features, out_rows, out_cols>& output)
	{
		//loop through every neuron in output
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					//loop through every neuron in input and add it to output
					float sum = 0.0f;
					for (size_t f = 0; f < features; ++f)
						for (size_t i = 0; i < rows; ++i)
							for (size_t j = 0; j < cols; ++j)
								sum += (feature_maps[f].at(i, j) *
									weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j));

					//add bias
					if (use_biases)
						output[f_0].at(i_0, j_0) = activate(sum + biases[f_0].at(i_0, j_0), activation_function);
					else
						output[f_0].at(i_0, j_0) = activate(sum, activation_function);
				}
			}
		}
	}

	static void feed_backwards(FeatureMaps<out_features, out_rows, out_cols>& input)
	{
		//go through every neuron in this layer
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t f = 0; f < features; ++f)
			{
				for (size_t i = 0; i < rows; ++i)
				{
					for (size_t j = 0; j < cols; ++j)
					{
						//go through every neuron in output layer and add it to this neuron
						float sum = 0.0f;
						for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
						{
							for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
							{
								sum += weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j)
									* input[f_0].at(i_0, j_0);
							}
						}
						if (use_biases && activation_function == CNN_FUNC_RBM)
							sum += generative_biases[f].at(i, j);
						feature_maps[f].at(i, j) = activate(sum, activation_function);
					}
				}
			}
		}
	}

	static void wake_sleep(float& learning_rate, size_t markov_iterations, bool use_dropout)
	{
		//find difference via gibbs sampling
		FeatureMaps<features, rows, cols> original = { 0 };

		FeatureMaps<out_features, out_rows, out_cols> discriminated = { 0 };

		FeatureMaps<out_features, out_rows, out_cols> reconstructed = { 0 };

		//Sample, but don't "normalize" second time
		feed_forwards(discriminated);
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
			reconstructed[f_0] = discriminated[f_0].clone();
		stochastic_sample<out_features, out_rows, out_cols>(reconstructed);
		feed_backwards(reconstructed);
		if (!mean_field)
			stochastic_sample(feature_maps);
		feed_forwards(reconstructed);
		for (size_t its = 1; its < markov_iterations; ++its)
		{
			stochastic_sample<out_features, out_rows, out_cols>(reconstructed);
			feed_backwards(reconstructed);
			if (!mean_field)
				stochastic_sample(feature_maps);
			feed_forwards(reconstructed);
		}

		if (!mean_field)
			stochastic_sample<out_features, out_rows, out_cols>(discriminated);

		//adjust weights
		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					for (size_t f = 0; f < features; ++f)
					{
						for (size_t i = 0; i < rows; ++i)
						{
							for (size_t j = 0; j < cols; ++j)
							{
								float delta_weight = reconstructed[f_0].at(i_0, j_0) * feature_maps[f].at(i, j) - discriminated[f_0].at(i_0, j_0) * original[f].at(i, j);
								weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) += -learning_rate * delta_weight;
							}
						}
					}
				}
			}

			//adjust hidden biases
			if (use_biases)
				for (size_t i_0 = 0; i_0 < biases[f_0].rows(); ++i_0)
					for (size_t j_0 = 0; j_0 < biases[f_0].cols(); ++j_0)
						biases[f_0].at(i_0, j_0) += -learning_rate * (reconstructed[f_0].at(i_0, j_0) - discriminated[f_0].at(i_0, j_0));
		}

		//adjust visible biases
		if (use_biases && activation_function == CNN_FUNC_RBM)
			for (size_t f = 0; f < features; ++f)
				for (size_t i = 0; i < rows; ++i)
					for (size_t j = 0; j < cols; ++j)
						generative_biases[f].at(i, j) += -learning_rate * (feature_maps[f].at(i, j) - original[f].at(i, j));
	}

	static void back_prop(size_t previous_layer_activation, FeatureMaps<out_features, out_rows, out_cols>& deriv, bool online, float learning_rate, bool use_hessian, float mu, bool use_momentum, float momentum_term, bool use_l2_weight_decay, bool include_biases_decay, float weight_decay_factor)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
		{
			temp[f] = feature_maps[f].clone();
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = 0;
		}

		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					if (use_biases)
					{
						//normal derivative
						biases_gradient[f_0].at(i_0, j_0) += deriv[f_0].at(i_0, j_0);

						//L2 weight decay
						if (use_l2_weight_decay && include_biases_decay && online)
							biases_gradient[f_0].at(i_0, j_0) += 2 * weight_decay_factor * biases[f_0].at(i_0, j_0);

						//hessian
						if (use_hessian && online)
							biases_gradient[f_0].at(i_0, j_0) /= (biases_hessian[f_0].at(i_0, j_0) + mu);

						//online update
						if (use_momentum && online)
						{
							biases[f_0].at(i_0, j_0) += -learning_rate * (biases_gradient[f_0].at(i_0, j_0) + momentum_term * biases_momentum[f_0].at(i_0, j_0));
							biases_momentum[f_0].at(i_0, j_0) = momentum_term * biases_momentum[f_0].at(i_0, j_0) + biases_gradient[f_0].at(i_0, j_0);
							biases_gradient[f_0].at(i_0, j_0) = 0;
						}

						else if (online)
						{
							biases[f_0].at(i_0, j_0) += -learning_rate * biases_gradient[f_0].at(i_0, j_0);
							biases_gradient[f_0].at(i_0, j_0) = 0;
						}
					}

					for (size_t f = 0; f < features; ++f)
					{
						for (size_t i = 0; i < rows; ++i)
						{
							for (size_t j = 0; j < cols; ++j)
							{
								//update deltas
								feature_maps[f].at(i, j) += deriv[f_0].at(i_0, j_0)
									* weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j);

								//normal derivative
								weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) += deriv[f_0].at(i_0, j_0) * temp[f].at(i, j);

								//L2 decay
								if (use_l2_weight_decay && online)
									weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) +=
									2 * weight_decay_factor * weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j);

								//hessian
								if (use_hessian && online)
									weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) /=
									(weights_hessian[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) + mu);

								//Online updates
								if (use_momentum && online)
								{
									weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) +=
										-learning_rate * (weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j)
											+ momentum_term * weights_momentum[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j));
									weights_momentum[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) =
										momentum_term * weights_momentum[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j)
										+ weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j);
									weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) = 0;
								}

								else if (online)
								{
									weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) +=
										-learning_rate * weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j);
									weights_gradient[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) = 0;
								}
							}
						}
					}
				}
			}
		}

		//apply derivatives
		chain_activations(feature_maps, temp, previous_layer_activation);
	}

	static void back_prop_second(size_t previous_layer_activation, FeatureMaps<out_features, out_rows, out_cols>& deriv, FeatureMaps<out_features, out_rows, out_cols>& deriv_first_in, FeatureMaps<features, rows, cols>& deriv_first_out, bool use_first_deriv, float gamma)
	{
		for (size_t i = 0; i < (out_features * out_rows * out_cols); ++i)
			for (size_t j = 0; j < (features * rows * cols); ++j)
				weights_hessian[0].at(i, j) *= (1 - gamma);
		if (use_biases)
			for (size_t f_0 = 0; f_0 < out_features; ++f_0)
				for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
					for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
						biases_hessian[f_0].at(i_0, j_0) *= (1 - gamma);

		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
		{
			temp[f] = feature_maps[f].clone();
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = 0;
		}

		for (size_t f_0 = 0; f_0 < out_features; ++f_0)
		{
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					if (use_biases)
						biases_hessian[f_0].at(i_0, j_0) += gamma * deriv[f_0].at(i_0, j_0);

					for (size_t f = 0; f < features; ++f)
					{
						for (size_t i = 0; i < rows; ++i)
						{
							for (size_t j = 0; j < cols; ++j)
							{
								float w = weights[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j);
								float x = feature_maps[f].at(i, j);

								feature_maps[f].at(i, j) += deriv[f_0].at(i_0, j_0) * w * w;
								weights_hessian[0].at(f_0 * out_rows * out_cols + i_0 * out_cols + j_0, f * rows * cols + i * cols + j) +=
									gamma * deriv[f_0].at(i_0, j_0) * x * x;

								if (use_first_deriv)
									deriv_first_out[f].at(i, j) += deriv_first_in[f_0].at(i_0, j_0) * w;
							}
						}
					}
				}
			}
		}

		//apply derivatives
		for (size_t f = 0; f < features; ++f)
		{
			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					float deriv_temp = activation_derivative(temp[f].at(i, j), activation_function);
					feature_maps[f].at(i, j) *= deriv_temp * deriv_temp;

					if (use_first_deriv)
					{
						feature_maps[f].at(i, j) += deriv_first_out[f].at(i, j) * activation_second_derivative(temp[f].at(i, j), activation_function);
						deriv_first_out[f].at(i, j) *= deriv_temp;
					}
				}
			}
		}
	}

	static constexpr size_t type = CNN_LAYER_PERCEPTRONFULLCONNECTIVITY;
	static constexpr size_t activation = activation_function;

	static bool mean_field;

	static FeatureMaps<features, rows, cols> feature_maps;
	static FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> biases;
	static FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> weights;

	static FeatureMaps<((use_biases && activation_function == CNN_FUNC_RBM) ? features : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? rows : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? cols : 0)> generative_biases;
	static FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> weights_hessian;
	static FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> biases_hessian;

	static FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> biases_gradient;
	static FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> weights_gradient;

	static FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> biases_momentum;
	static FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> weights_momentum;

	static FeatureMaps<features, rows, cols> activations_mean;
	static FeatureMaps<features, rows, cols> activations_variance;
};

//static variable initialization todo: add custom weight?
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> bool PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::mean_field = false;
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<features, rows, cols> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::feature_maps = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::biases = { 0, .1f };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::weights = { -.1f, .1f };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<((use_biases && activation_function == CNN_FUNC_RBM) ? features : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? rows : 0), ((use_biases && activation_function == CNN_FUNC_RBM) ? cols : 0)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::generative_biases = { 0, .1f };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::weights_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::biases_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::biases_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::weights_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<(use_biases ? out_features : 0), (use_biases ? out_rows : 0), (use_biases ? out_cols : 0)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::biases_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<1, (out_features * out_rows * out_cols), (features * rows * cols)> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::weights_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<features, rows, cols> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::activations_mean = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_features, size_t out_rows, size_t out_cols, size_t activation_function, bool use_biases> FeatureMaps<features, rows, cols> PerceptronFullConnectivityLayer<index, features, rows, cols, out_features, out_rows, out_cols, activation_function, use_biases>::activations_variance = { 0 };

template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols>
class MaxpoolLayer : private Layer_Functions<features, rows, cols>
{
public:
	MaxpoolLayer() = default;

	~MaxpoolLayer() = default;

	static void feed_forwards(FeatureMaps<features, out_rows, out_cols>& output)
	{
		for (size_t f_0 = 0; f_0 < features; ++f_0)
			for (size_t i = 0; i < output[f_0].rows(); ++i)
				for (size_t j = 0; j < output[f_0].cols(); ++j)
					output[f_0].at(i, j) = -INFINITY;

		for (size_t f = 0; f < features; ++f)
		{
			constexpr size_t down = rows / out_rows;
			constexpr size_t across = cols / out_cols;
			Matrix2D<Matrix2D<float, down, across>, out_rows, out_cols> samples;

			//get samples
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					//get the current sample
					size_t maxI = (i_0 + 1) * down;
					size_t maxJ = (j_0 + 1) * across;
					for (size_t i2 = i_0 * down; i2 < maxI; ++i2)
					{
						for (size_t j2 = j_0 * across; j2 < maxJ; ++j2)
						{
							samples.at(i_0, j_0).at(maxI - i2 - 1, maxJ - j2 - 1) = feature_maps[f].at(i2, j2);
						}
					}
				}
			}

			//find maxes
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					for (size_t n = 0; n < samples.at(i_0, j_0).rows(); ++n)
					{
						for (size_t m = 0; m < samples.at(i_0, j_0).cols(); ++m)
						{
							if (samples.at(i_0, j_0).at(n, m) > output[f].at(i_0, j_0))
							{
								output[f].at(i_0, j_0) = samples.at(i_0, j_0).at(n, m);
								switches[f].at(i_0, j_0) = std::make_pair(n, m);
							}
						}
					}
				}
			}
		}
	}

	static void feed_backwards(FeatureMaps<features, out_rows, out_cols>& input)
	{
		for (size_t f = 0; f < features; ++f)
		{
			size_t down = rows / out_rows;
			size_t across = cols / out_cols;

			//search each sample
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					std::pair<size_t, size_t> coords = switches[f].at(i_0, j_0);
					for (size_t i = 0; i < down; ++i)
					{
						for (size_t j = 0; j < across; ++j)
						{
							if (i == coords.first && j == coords.second)
								feature_maps[f].at(i_0 * down + i, j_0 * across + j) = input[f].at(i_0, j_0);
							else
								feature_maps[f].at(i * down, j * across) = 0;
						}
					}
				}
			}
		}
	}

	static void back_prop(size_t previous_layer_activation, FeatureMaps<features, out_rows, out_cols>& deriv, bool online, float learning_rate, bool use_hessian, float mu, bool use_momentum, float momentum_term, bool use_l2_weight_decay, bool include_biases_decay, float weight_decay_factor)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		//just move the values back to which ones were passed on
		for (size_t f = 0; f < features; ++f)
		{
			size_t down = rows / out_rows;
			size_t across = cols / out_cols;

			//search each sample
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					std::pair<size_t, size_t> coords = switches[f].at(i_0, j_0);
					for (size_t i = 0; i < down; ++i)
					{
						for (size_t j = 0; j < across; ++j)
						{
							if (i == coords.first && j == coords.second)
								feature_maps[f].at(i_0 * down + i, j_0 * across + j) = deriv[f].at(i_0, j_0);
							else
								feature_maps[f].at(i * down, j * across) = 0;
						}
					}
				}
			}
		}

		//apply derivatives
		chain_activations(feature_maps, temp, previous_layer_activation);
	}

	static void back_prop_second(size_t previous_layer_activation, FeatureMaps<features, out_rows, out_cols>& deriv, FeatureMaps<features, out_rows, out_cols>& deriv_first_in, FeatureMaps<features, rows, cols>& deriv_first_out, bool use_first_deriv, float gamma)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		//just move the values back to which ones were passed on
		for (size_t f = 0; f < features; ++f)
		{
			size_t down = rows / out_rows;
			size_t across = cols / out_cols;

			//search each sample
			for (size_t i_0 = 0; i_0 < out_rows; ++i_0)
			{
				for (size_t j_0 = 0; j_0 < out_cols; ++j_0)
				{
					std::pair<size_t, size_t> coords = switches[f].at(i_0, j_0);
					for (size_t i = 0; i < down; ++i)
					{
						for (size_t j = 0; j < across; ++j)
						{
							if (i == coords.first && j == coords.second)
							{
								feature_maps[f].at(i_0 * down + i, j_0 * across + j) = deriv[f].at(i_0, j_0);
								if (use_first_deriv)
									deriv_first_out[f].at(i, j) = deriv_first_in[f].at(i_0, j_0);
							}
							else
							{
								feature_maps[f].at(i * down, j * across) = 0;
								if (use_first_deriv)
									deriv_first_out[f].at(i, j) = 0;
							}
						}
					}
				}
			}
		}

		//apply derivatives
		chain_second_activations(feature_maps, temp, previous_layer_activation);
		if (use_first_deriv)
			chain_activations(deriv_first_out, temp, previous_layer_activation);

	}

	static constexpr size_t type = CNN_LAYER_MAXPOOL;
	static constexpr size_t activation = CNN_FUNC_LINEAR;

	static FeatureMaps<features, rows, cols> feature_maps;
	static FeatureMaps<0, 0, 0> biases;
	static FeatureMaps<0, 0, 0> weights;

	static FeatureMaps<0, 0, 0> generative_biases;
	static FeatureMaps<0, 0, 0> weights_hessian;
	static FeatureMaps<0, 0, 0> biases_hessian;

	static FeatureMaps<0, 0, 0> biases_gradient;
	static FeatureMaps<0, 0, 0> weights_gradient;

	static FeatureMaps<0, 0, 0> biases_momentum;
	static FeatureMaps<0, 0, 0> weights_momentum;

	static FeatureMaps<features, rows, cols> activations_mean;
	static FeatureMaps<features, rows, cols> activations_variance;

private:
	static FeatureMaps<features, out_rows, out_cols, std::pair<size_t, size_t>> switches;
};

//init static
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<features, rows, cols> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::feature_maps = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::weights = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::generative_biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::weights_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::biases_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::biases_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::weights_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::biases_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<0, 0, 0> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::weights_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<features, rows, cols> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::activations_mean = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<features, rows, cols> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::activations_variance = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols, size_t out_rows, size_t out_cols> FeatureMaps<features, out_rows, out_cols, std::pair<size_t, size_t>> MaxpoolLayer<index, features, rows, cols, out_rows, out_cols>::switches = {};

template<size_t index, size_t features, size_t rows, size_t cols>
class SoftMaxLayer : private Layer_Functions<features, rows, cols>
{
public:
	SoftMaxLayer() = default;

	~SoftMaxLayer() = default;

	static void feed_forwards(FeatureMaps<features, rows, cols>& output)
	{
		for (size_t f = 0; f < features; ++f)
		{
			//find total
			float sum = 0.0f;
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					sum += feature_maps[f].at(i, j) < 6 ? exp(feature_maps[f].at(i, j)) : 4;

			//get prob
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					output[f].at(i, j) = exp(feature_maps[f].at(i, j)) / sum;
		}
	}

	static void feed_backwards(FeatureMaps<features, rows, cols>& input)
	{
		//assume that the original input has a mean of 0, so sum of original input would *approximately* be the total number of inputs
		size_t total = rows * cols;

		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = log(total * input[f].at(i, j));
	}

	static void back_prop(size_t previous_layer_activation, FeatureMaps<features, rows, cols>& deriv, bool online, float learning_rate, bool use_hessian, float mu, bool use_momentum, float momentum_term, bool use_l2_weight_decay, bool include_biases_decay, float weight_decay_factor)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		for (size_t f = 0; f < features; ++f)
		{
			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					//cycle through all again
					for (size_t i2 = 0; i2 < rows; ++i2)
					{
						for (size_t j2 = 0; j2 < cols; ++j2)
						{
							/*float h_i = data[f].at(i, j);
							float h_j = data[f].at(i2, j2);
							feature_maps[f].at(i, j) += (i2 == i && j2 == j) ? h_i * (1 - h_i) : -h_i * h_j;*///todo: check
						}
					}
				}
			}
		}
		//apply derivatives
		chain_activations(feature_maps, temp, previous_layer_activation);
	}

	static void back_prop_second(size_t previous_layer_activation, FeatureMaps<features, rows, cols>& deriv, FeatureMaps<features, rows, cols>& deriv_first_in, FeatureMaps<features, rows, cols>& deriv_first_out, bool use_first_deriv, float gamma)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		for (size_t f = 0; f < features; ++f)
		{
			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					//cycle through all again
					for (size_t i2 = 0; i2 < rows; ++i2)
					{
						for (size_t j2 = 0; j2 < cols; ++j2)
						{
							/*float h_i = data[f].at(i, j);
							float h_j = data[f].at(i2, j2);
							feature_maps[f].at(i, j) += (i2 == i && j2 == j) ? h_i * (1 - h_i) : -h_i * h_j;*///todo: add second
						}
					}
				}
			}
		}

		//apply derivatives
		chain_second_activations(feature_maps, temp, previous_layer_activation);
		if (use_first_deriv)
			chain_activations(deriv_first_out, temp, previous_layer_activation);
	}

	static constexpr size_t type = CNN_LAYER_SOFTMAX;
	static constexpr size_t activation = CNN_FUNC_LINEAR;

	static FeatureMaps<features, rows, cols> feature_maps;
	static FeatureMaps<0, 0, 0> biases;
	static FeatureMaps<0, 0, 0> weights;

	static FeatureMaps<0, 0, 0> generative_biases;
	static FeatureMaps<0, 0, 0> weights_hessian;
	static FeatureMaps<0, 0, 0> biases_hessian;

	static FeatureMaps<0, 0, 0> biases_gradient;
	static FeatureMaps<0, 0, 0> weights_gradient;

	static FeatureMaps<0, 0, 0> biases_momentum;
	static FeatureMaps<0, 0, 0> weights_momentum;

	static FeatureMaps<features, rows, cols> activations_mean;
	static FeatureMaps<features, rows, cols> activations_variance;
};

//init static
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> SoftMaxLayer<index, features, rows, cols>::feature_maps = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::weights = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::generative_biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::weights_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::biases_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::biases_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::weights_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::biases_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> SoftMaxLayer<index, features, rows, cols>::weights_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> SoftMaxLayer<index, features, rows, cols>::activations_mean = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> SoftMaxLayer<index, features, rows, cols>::activations_variance = { 0 };

template<size_t index, size_t features, size_t rows, size_t cols>
class InputLayer : private Layer_Functions<features, rows, cols>
{
public:
	InputLayer() = default;

	~InputLayer() = default;

	static void feed_forwards(FeatureMaps<features, rows, cols>& output)
	{
		//just output
		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					output[f].at(i, j) = feature_maps[f].at(i, j);
	}

	static void feed_backwards(FeatureMaps<features, rows, cols>& input)
	{
		//just output
		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = input[f].at(i, j);
	}

	static void back_prop(size_t previous_layer_activation, FeatureMaps<features, rows, cols>& deriv, FeatureMaps<0, 0, 0>& weights_momentum, FeatureMaps<0, 0, 0>& biases_momentum, bool online, float learning_rate, bool use_hessian, float mu, bool use_momentum, float momentum_term, bool use_l2_weight_decay, bool include_biases_decay, float weight_decay_factor)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = deriv[f].at(i, j);
		//apply derivatives
		chain_activations(feature_maps, temp, previous_layer_activation);
	}

	static void back_prop_second(size_t previous_layer_activation, FeatureMaps<features, rows, cols>& deriv, FeatureMaps<features, rows, cols>& deriv_first_in, FeatureMaps<features, rows, cols>& deriv_first_out, bool use_first_deriv, float gamma)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		for (size_t f = 0; f < features; ++f)
		{
			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					feature_maps[f].at(i, j) = deriv[f].at(i, j);
					if (use_first_deriv)
						deriv_first_out[f].at(i, j) = deriv_first_in[f].at(i, j);
				}
			}
		}

		//apply derivatives
		chain_second_activations(feature_maps, temp, previous_layer_activation);
		if (use_first_deriv)
			chain_activations(deriv_first_out, temp, previous_layer_activation);
	}

	static constexpr size_t type = CNN_LAYER_INPUT;
	static constexpr size_t activation = CNN_FUNC_LINEAR;

	static FeatureMaps<features, rows, cols> feature_maps;
	static FeatureMaps<0, 0, 0> biases;
	static FeatureMaps<0, 0, 0> weights;

	static FeatureMaps<0, 0, 0> generative_biases;
	static FeatureMaps<0, 0, 0> weights_hessian;
	static FeatureMaps<0, 0, 0> biases_hessian;

	static FeatureMaps<0, 0, 0> biases_gradient;
	static FeatureMaps<0, 0, 0> weights_gradient;

	static FeatureMaps<0, 0, 0> biases_momentum;
	static FeatureMaps<0, 0, 0> weights_momentum;

	static FeatureMaps<features, rows, cols> activations_mean;
	static FeatureMaps<features, rows, cols> activations_variance;
};

//init static
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> InputLayer<index, features, rows, cols>::feature_maps = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::weights = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::generative_biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::weights_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::biases_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::biases_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::weights_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::biases_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> InputLayer<index, features, rows, cols>::weights_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> InputLayer<index, features, rows, cols>::activations_mean = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> InputLayer<index, features, rows, cols>::activations_variance = { 0 };

template<size_t index, size_t features, size_t rows, size_t cols>
class OutputLayer : private Layer_Functions<features, rows, cols>
{
public:
	OutputLayer() = default;

	~OutputLayer() = default;

	static void feed_forwards(FeatureMaps<features, rows, cols>& output)
	{
		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					output[f].at(i, j) = feature_maps[f].at(i, j);
	}

	static void feed_backwards(FeatureMaps<features, rows, cols>& input)
	{
		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = input[f].at(i, j);
	}

	static void back_prop(size_t previous_layer_activation, FeatureMaps<features, rows, cols>& deriv, bool online, float learning_rate, bool use_hessian, float mu, bool use_momentum, float momentum_term, bool use_l2_weight_decay, bool include_biases_decay, float weight_decay_factor)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		for (size_t f = 0; f < features; ++f)
			for (size_t i = 0; i < rows; ++i)
				for (size_t j = 0; j < cols; ++j)
					feature_maps[f].at(i, j) = deriv[f].at(i, j);
		//apply derivatives
		chain_activations(feature_maps, temp, previous_layer_activation);
	}

	static void back_prop_second(size_t previous_layer_activation, FeatureMaps<features, rows, cols>& deriv, FeatureMaps<features, rows, cols>& deriv_first_in, FeatureMaps<features, rows, cols>& deriv_first_out, bool use_first_deriv, float gamma)
	{
		FeatureMaps<features, rows, cols> temp = { 0 };
		for (int f = 0; f < features; ++f)
			temp[f] = feature_maps[f].clone();

		for (size_t f = 0; f < features; ++f)
		{
			for (size_t i = 0; i < rows; ++i)
			{
				for (size_t j = 0; j < cols; ++j)
				{
					feature_maps[f].at(i, j) = deriv[f].at(i, j);
					if (use_first_deriv)
						deriv_first_out[f].at(i, j) = deriv_first_in[f].at(i, j);
				}
			}
		}
		//apply derivatives
		chain_second_activations(feature_maps, temp, previous_layer_activation);
		if (use_first_deriv)
			chain_activations(deriv_first_out, temp, previous_layer_activation);
	}

	static constexpr size_t type = CNN_LAYER_OUTPUT;
	static constexpr size_t activation = CNN_FUNC_LINEAR;

	static FeatureMaps<features, rows, cols> feature_maps;
	static FeatureMaps<0, 0, 0> biases;
	static FeatureMaps<0, 0, 0> weights;

	static FeatureMaps<0, 0, 0> generative_biases;
	static FeatureMaps<0, 0, 0> weights_hessian;
	static FeatureMaps<0, 0, 0> biases_hessian;

	static FeatureMaps<0, 0, 0> biases_gradient;
	static FeatureMaps<0, 0, 0> weights_gradient;

	static FeatureMaps<0, 0, 0> biases_momentum;
	static FeatureMaps<0, 0, 0> weights_momentum;

	static FeatureMaps<features, rows, cols> activations_mean;
	static FeatureMaps<features, rows, cols> activations_variance;
};

//init static
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> OutputLayer<index, features, rows, cols>::feature_maps = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::weights = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::generative_biases = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::weights_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::biases_hessian = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::biases_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::weights_gradient = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::biases_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<0, 0, 0> OutputLayer<index, features, rows, cols>::weights_momentum = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> OutputLayer<index, features, rows, cols>::activations_mean = { 0 };
template<size_t index, size_t features, size_t rows, size_t cols> FeatureMaps<features, rows, cols> OutputLayer<index, features, rows, cols>::activations_variance = { 0 };