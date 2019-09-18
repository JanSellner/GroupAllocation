#pragma once
#include <vector>
#include <valarray>
#include <numeric>
#include <unordered_set>
#include "Matrix.h"

using uchar = unsigned char;

class GroupEvaluation
{
public:
	GroupEvaluation(int n_groups, int n_users, std::vector<int> foreigners = {}, std::vector<double> alphas = {})
		: n_groups(n_groups),
	      n_days(n_groups),
	      n_users(n_users),
	      foreigners(std::move(foreigners)),
	      alphas(std::move(alphas))
	{
		if (this->alphas.empty())
		{
			int n_alphas = this->foreigners.empty() ? 2 : 3;
			this->alphas = std::vector<double>(n_alphas, 1.0 / n_alphas);
		}
	}

	double error_group_sizes(Matrix<int>& days)
	{
		Matrix<int> counts(n_days, n_groups);
		for (size_t row = 0; row < days.rows; ++row)
		{
			const std::valarray<int> day = days.row(row);

			counts.row(row) = count_unique(day);
		}

		std::valarray<double> normalized_counts(counts.data.size());
		double mean = 0.0;
		double mean_abs = 0.0;

		// We can calculate both means in the same loop
		for (size_t i = 0; i < counts.data.size(); ++i)
		{
			double normalized = counts.data[i] / (static_cast<double>(n_users) / n_groups);
			normalized_counts[i] = normalized;	// Required for the variance calculation later

			mean += normalized;
			mean_abs += std::abs(normalized - 1);
		}

		size_t size = normalized_counts.size();
		mean /= size;
		mean_abs /= size;

		double std = std::sqrt(std::accumulate(std::begin(normalized_counts), std::end(normalized_counts), 0.0, [&mean, size](const double accumulator, const double val)
		{
			return accumulator + (val - mean) * (val - mean) / (size - 1);
		}));

		return mean_abs + std;
	}

	double error_meetings(Matrix<int>& days)
	{
		std::valarray<double> meetings(0.0, n_users);

		for (size_t current = 0; current < meetings.size(); ++current)
		{
			std::unordered_set<size_t> indices;

			for (size_t row = 0; row < days.rows; ++row)
			{
				const std::valarray<int> day = days.row(row);
				for (size_t others = 0; others < day.size(); ++others)
				{
					if (day[others] == day[current])
					{
						indices.insert(others);
					}
				}
			}

			// Normalize meetings directly
			meetings[current] = indices.size() / static_cast<double>(n_users);
		}

		double mean = std::accumulate(std::begin(meetings), std::end(meetings), 0.0) / meetings.size();

		size_t size = meetings.size();
		double std = std::sqrt(std::accumulate(std::begin(meetings), std::end(meetings), 0.0, [mean, size](const double accumulator, const double val)
		{
			return accumulator + (val - mean) * (val - mean) / (size - 1);
		}));

		return 1 - mean + std;
	}

	double error_foreigners(Matrix<int>& days)
	{
		Matrix<double> entropies(n_days, n_groups);
		for (size_t row = 0; row < days.rows; ++row)
		{
			const std::valarray<int> day = days.row(row);

			for (int group = 0; group < n_groups; ++group)
			{
				std::valarray<uchar> counts(static_cast<uchar>(0), 2);
				for (size_t d = 0; d < day.size(); ++d)
				{
					if (day[d] == group)
					{
						counts[foreigners[d]]++;
					}
				}

				if (counts[0] == 0 || counts[1] == 0)
				{
					entropies(row, group) = 1;
					continue;
				}

				double total = counts.sum();
				std::valarray<double> probabilities = { counts[0] / total, counts[1] / total };

				double entropy = -probabilities[0] * std::log2(probabilities[0]) - probabilities[1] * std::log2(probabilities[1]);
				entropies(row, group) = 1 - entropy;
			}
		}

		auto values = entropies.data;
		double mean = std::accumulate(std::begin(values), std::end(values), 0.0) / values.size();

		size_t size = values.size();
		double std = std::sqrt(std::accumulate(std::begin(values), std::end(values), 0.0, [mean, size](const double accumulator, const double val)
		{
			return accumulator + (val - mean) * (val - mean) / (size - 1);
		}));

		return mean + std;
	}

	double error_total(Matrix<int>& days)
	{
		double error = error_group_sizes(days) * alphas[0] + error_meetings(days) * alphas[1];
		if (!foreigners.empty())
		{
			error += error_foreigners(days) * alphas[2];
		}

		return error;
	}

private:
	std::valarray<int> count_unique(const std::valarray<int>& day)
	{
		std::valarray<int> counts(day.size());

		for (size_t i = 0; i < counts.size(); ++i)
		{
			counts[day[i]]++;
		}

		return counts;
	}

private:
	int n_groups;
	int n_days;
	int n_users;
	std::vector<int> foreigners;
	std::vector<double> alphas;
};
