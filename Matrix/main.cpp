#include <iostream>
#include <vector>
#include <thread>
#include <random>

/*Things to edit: threading, holding of Matrix, validation/checking/try-catch, memory management*/

/*Goal is to expand this into an image based project where I take an image make it into matrix's and edit*/

using Matrix = std::vector<std::vector<int>>;

void multiply(const Matrix& A, const Matrix& B, Matrix& C, int start_row, int chunk_size) {

    const int cols_b        = B[0].size();
    const int cols_a        = A[0].size();
    const int end_row       = std::min(start_row + chunk_size, static_cast<int>(A.size()));

    for (int i = start_row; i < end_row; ++i) {

        for (int k = 0; k < cols_a; ++k) {

            const int a_ik  = A[i][k];

            for (int j = 0; j < cols_b; ++j) {

                C[i][j]    += a_ik * B[k][j];

            }

        }

    }

}

Matrix parallel_matrix_multiply(const Matrix& A, const Matrix& B) {

    const int rows_a    = A.size();
    const int cols_b    = B[0].size();

    Matrix C(rows_a, std::vector<int>(cols_b, 0));

    // START OF THREADS, LOOK BACK AT THIS!
    const unsigned int thread_count = std::thread::hardware_concurrency();
    const int chunk_size = (rows_a + thread_count - 1) / thread_count;

    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    // This should just be creating a thread for the matrix
    for (int i = 0; i < rows_a; i += chunk_size) {

        threads.emplace_back(multiply, std::cref(A), std::cref(B),
                           std::ref(C), i, chunk_size);

    }

    // Wait for threads to complete
    for (auto & thread : threads) {

        thread.join();

    }

    return C;

}

Matrix generate_random_matrix(int rows, int cols) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);

    Matrix matrix(rows, std::vector<int>(cols));

    for (auto& row : matrix) {

        for (auto& elem : row) {

            elem = dis(gen);

        }

    }

    return matrix;

}

Matrix get_matrix_input(int rows, int cols, const std::string& name) {

    std::cout << "Input " << name << " manually? If yes, input 1 by 1 (y/n): " << std::endl;

    char choice;
    std::cin >> choice;

    if (std::tolower(choice) == 'y') {

        Matrix matrix(rows, std::vector<int>(cols));

        std::cout << "Enter " << name << " values:\n";

        for (int i = 0; i < rows; ++i) {

            for (int j = 0; j < cols; ++j) {

                std::cout << name << "[" << (i + 1) << "][" << (j + 1) << "]: " << std::endl;
                std::cin >> matrix[i][j];

            }

        }

        return matrix;

    }

    std::cout << name << " filled." << std::endl;

    return generate_random_matrix(rows, cols);

}

bool CheckMatrix(int& rows_a, int& cols_a, int& rows_b, int& cols_b) {

    std::cout << "Enter dimensions for Matrix A (rows cols): " << std::endl;
    std::cin >> rows_a >> cols_a;
    std::cout << "Enter dimensions for Matrix B (rows cols): " << std::endl;
    std::cin >> rows_b >> cols_b;

    if (cols_a != rows_b) {

        std::cerr << "Error: Matrix dimensions incompatible for multiplication.\n";

        return false;

    }

    return true;

}

void display_matrix(const Matrix& matrix, const std::string& name) {

    std::cout << "\n" << name << ":\n";

    for (const auto& row : matrix) {

        for (const auto& elem : row) {

            std::cout << elem << " ";
        }

        std::cout << '\n';

    }

}


int main() {

    int rows_a, cols_a, rows_b, cols_b;

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