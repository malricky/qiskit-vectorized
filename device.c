#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <veda_device.h>
#include <complex.h>
#include <omp.h>

#define VEDA(err) check(err, __FILE__, __LINE__)

void check(VEDAresult err, const char* file, const int line) {
	if(err != VEDA_SUCCESS) {
		const char *name, *str;
		vedaGetErrorName	(err, &name);
		vedaGetErrorString	(err, &str);
		printf("%s: %s @ %s:%i\n", name, str, file, line);
		exit(1);
	}
}


typedef double _Complex complex_t;

void convertToBase4Array(unsigned int num, int* result, int size) {
  
    for (int i = size - 1; i >= 0; i--) {
        result[i] = num & 0x3; // Extract the last 2 bits
        num >>= 2; // Shift right by 2 bits
    }
}

/**
 * @brief Returns a 2D array with countin in the base on N.
 * say, a number system with base 3, can be generated
 * as, `base4numbersystem(int N, int* elements);`
 *
 * @param N Base of the number system
 * @param elements This will store the number of elements in the number system
 * @return int* Returns a 2D array with countin in the base on N.
 */
int* base4numbersystem(int N, int* elements){
    //*elements = (int)pow(4, N);

    int base4Array[(int)*elements][N];
    int* ind_mu = (int*) malloc((*elements * N) * sizeof(int));

    for(unsigned int i = 0; i < (int)(*elements); i++){
        convertToBase4Array(i, &base4Array[i][0], N);

        for (int j=0; j < N; j++)
            ind_mu[i*N + j] = base4Array[i][j];
    }

    return ind_mu;

}

void build_binary_index_value(unsigned int N, unsigned int binary_index_value[]){
    for (int i = 0; i < N; i ++)
        binary_index_value[i] = (unsigned int) pow(2, N - i - 1);
}

void mapping(unsigned int _number_of_qubits, VEDAdeviceptr a, VEDAdeviceptr b, int num_elements) {

    struct timeval start_time, end_time;
    
    complex_t *densitymatrix;
    complex_t *densitymatrix_b;

    VEDA(vedaMemPtr((void**)&densitymatrix,a));
    VEDA(vedaMemPtr((void**)&densitymatrix_b,b));

    unsigned int int2bin[4][2] = { {0, 0}, {0, 1}, {1, 0}, {1, 1} };

    gettimeofday(&start_time, NULL);

    int elements = num_elements;
    int* ind_mu = base4numbersystem(_number_of_qubits, &elements);

    unsigned int power_of_two = (unsigned int)pow(2, _number_of_qubits);

    unsigned int binary_index_value[_number_of_qubits];
    build_binary_index_value(_number_of_qubits, binary_index_value);
    // Profiling from here

    unsigned int final_index[elements][2];
    
    for(unsigned int i = 0; i < elements; i++){

        final_index[i][0] = 0;
        final_index[i][1] = 0;
        
        /*double idxb[_number_of_qubits];

        for (int j=0; j < _number_of_qubits; j++) {
            idxb[j] = ind_mu[i*_number_of_qubits + j];
        }*/

        for (int j=0; j < _number_of_qubits; j++) {
            unsigned int _temp = ind_mu[i*_number_of_qubits + j];
    
            // (x, y)      =  index_list[j] * binary_index_value[j]
            final_index[i][0] += int2bin[_temp][0] * binary_index_value[j];
            final_index[i][1] += int2bin[_temp][1] * binary_index_value[j];
        }
        int index = (final_index[i][0] * power_of_two) + final_index[i][1];
        densitymatrix[index] = densitymatrix_b[i];
    }
    free(ind_mu);

    gettimeofday(&end_time, NULL);

    // Calculate the elapsed time in seconds
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                          (end_time.tv_usec - start_time.tv_usec) / 1e6;

    // Print the elapsed time
    printf("[i] Elapsed Time : %fs\n", elapsed_time);
}
