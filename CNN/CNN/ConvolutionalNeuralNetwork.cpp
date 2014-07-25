#include "ConvolutionalNeuralNetwork.h"

convolutional_neural_network::convolutional_neural_network()
{
}

convolutional_neural_network::~convolutional_neural_network()
{
}

matrix<float> convolutional_neural_network::convolve(matrix<float> input_matrix, matrix<float> kernal)
{
	int M = (kernal.cols - 1) / 2;
	int N = (kernal.rows - 1) / 2;
	matrix<float> result(input_matrix.cols - (2 * M), input_matrix.rows - (2 * N), 1);

	for (int k = 0; k < input_matrix.dims; ++k)
	{
		matrix<float> current(input_matrix.cols - (2 * M), input_matrix.rows - (2 * N), 1);
		//apply to every pixel
		for (int i = M; i < input_matrix.rows - N; ++i)
		{
			for (int j = N; j < input_matrix.cols - M; ++j)
			{
				//find sum
				float sum = 0.0f;
				for (int m = -M; m <= M; ++m)
				for (int n = -N; n <= N; ++n)
					sum += (input_matrix.at(i + m, j + n, k) * kernal.at(M + m, N + n, 0));
				current.set(i - M, j - N, 0, sum);
			}
		}

		//add channels
		for (int i = 0; i < result.rows; ++i)
		for (int j = 0; j < result.cols; ++j)
			result.set(i, j, 0, result.at(i, j, 0) + current.at(i, j, 0));
	}
	return result;
}

matrix<float> convolutional_neural_network::deconvolve(matrix<float> input_matrix, matrix<float> kernal)
{
	int N = (kernal.cols - 1) / 2;
	int M = (kernal.rows - 1) / 2;
	matrix<float> result(input_matrix.cols + (2 * N), input_matrix.rows + (2 * N), 1, INFINITY);

	matrix<float> top_left = deconvolve_single(input_matrix.at(0, 0, 0), kernal);
	for (int i = 0; i < kernal.rows; ++i)
	for (int j = 0; j< kernal.cols; ++j)
	if (top_left.at(i, j, 0) != 0)
		result.set(i, j, 0, top_left.at(i, j, 0));

	for (int i = 0; i < input_matrix.rows; ++i)
	{
		for (int j = 1; j < input_matrix.cols; ++j)
		{
			matrix<float> current_matrix = deconvolve_single(input_matrix.at(i, j, 0), kernal);
			matrix<float> new_kernal = kernal;

			float overlap_sum = 0.0f;
			int new_i = i + N;
			int new_j = j + M;
			int n = 0;
			int m = 0;

			for (int i2 = new_i - N; i2 <= new_i + N; ++i2)
			{
				for (int j2 = new_j - M; j2 <= new_j + M; ++j2)
				{
					if (result.at(i2, j2, 0) != INFINITY)
					{
						overlap_sum += kernal.at(n, m, 0) * result.at(i2, j2, 0);
						new_kernal.set(n, m, 0, 0);
					}
					++m;
				}
				m = 0;
				++n;
			}

			matrix<float> new_matrix = deconvolve_single(input_matrix.at(i, j, 0) - overlap_sum, new_kernal);

			n = 0;
			m = 0;
			for (int i2 = new_i - N; i2 <= new_i + N; ++i2)
			{
				for (int j2 = new_j - M; j2 <= new_j + M; ++j2)
				{
					if (new_matrix.at(n, m, 0) != 0)
						result.set(i2, j2, 0, new_matrix.at(n, m, 0));
					++m;
				}
				m = 0;
				++n;
			}
		}
	}

	for (int i = 0; i < result.rows; ++i)
	for (int j = 0; j < result.cols; ++j)
	if (result.at(i, j, 0) == INFINITY)
		result.set(i, j, 0, 0);

	return result;
}

