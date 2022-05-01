#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include "hw2_output.h"
#include "hw2_output.c"
#include <semaphore.h>

int** cig_butts;
sem_t** mutex;
enum hw2_actions action;
struct Private_struct
{
    int gid;
    int time;
    int si;
    int sj;
    int ng;
    int** top_corners;
};

void *collect_cigg_butts( struct Private_struct private_struct)
{
    hw2_init_notifier();
    hw2_notify(action = GATHERER_CREATED, private_struct.gid, 0, 0);
    for(int num = 0; num < private_struct.ng; num++)
    {
        for(int i = 0; i < private_struct.si; i++ )
        {
            for (int j = 0; j < private_struct.sj; j++)
            {
                sem_wait(&mutex[private_struct.top_corners[i][0] + i][private_struct.top_corners[i][1] + j]);
            }
            
        }
        hw2_notify(action = GATHERER_ARRIVED, private_struct.gid, private_struct.top_corners[0],private_struct.top_corners[1]);
        for(int i = 0; i < private_struct.si; i++ )
        {
            for (int j = 0; j < private_struct.sj; j++)
            {
                while(cig_butts[private_struct.top_corners[i][0] + i][private_struct.top_corners[i][1] + j] != 0)
                {
                    
                }
            }
            
        }

    }
    
}

int main()
{
    int gridx, gridy;
    int num_privates;
    scanf("%d %d", &gridx, &gridy);
    cig_butts = (int**)malloc(gridx * sizeof(int*));
    mutex = (sem_t**)malloc(gridx * sizeof(sem_t*));
    for(int i = 0; i < gridx; i++)
    {
        cig_butts[i] = (int*)malloc(gridy * sizeof(int));
        mutex[i] = (sem_t*)malloc(gridy * sizeof(sem_t));
        for(int j = 0; j < gridy; j++)
        {
            scanf("%d", &cig_butts[i][j]);
            sem_init(&mutex[i][j], 0, 1);
        }
    }
    scanf("%d", &num_privates);
    struct Private_struct privates[num_privates];
    pthread_t private_thread[num_privates];
    for(int i = 0; i < num_privates; i++)
    {
        scanf("%d %d %d %d %d", &privates[i].gid, &privates[i].si, &privates[i].sj, &privates[i].time, &privates[i].ng);
        privates[i].top_corners = (int**)malloc(privates[i].ng * sizeof(int*));
        for(int j= 0; j < privates[i].ng; j++)
        {
            privates[i].top_corners[j] = (int*)malloc(2 * sizeof(int));
            scanf("%d %d", &privates[i].top_corners[j][0], &privates[i].top_corners[j][1]);
        }
    }

    for(int i = 0; i < num_privates; i++)
    {
        pthread_create( &private_thread[i], NULL, collect_cigg_butts(privates[i]), NULL);
    }

    for(int i = 0; i < num_privates; i++)
    {
        pthread_join( private_thread[i], NULL);
    }

    return 0;

}