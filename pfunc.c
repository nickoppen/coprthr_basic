#include <coprthr_mpi.h>
#include "shared.h"

#ifdef _USING_MPI_
__kernel void
p_func_thread_mpi( void * p_args )
{
    int i;
    int rank;

	my_args_t* pargs = (my_args_t*)p_args;

	float n = pargs->n;
	int cnt = (int)(pargs->width / ECORES);         /// width is the size of the entire work space there are 16 cores so each does width/16
	float * g_pmem = pargs->p_mem;

    /// store the starting position of free space
	void * memfree = coprthr_tls_sbrk(0);
    /// grab a chunk of free space for local storage
	float * localSpace = (float*)coprthr_tls_sbrk(cnt * sizeof(float));

	MPI_Status status;
	MPI_Init(0, MPI_BUF_SIZE);
	MPI_Comm comm = MPI_COMM_THREAD;
	MPI_Comm_rank(comm, &rank);
//	MPI_Comm_size(comm, &size);                 /// other MPI stuff that we don't need just now
//	MPI_Cart_shift(comm, 0, 1, &left, &right);

    /// copy in
	e_dma_copy(localSpace, g_pmem + (cnt * rank), cnt*sizeof(float));

    /// modify
	for (i=0; i<cnt; i++)
        localSpace[i] = localSpace[i] * n * (float)rank;

    /// copy out
	e_dma_copy(g_pmem + (cnt * rank), localSpace, cnt*sizeof(float));

    /// reset the free space pointer to where it was initially
    coprthr_tls_brk(memfree);
//	MPI_Finalize();
}

#else

__kernel void
p_func_thread_std( float * p_arg_n,  int* p_arg_width, void * p_mem )
{
    int i;
    int rank;

	float n = p_arg_n[0];
	int cnt = p_arg_width[0] / ECORES;         /// width is the size of the entire work space there are 16 cores so each does width/16
	float * g_pmem = (float *)p_mem;

    /// store the starting position of free space
	void * memfree = coprthr_tls_sbrk(0);
    /// grab a chunk of free space for local storage
	float * localSpace = (float*)coprthr_tls_sbrk(cnt * sizeof(float));

    rank = get_global_id(0);
    /// copy in
	e_dma_copy(localSpace, g_pmem + (cnt * rank), cnt*sizeof(float));

    /// modify
	for (i=0; i<cnt; i++)
        localSpace[i] = n * (float)rank;

    /// copy out
	e_dma_copy(g_pmem + (cnt * rank), localSpace, cnt*sizeof(float));

    /// reset the free space pointer to where it was initially
    coprthr_tls_brk(memfree);
//	MPI_Finalize();
}

#endif