matrix<float> convolutional_neural_network::deconvolve_single(float input_value, matrix<float> kernal)
{
	int N = (kernal.cols - 1) / 2;
	int M = (kernal.rows - 1) / 2;
	int symmetry = 0;

	matrix<float> result(kernal.cols, kernal.rows, 1);
	std::vector<float> p;
	std::vector<float> s;

	std::vector<std::pair<int, int>> matched;

	for (int i = 0; i < kernal.rows; ++i)
	{
		for (int j = 0; j < kernal.cols; ++j)
		{
			std::pair<int, int> coords(i, j);

			std::pair<int, int> matched_coords;
			if (kernal.at(i, j, 0) == -kernal.at(kernal.cols - 1 - j, kernal.rows - 1 - i, 0))//bottom left to right diagonal
			{
				symmetry = 1;
				matched_coords = std::pair<int, int>(kernal.cols - i - j, kernal.rows - 1 - i);
			}
			else if (kernal.at(i, j, 0) == -kernal.at(j, kernal.rows - 1 - i, 0))//bottom right to left diagonal
			{
				symmetry = 2;
				matched_coords = std::pair<int, int>(j, kernal.rows - 1 - i);
			}
			else if (kernal.at(i, j, 0) == -kernal.at(i, kernal.rows - 1 - j, 0))//across
			{
				symmetry = 3;
				matched_coords = std::pair<int, int>(i, kernal.rows - 1 - j);
			}
			else if (kernal.at(i, j, 0) == -kernal.at(kernal.cols - 1 - i, j, 0))//up and down
			{
				symmetry = 4;
				matched_coords = std::pair<int, int>(kernal.cols - 1 - i, j);
			}

			bool matched_before = false;
			for (int l = 0; l < matched.size(); ++l)
			{
				if (matched[l] == matched_coords || matched[l] == coords)
				{
					matched_before = true;
					break;
				}
			}

			if (symmetry != 0 && kernal.at(i, j, 0) != 0 && !matched_before &&
				((kernal.at(i, j, 0) > 0 && input_value > 0) || (kernal.at(i, j, 0) < 0 && input_value < 0)))
			{
				p.push_back(kernal.at(i, j, 0));
				matched.push_back(matched_coords);
				matched.push_back(coords);
			}

			if ((kernal.at(i, j, 0) > 0 && input_value > 0) || (kernal.at(i, j, 0) < 0 && input_value < 0))
				s.push_back(kernal.at(i, j, 0));
			symmetry = 0;
		}
	}

	float sum_P = 0.0f;
	float sum_S = 0.0f;

	for (int i = 0; i < p.size(); ++i)
		sum_P += abs(p[i]);
	for (int i = 0; i < s.size(); ++i)
		sum_S += abs(s[i]);

	//p
	float P = sum_P / sum_S;

	//k
	std::vector<float> k;
	for (int i = 0; i < kernal.rows; ++i)
	for (int j = 0; j < kernal.cols; ++j)
		k.push_back(kernal.at(i, j, 0));
	//m_n
	std::vector<float> m(k.size());
	for (int n = 0; n < k.size(); ++n)
	{
		bool in_p = false;
		bool in_s = false;

		for (int i = 0; i < p.size(); ++i)
		{
			if (p[i] == k[n])
			{
				in_p = true;
				break;
			}
		}

		for (int i = 0; i < s.size(); ++i)
		{
			if (s[i] == k[n])
			{
				in_s = true;
				break;
			}
		}

		//algorithms
		if (k[n] == 0)
			m[n] = 0;
		else if (in_p && in_s)
			m[n] = abs(((P * input_value) / p.size()) / k[n]);
		else if (!in_p && in_s)
			m[n] = abs(((1 - P) * input_value) / k[n]);
		else if (!in_s)
			m[n] = abs(1 / k[n]);
	}

	float sum = 0.0f;
	for (int i = 0; i < k.size(); ++i)
		sum += k[i] * m[i];

	float C = input_value / sum;

	int n = 0;
	for (int i = 0; i < result.rows; ++i)
	{
		for (int j = 0; j < result.cols; ++j)
		{
			result.set(i, j, 0, abs(m[n] * C));
			++n;
		}
	}

	return result;
}

matrix<float> convolutional_neural_network::feed_forward(layer input_layer, int num_output)
{
	matrix<float> result(num_output, 1, 1);

	for (int i = 0; i < num_output; ++i)
	{
		//Find sum of weights and run through ReL
		float sum = 0.0f;
		for (int j = 0; j < input_layer.at(0).cols; ++j)
			sum += (input_layer.neuron_at(0, j, 0, 0) * input_layer.data_value_at(i, j, 0, 0));
		result.set(i, 0, 0, max(0, sum));
	}
	return result;
}

matrix<float> convolutional_neural_network::maxpool(matrix<float> input_matrix, int cols, int rows)
{
	std::vector<std::vector<matrix<float>>> samples;
	int across = input_matrix.cols / cols;
	int down = input_matrix.rows / rows;

	//get samples
	for (int i = 0; i < rows; ++i)
	{
		samples.push_back(std::vector<matrix<float>>());
		for (int j = 0; j < cols; ++j)
			samples[i].push_back(input_matrix.from(i * across, j * down, across, down));
	}
	
	//cycle through each sample
	matrix<float> result(cols, rows, 1);
	for (int i = 0; i < samples.size(); ++i)
	{
		for (int j = 0; j < samples[i].size(); ++j)
		{
			//cycle through sample and find max
			float max_value = 0.0f;
			for (int x = 0; x < samples[i][j].rows; ++x)
				for (int y = 0; y < samples[i][j].cols; ++y)
					max_value = max(max_value, samples[i][j].at(x, y, 0));
			result.set(i, j, 0, max_value);
		}
	}
	return result;
}

layer convolutional_neural_network::discriminate_to(int i)
{
	layer current = m_layers[0];
	for (int j = 0; j < i; ++j)
	{
		std::vector<matrix<float>> feature_maps;
		switch (current.type)
		{
		case CNN_CONVOLUTION:
			for (int i2 = 0; i2 = current.feature_map_count; ++i2)
			{
				matrix<float> total = convolve(current.at(i2), current.data_at(0));
				for (int k = 1; k < current.data_count; ++k)
					total += convolve(current.at(i2), current.data_at(k));
				feature_maps.push_back(total);
			}
			current = m_layers[j + 1];
			current.set_feature_maps(feature_maps);
			break;
		case CNN_FEED_FORWARD:
			feature_maps.push_back(feed_forward(current, m_layers[j + 1].at(0).cols));
			current = m_layers[j + 1];
			current.set_feature_maps(feature_maps);
			break;
		case CNN_OUTPUT:
			current.set_feature_maps({ logistic_regression(current.at(0)) });
			return current;
			break;
		}
	}
	return current;
}

layer convolutional_neural_network::generate_until(int i, matrix<float> input_matrix)
{
	return layer();
}

float convolutional_neural_network::max(float a, float b)
{
	return (a > b) ? a : b;
}

matrix<float> convolutional_neural_network::logistic_regression(matrix<float> input_data)
{
	matrix<float> result(input_data.rows, 1, 1);

	float sum = 0.0f;
	for (int i = 0; i < input_data.rows; ++i)
		sum += exp(input_data.at(i, 0, 0));

	for (int i = 0; i < input_data.rows; ++i)
		result.set(i, 0, 0, (exp(input_data.at(i, 0, 0) / sum)));
	return result;
}