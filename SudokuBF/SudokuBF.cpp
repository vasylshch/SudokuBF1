// SudokuBF.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// inspired by
// https://www.sudokuwiki.org/Getting_Started
// https://www.sudokuwiki.org/Brute_Force_vs_Logical_Strategies

// Motivation. It should be:
// short
// clear
// performance does matter
// 1 file
// brute force algo on finding all sudoku's solutions close to current one
// sss_size - configurable compile time constant

#include <iostream>
#include <cassert>
#include <bitset>
#include <array>
#include <optional>
#include <string>
#include <string_view>

constexpr size_t sss_size = 3;							// subsquare side size
constexpr size_t unit_size = sss_size * sss_size;		// number of cells in a row, column and subsquare (unit)
constexpr size_t field_size = unit_size * unit_size;	// total number of cells on the field

using value_type = int;
using coor_type = size_t;	// row, column and subsquare coordinates must be in range [0..unit_size)

constexpr value_type MIN_VALUE = 1;	// could be any (non-negative) value
constexpr value_type MAX_VALUE = MIN_VALUE + unit_size - 1;

// valid values should be >= 0
// mask_type should be big enough in order to hold 1 << MAX_VALUE
using mask_type = std::bitset<MAX_VALUE + 1>;

static_assert(MIN_VALUE >= 0);
static_assert(MIN_VALUE <= MAX_VALUE);
static_assert(mask_type{}.size() > MAX_VALUE);


constexpr coor_type getSubSquareCoor(coor_type row, coor_type column) noexcept
{
	if constexpr (unit_size == 9)
	{
		constexpr std::array<std::array<std::uint8_t, 9>, 9> subSquareCoorUnit9
		{ {
			{0, 0, 0, 1, 1, 1, 2, 2, 2},
			{0, 0, 0, 1, 1, 1, 2, 2, 2},
			{0, 0, 0, 1, 1, 1, 2, 2, 2},
			{3, 3, 3, 4, 4, 4, 5, 5, 5},
			{3, 3, 3, 4, 4, 4, 5, 5, 5},
			{3, 3, 3, 4, 4, 4, 5, 5, 5},
			{6, 6, 6, 7, 7, 7, 8, 8, 8},
			{6, 6, 6, 7, 7, 7, 8, 8, 8},
			{6, 6, 6, 7, 7, 7, 8, 8, 8}
		} };

		return subSquareCoorUnit9[row][column];
	}
	else
	{
		return (row / sss_size) * sss_size + (column / sss_size);
	}
}


class Field
{
public:
	Field(void) = default;
	Field(std::string_view strField);

	void setValue(coor_type row, coor_type column, value_type value)
	{
		rows[row].set(value);
		columns[column].set(value);
		subSquares[getSubSquareCoor(row, column)].set(value);

		values[row][column] = value;
	}

	void removeValue(coor_type row, coor_type column, value_type value)
	{
		rows[row].reset(value);
		columns[column].reset(value);
		subSquares[getSubSquareCoor(row, column)].reset(value);

		values[row][column].reset();
	}

	bool couldSetValue(coor_type row, coor_type column, value_type value) const
	{
		if (rows[row].test(value))
		{
			return false;
		}
		if (columns[column].test(value))
		{
			return false;
		}
		if (subSquares[getSubSquareCoor(row, column)].test(value))
		{
			return false;
		}

		return true;
	}

	bool isKnown(coor_type row, coor_type column) const noexcept
	{
		return values[row][column].has_value();
	}

	value_type getValue(coor_type row, coor_type column) const noexcept
	{
		return *values[row][column];
	}

private:
	std::array<mask_type, unit_size> rows;			// bit array to signal presence of valid values on a row
	std::array<mask_type, unit_size> columns;		// bit array to signal presence of valid values on a column
	std::array<mask_type, unit_size> subSquares;	// bit array to signal presence of valid values in a subSquare

	std::array<std::array<std::optional<value_type>, unit_size>, unit_size> values;	// unknown or value of each cell
};


Field::Field(std::string_view strField)
{
	// initially UNKNOWN_VALUE is set in all cells
	// values are read till the first value that contradicts to rules

	// this assert could be ignored. It just indicates that strField is "strange"
	// "extra" initialization values are ignored
	// trailing end of the field is set to UNKNOWN_VALUE initially
	assert(strField.size() == field_size);
	const size_t cellsToInit = std::min(strField.size(), field_size);
	for(size_t cell = 0; cell < cellsToInit; ++cell)
	{
		const coor_type row = cell / unit_size, column = cell % unit_size;
		// map '1' to MIN_VALUE and so on...
		value_type value = static_cast<value_type>(strField[cell] - '1') + MIN_VALUE;
		if ((value >= MIN_VALUE) && (value <= MAX_VALUE))
		{
			if (couldSetValue(row, column, value))
			{
				setValue(row, column, value);
			}
			else
			{
				// asset here is just for indication that something is "wrong" with strField,
				// so it is not "fully" handlded. Meanwhile field is still in "correct" state
				assert(0);
				return;
			}
		}
	}
}


std::string toString(const Field& field)
{
	std::string result;
	result.resize(field_size);
	size_t cell = 0;
	for (coor_type row = 0; row < unit_size; ++row)
		for (coor_type column = 0; column < unit_size; ++column)
		{
			if (!field.isKnown(row, column))
			{
				// map UNKNOWN_VALUE to '0'
				result[cell++] = '0';
			}
			else
			{
				// map MIN_VALUE to '1' and so on...
				result[cell++] = static_cast<char>('1' + (field.getValue(row,column) - MIN_VALUE));
			}
		}

	return result;
}

void printField(const Field& field)
{
	std::cout << '\n' << toString(field) << '\n';
}


void bruteForceImpl(Field *field, coor_type startRow, coor_type startColumn, int *solutionCount)
{
	assert(field);
	assert(solutionCount);

	for (coor_type row = startRow; row < unit_size; ++row)
	{
		for (coor_type column = (row == startRow) ? startColumn : 0; column < unit_size; ++column) // for each cell
		{
			if (!field->isKnown(row, column)) // that is still unknown
			{
				for (value_type value = MIN_VALUE; value <= MAX_VALUE; ++value) // for each possible value
				{
					// check if we could set this value to this cell according to the rules
					if (!field->couldSetValue(row, column, value))
					{
						continue;
					}
					// valid to place this value in this cell
					field->setValue(row, column, value);
					// call this function recursively to fill in still unkown cells
					bruteForceImpl(field, row, column, solutionCount);
					// remove current assumtion
					field->removeValue(row, column, value);
				}
				return;
			}
		}
	}
	// no unknown cells, must be a solution
	++(*solutionCount);
}

int bruteForce(std::string_view strField)
{
	Field field{ strField };
	int solutionCount{ 0 };

	bruteForceImpl(&field, 0, 0, &solutionCount);

	return solutionCount;
}


int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;

	return bruteForce(argv[1]);
}
