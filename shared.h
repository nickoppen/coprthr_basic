#ifndef SHARED_H_INCLUDED
#define SHARED_H_INCLUDED

#define WIDTH 64
#define MPI_BUF_SIZE 1024
#define ECORES 16

#define _USING_MPI_

typedef struct
{
    float n;
    int width;
	float * p_mem;
} my_args_t;


#endif // SHARED_H_INCLUDED
