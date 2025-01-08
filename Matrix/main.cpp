#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <stdexcept>
#include <future>
#include <algorithm>
#include <memory>

/*Things to edit: threading, holding of Matrix, validation/checking/try-catch, memory management*/
/*Goal is to expand this into an image based project where I take an image make it into matrix's and edit*/
//*update* Mostly done with goal.

class MatrixData {

    std::unique_ptr<float[]>    data;
    size_t                      rows;
    size_t                      cols;

public:

    MatrixData(size_t r, size_t c) : data(std::make_unique<float[]>(r * c)), rows(r), cols(c) {}
    float& at(size_t i, size_t j)             { return data[i * cols + j]; }
    const float& at(size_t i, size_t j) const { return data[i * cols + j]; }
    size_t getRows() const                    { return rows; }
    size_t getCols() const                    { return cols; }

};

using Matrix = std::unique_ptr<MatrixData>;

inline Matrix create_matrix(size_t rows, size_t cols) {

    return std::make_unique<MatrixData>(rows, cols);

}

void multiply_chunk(const Matrix& A, const Matrix& B, Matrix& C, size_t start_row, size_t end_row) {

    const size_t cols_b        = B->getCols();
    const size_t cols_a        = A->getCols();
    std::vector<float> temp_row(cols_b, 0.0f);

    for (size_t i = start_row; i < end_row; ++i) {

        std::fill_n(temp_row.begin(), cols_b, 0.0f);

        for (size_t k = 0; k < cols_a; ++k) {

            const float a_ik   = A->at(i, k);

            for (size_t j = 0; j < cols_b; ++j) {

                temp_row[j]   += a_ik * B->at(k, j);

            }

        }

        for (size_t j = 0; j < cols_b; ++j) {

            C->at(i, j)       = temp_row[j];

        }

    }

}

Matrix parallel_matrix_multiply(const Matrix& A, const Matrix& B) {

    if (A->getCols() != B->getRows()) {

        throw std::runtime_error("Matrix dimensions incompatible for multiplication");

    }

    const size_t rows_a                  = A->getRows();
    const size_t cols_b                  = B->getCols();
    Matrix C                             = create_matrix(rows_a, cols_b);

    unsigned int thread_count            = std::thread::hardware_concurrency();

    if (thread_count == 0) {

        thread_count                     = 1;

    }

    const size_t cache_line_size         = 64;
    const size_t elements_per_cache_line = cache_line_size / sizeof(float);
    const size_t min_chunk_size          = (elements_per_cache_line + cols_b - 1) / cols_b;
    const size_t chunk_size              = std::max(min_chunk_size, (rows_a + thread_count - 1) / thread_count);

    std::vector<std::future<void>> futures;
    futures.reserve(thread_count);

    for (size_t i = 0; i < rows_a; i += chunk_size) {

        size_t end_row                   = std::min(i + chunk_size, rows_a);
        futures.emplace_back(std::async(std::launch::async,
            multiply_chunk, std::ref(A), std::ref(B), std::ref(C), i, end_row));

    }

    for (auto& future : futures) {

        try {

            future.get();

        }

        catch (const std::exception& e) {

            throw std::runtime_error(std::string("Thread execution failed: ") + e.what());

        }

    }

    return C;

}

Matrix generate_random_matrix(size_t rows, size_t cols) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0, 9);

    Matrix matrix            = create_matrix(rows, cols);

    for (size_t i = 0; i < rows; ++i) {

        for (size_t j = 0; j < cols; ++j) {

            matrix->at(i, j) = dis(gen);

        }

    }

    return matrix;

}

Matrix get_matrix_input(size_t rows, size_t cols, const std::string& name) {

    std::cout << "Input " << name << " manually? (y/n): ";
    char choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Matrix matrix                = create_matrix(rows, cols);

    if (std::tolower(choice) == 'y') {

        std::cout << "Enter " << name << " values:\n";

        for (size_t i = 0; i < rows; ++i) {

            for (size_t j = 0; j < cols; ++j) {

                std::cout << name << "[" << (i + 1) << "][" << (j + 1) << "]: ";
                float value;

                if (!(std::cin >> value)) {

                    throw std::runtime_error("Invalid input");

                }

                matrix->at(i, j) = value;

            }

        }

        return matrix;

    }

    return generate_random_matrix(rows, cols);

}

bool CheckMatrix(size_t& rows_a, size_t& cols_a, size_t& rows_b, size_t& cols_b) {

    std::cout << "Enter dimensions for Matrix A (rows cols): ";
    if (!(std::cin >> rows_a >> cols_a)) {

        throw std::runtime_error("Invalid dimensions for Matrix A");

    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter dimensions for Matrix B (rows cols): ";
    if (!(std::cin >> rows_b >> cols_b)) {

        throw std::runtime_error("Invalid dimensions for Matrix B");

    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (cols_a != rows_b) {

        std::cerr << "Error: Matrix dimensions incompatible for multiplication.\n";
        return false;

    }

    return true;

}

void display_matrix(const Matrix& matrix, const std::string& name) {

    std::cout << "\n" << name << ":\n";

    for (size_t i = 0; i < matrix->getRows(); ++i) {

        for (size_t j = 0; j < matrix->getCols(); ++j) {

            std::cout << matrix->at(i, j) << " ";

        }

        std::cout << '\n';

    }

}

int main() {

    try {

        size_t rows_a, cols_a, rows_b, cols_b;

        if (!CheckMatrix(rows_a, cols_a, rows_b, cols_b)) {

            return -1;

        }

        Matrix A = get_matrix_input(rows_a, cols_a, "A");
        Matrix B = get_matrix_input(rows_b, cols_b, "B");

        display_matrix(A, "Matrix A");
        display_matrix(B, "Matrix B");

        Matrix C = parallel_matrix_multiply(A, B);
        display_matrix(C, "Result Matrix C");

        return 0;

    }

    catch (const std::exception& e) {

        std::cerr << "Error: " << e.what() << std::endl;
        return 1;

    }

}
