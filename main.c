#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include "hw2_output.h"
#include "hw2_output.c"
#include <semaphore.h>
#include <time.h>

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

int** cig_butts;
sem_t** cig_butts_inter;
sem_t lock;
sem_t prior;
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

void *collect_cigg_butts( void* arguments)
{
    struct Private_struct *private_prop = arguments;
    hw2_notify(action = GATHERER_CREATED, (*private_prop).gid, 0, 0);
    for(int num = 0; num < (*private_prop).ng; num++)
    {
        sem_wait(&prior);
        for(int i = 0; i < (*private_prop).si; i++ )
        {
            for (int j = 0; j < (*private_prop).sj; j++)
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                sem_wait(&cig_butts_inter[coord_i][coord_j]);
            }
            
        }
        sem_post(&prior);
        hw2_notify(action = GATHERER_ARRIVED, (*private_prop).gid, (*private_prop).top_corners[num][0],(*private_prop).top_corners[num][1]);
        for(int j = 0; j < (*private_prop).sj; j++)
        {
            for(int i = 0; i < (*private_prop).si; i++ )
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                while(cig_butts[coord_i][coord_j] != 0)
                {
                    usleep((*private_prop).time);
                    sem_wait(&lock);
                    cig_butts[coord_i][coord_j]--;
                    hw2_notify(action = GATHERER_GATHERED, (*private_prop).gid, coord_i,coord_j);
                    sem_post(&lock);
                }
            }
            
        }

        for(int i = 0; i < (*private_prop).si; i++ )
        {
            for (int j = 0; j < (*private_prop).sj; j++)
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                sem_post(&cig_butts_inter[coord_i][coord_j]);
            }
            
        }
        hw2_notify(action = GATHERER_CLEARED, (*private_prop).gid, 0, 0);
    }
    hw2_notify(action = GATHERER_EXITED, (*private_prop).gid, 0, 0);
}


int main()
{
    int gridx, gridy;
    int num_privates;
    scanf("%d %d", &gridx, &gridy);
    cig_butts = (int**)malloc(gridx * sizeof(int*));
    cig_butts_inter = (sem_t**)malloc(gridx * sizeof(sem_t*));
    for(int i = 0; i < gridx; i++)
    {
        cig_butts[i] = (int*)malloc(gridy * sizeof(int));
        cig_butts_inter[i] = (sem_t*)malloc(gridy * sizeof(sem_t));
        for(int j = 0; j < gridy; j++)
        {
            scanf("%d", &cig_butts[i][j]);
            sem_init(&cig_butts_inter[i][j], 0, 1);
        }
    }
    sem_init(&lock, 0, 1);
    sem_init(&prior, 0, 1);
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
    
    hw2_init_notifier();
    for(int i = 0; i < num_privates; i++)
    {
        pthread_create( &private_thread[i], NULL, &collect_cigg_butts, (void*)&privates[i]);
    }

    for(int i = 0; i < num_privates; i++)
    {
        pthread_join( private_thread[i], NULL);
    }

    return 0;

}