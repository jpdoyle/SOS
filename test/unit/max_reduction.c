/*
* reduce [0...num_pes]
*/

#include <shmem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

long pSync[SHMEM_REDUCE_SYNC_SIZE];

#define N 3

long src[N];
long dst[N];

#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define WRK_SIZE MAX(N/2+1, SHMEM_REDUCE_MIN_WRKDATA_SIZE)

long pWrk[WRK_SIZE];

int
main(int argc, char* argv[])
{
    int i, Verbose=0;
    char *pgm;

    if ((pgm=strrchr(argv[0],'/')))
        pgm++;
    else
        pgm = argv[0];

	if (argc > 1) {
        if (strncmp(argv[1],"-v",3) == 0)
            Verbose=1;
        else if (strncmp(argv[1],"-h",3) == 0) {
            fprintf(stderr,"usage: %s {v(verbose)|h(help)}\n",pgm);
            shmem_finalize();
            exit(1);
        }
    }

    for (i = 0; i < SHMEM_REDUCE_SYNC_SIZE; i += 1) {
        pSync[i] = SHMEM_SYNC_VALUE;
    }

    shmem_init();

    for (i = 0; i < N; i += 1) {
        src[i] = shmem_my_pe() + i;
    }
    shmem_barrier_all();

    shmem_long_max_to_all(dst, src, N, 0, 0, shmem_n_pes(), pWrk, pSync);

    if (Verbose) {
        printf("%d/%d	dst =", shmem_my_pe(), shmem_n_pes() );
        for (i = 0; i < N; i+= 1) {
            printf(" %ld", dst[i]);
        }
        printf("\n");
    }

    for (i = 0; i < N; i+= 1) {
        if (dst[i] != shmem_n_pes() - 1 + i) {
            printf("[%3d] Error: dst[%d] == %ld, expected %ld\n",
                   shmem_my_pe(), i, dst[i], shmem_n_pes() - 1 + (long) i);
            shmem_global_exit(1);
        }
    }

    shmem_finalize();

    return 0;
}
