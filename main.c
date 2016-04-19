#include <stdio.h>
#include <stdlib.h>

#include "coprthr.h"
#include "coprthr_cc.h"
#include "coprthr_thread.h"
#include "coprthr_mpi.h"

#include "shared.h"

int main()
{
    float host_buffer[WIDTH];
    int i;

	int dd = coprthr_dopen(COPRTHR_DEVICE_E32,COPRTHR_O_THREAD);
	printf("dd=%d\n",dd);
	if (dd<0)
	{
        printf("device open failed\n");
        exit(0);
    }

	coprthr_program_t prg;
	prg = coprthr_cc_read_bin("./pfunc.cbin.3.e32", 0);
	printf("prg=%p \n",prg);
	if (!(prg))
	{
        printf("file mpi_pfunc.cbin.3.e32 not found\n");
        coprthr_dclose(dd);
	    exit(0);
    }

#ifdef _USING_MPI_
	coprthr_sym_t thr = coprthr_getsym(prg,"p_func_thread_mpi");
#else
	coprthr_kernel_t thr = coprthr_getsym(prg,"p_func_thread_std");
#endif // _USING_MPI_

	printf("thr=%p\n",thr);
	if (thr == (coprthr_sym_t)0xffffffff)
	{
        printf("kernel p_func_thread not found\n");
        coprthr_dclose(dd);
        exit(0);
    }

    /// initialise host buffer
    for (i=0; i < WIDTH; i++)
        host_buffer[i] = -1 * i;
	/// write data to shared DRAM
	coprthr_mem_t p_data_mem = coprthr_dmalloc(dd, WIDTH*sizeof(float), 0);
	coprthr_dwrite(dd, p_data_mem, 0, host_buffer, WIDTH*sizeof(float), COPRTHR_E_WAIT);

#ifdef _USING_MPI_
	my_args_t args = {
		.n = 2.0,
		.width = WIDTH,
		.p_mem = coprthr_memptr(p_data_mem, 0),
	};
	coprthr_mpiexec(dd, ECORES, thr, &args, sizeof(args), 0);
#else
    float n[] = { 2.0 };
    int width[] = { WIDTH };
	coprthr_mem_t p_arg_n = coprthr_dmalloc(dd, sizeof(float), 0);
	coprthr_mem_t p_arg_width = coprthr_dmalloc(dd, sizeof(int), 0);
	coprthr_dwrite(dd, p_arg_n,     0, &n,     sizeof(float), COPRTHR_E_WAIT);
	coprthr_dwrite(dd, p_arg_width, 0, &width, sizeof(int),   COPRTHR_E_WAIT);
    void * p_args[] = { &p_arg_n, &p_arg_width, &p_data_mem };
	coprthr_dexec(dd, thr, 3, p_args, ECORES, 0, COPRTHR_E_WAIT);
#endif // _USING_MPI_

    /// copy the results back out to the host buffer
	coprthr_dread(dd, p_data_mem, 0, host_buffer, WIDTH*sizeof(float),COPRTHR_E_WAIT);

	coprthr_dclose(dd);

	for (i = 0; i < WIDTH; i++)
        printf("%f, ", host_buffer[i]);
    printf("\n");

    exit(1);
}
