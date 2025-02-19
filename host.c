#include <stdio.h>
#include <stdlib.h>
#include <veda.h>
#include <complex.h>
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

int mapping_host(unsigned int _number_of_qubits, complex_t *densitymatrix, complex_t *densitymatrix_b) {

    size_t size = (1 << (2 * _number_of_qubits)) * sizeof(complex_t);
    //size_t chunk_size = (2 * 1024 * 1024) / sizeof(complex_t) * sizeof(complex_t);

    size_t chunk_size = size/2;

    int iter = 2;
    int num_elements;

    /*if(size > chunk_size){
        printf("Size more than chunk_size\n");
        iter = size/chunk_size;
    }
    else{
        chunk_size = size;
        iter = 1;
    }*/

    num_elements = chunk_size/sizeof(complex_t);
   
    VEDA(vedaInit(0));

    for(int i=0;i<iter;i++){

        VEDAcontext ctx;
        VEDAdevice vd;

        VEDA(vedaDeviceGet(&vd,1+i));

        VEDA(vedaCtxCreate(&ctx,1,vd));
        VEDA(vedaCtxPushCurrent(ctx));


        VEDAmodule mod;
        VEDA(vedaModuleLoad(&mod, "/home/Ricky_Intern_2024/.local/lib/python3.6/site-packages/qiskit/providers/basicaer/mapping.vso"));

        VEDAfunction func;
        VEDA(vedaModuleGetFunction(&func, mod, "mapping"));

        VEDAdeviceptr a;
        VEDAdeviceptr b;

        VEDA(vedaMemAllocAsync(&a,chunk_size,0)); 
        VEDA(vedaMemAllocAsync(&b,chunk_size,1));

        VEDA(vedaMemcpyHtoDAsync(a,densitymatrix+(i*(chunk_size/sizeof(complex_t))),chunk_size,0));
        VEDA(vedaMemcpyHtoDAsync(b,densitymatrix_b+(i*(chunk_size/sizeof(complex_t))),chunk_size,1));

        VEDAargs args;
        VEDA(vedaArgsCreate(&args));
        VEDA(vedaArgsSetU32(args, 0, _number_of_qubits));
        VEDA(vedaArgsSetVPtr(args, 1, a));
        VEDA(vedaArgsSetVPtr(args, 2, b));
        VEDA(vedaArgsSetI32(args, 3, num_elements));

        VEDA(vedaLaunchKernel(func, 0, args));
        VEDA(vedaStreamSynchronize(0));
        VEDA(vedaStreamSynchronize(1));
        VEDA(vedaCtxSynchronize());

        VEDA(vedaMemcpyDtoHAsync(densitymatrix+(i*(chunk_size/sizeof(complex_t))),a,chunk_size,0));
        VEDA(vedaMemcpyDtoHAsync(densitymatrix_b+(i*(chunk_size/sizeof(complex_t))),b,chunk_size,1));
        VEDA(vedaArgsDestroy(args));


        VEDA(vedaMemFreeAsync(a,0));
        VEDA(vedaMemFreeAsync(b,1));

    
    }

    VEDA(vedaExit());

    return 0;

}
