#include <shmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#define NUM_POINTS 1000000


unsigned long inside = 0, total = 0;

int
main(int argc, char* argv[], char *envp[])
{
    int me, myshmem_n_pes;
    /*
    ** Starts/Initializes SHMEM/OpenSHMEM
    */
    shmem_init();
    /*
    ** Fetch the number or processes
    ** Some implementations use num_pes();
    */
    myshmem_n_pes = shmem_n_pes();
    /*
    ** Assign my process ID to me
    */
    me = shmem_my_pe();

    struct drand48_data randstate;
    srand48_r(time(NULL)*me,&randstate);

    for(total = 0; total < NUM_POINTS; ++total) {
        double x,y;
        drand48_r(&randstate,&x);
        drand48_r(&randstate,&y);

        if(x*x + y*y < 1) {
            ++inside;
        }
    }

    shmem_barrier_all();

    if(me == 0) {
        for(int i = 1; i < myshmem_n_pes; ++i) {
            unsigned long remoteInside,remoteTotal;
            shmem_long_get(&remoteInside,&inside,1,1);
            shmem_long_get(&remoteTotal,&total,1,1);
            total += remoteTotal;
            inside += remoteInside;
        }

        double approx_pi = 4.0*inside/(double)total;

        assert(fabs(M_PI-approx_pi) < 0.1);

        if (NULL == getenv("MAKELEVEL")) {
            printf("Pi from %lu points on %d PEs: %lf\n",total,myshmem_n_pes,approx_pi);
        }
    }

    shmem_finalize();

    return 0;
}

