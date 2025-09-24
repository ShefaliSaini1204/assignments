#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

const int SUCCESS = 0;
const int ERROR = 1;

std::vector<std::vector<int>> check_matrix(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error in opening file." << filename << std::endl;
        exit(ERROR);
    }

    int rows, cols;
    if (!(file >> rows >> cols))
    {
        std::cerr << "Error in determining the dimensions of the matrix from the file" << filename << std::endl;
        exit(ERROR);
    }

    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols));
    int count = 0;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            if (!(file >> matrix[i][j]))
            {
                std::cerr << "Error as the given data is not sufficient as per the dimensions of the matrix" << std::endl;
                exit(ERROR);
            }
            count++;
        }
    }

    
    int temp;
    if (file >> temp)
    {
        std::cerr << "Error as the input is more than required." << std::endl;
        exit(ERROR);
    }

    file.close();
    return matrix;
}

std::vector<int> check_vector(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error in opening the given file-" << filename << std::endl;
        exit(ERROR);
    }

    int rows;
    if (!(file >> rows))
    {
        std::cerr << "Error in determining the size of the file" << filename << std::endl;
        exit(ERROR);
    }

    std::vector<int> vec(rows);
    int count = 0;
    for (int i = 0; i < rows; ++i)
    {
        if (!(file >> vec[i]))
        {
            std::cerr << "Error as the data is not sufficient as per the vector size" << std::endl;
            exit(ERROR);
        }
        count++;
    }

    
    int temp;
    if (file >> temp)
    {
        std::cerr << "Error as more data than required." << std::endl;
        exit(ERROR);
    }

    file.close();
    return vec;
}


std::vector<int> multiplication(const std::vector<std::vector<int>> &matrix, const std::vector<int> &vec)
{
    if (matrix.empty() || vec.empty() || matrix[0].size() != vec.size())
    {
        std::cerr << "Mismatch in dimensions for multiplication." << std::endl;
        exit(ERROR);
    }

    std::vector<int> result(matrix.size(), 0);
    for (size_t i = 0; i < matrix.size(); ++i)
    {
        for (size_t j = 0; j < vec.size(); ++j)
        {
            result[i] += matrix[i][j] * vec[j];
        }
    }

    return result;
}


int main(int argc, char *argv[])
{
   
    if (argc != 5)
    {
        std::cerr << "Missing input arguments."<< std::endl;
        return ERROR;
    }

    std::string matrix_input, vector_input;

   
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-m" && i + 1 < argc)
        {
            matrix_input = argv[++i];
        }
        else if (arg == "-v" && i + 1 < argc)
        {
            vector_input = argv[++i];
        }
        else
        {
            std::cerr << "Invalid input arguments." << std::endl;
            return ERROR;
        }
    }

    
    std::vector<std::vector<int>> final_matrix = check_matrix(matrix_input);
    std::vector<int> final_vector = check_vector(vector_input);

   
    std::vector<int> multiply_result = multiplication(final_matrix, final_vector);

    
    for (int val : multiply_result)
    {
        std::cout << val << std::endl;
    }

    return SUCCESS;
}

