#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hw2_output.c"
#include <semaphore.h>
#include <time.h>
#include <signal.h>

int** cig_butts;

sem_t lock;
sem_t sem_break;
sem_t** gatherers_prior;
sem_t** sneakers_prior;

int sneaker_cord[8][2] = {{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};
int* gatherers_job;
int* sneakers_job;
enum hw2_actions action;    

struct Private_struct
{
    int gid;
    int time;
    int si;
    int sj;
    int ng;
    int** top_corners;
    int queue;
};

struct Intersection_struct
{
    int num_gatherer;
    int num_sneaker;
    struct Private_struct* gatherer_info;
    struct Sneaker_struct* sneaker_info;
};

struct Sneaker_struct
{
    int sid;
    int time;
    int ns;
    int* cigg_num;
    int** litter_centers;
    int queue;
};

struct Commander
{
    int num_orders;
    int* time_order;
    int* command;
    int num_privates;
    struct Private_struct* privates;
    int num_sneakers;
    struct Sneaker_struct* sneakers;
    pthread_t* threads;
};


int job_done(int* gatherer_job, int* sneaker_job, struct Intersection_struct* info)
{
    int is_Okay = 1;
    for(int i = 0; i < info->num_gatherer; i++)
    {
        if(gatherer_job[i] != info->gatherer_info[i].ng)
        {
            is_Okay = 0;
        }
    }
    for(int i = 0; i < info->num_sneaker; i++)
    {
        if(sneaker_job[i] != info->sneaker_info[i].ns)
        {
            is_Okay = 0;
        }
    }
    return is_Okay;
}

int check_intersection(struct Private_struct gather_i, struct Private_struct gather_j, int i_q, int j_q)
{
    for(int i = 0; i < gather_i.si; i++)
    {
        for(int j = 0; j < gather_i.sj; j++)
        {
            for(int k = 0; k < gather_j.si; k++)
            {
                for(int l = 0; l < gather_j.sj; l++)
                {
                    if(gather_i.top_corners[i_q][0] + i ==  gather_j.top_corners[j_q][0] + k && gather_i.top_corners[i_q][1] + j == gather_j.top_corners[j_q][1] + l)
                    {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

int check_intersection2(struct Private_struct gather, struct Sneaker_struct sneaker, int g_id, int s_id)
{
    for(int i = 0; i < gather.si; i++)
    {
        for(int j = 0; j < gather.sj; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                for(int l = 0; l < 3; l++)
                {
                    if(gather.top_corners[g_id][0] + i ==  sneaker.litter_centers[s_id][0] + k - 1 && gather.top_corners[g_id][1] + j == sneaker.litter_centers[s_id][1] + l - 1)
                    {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

struct Commander simplify_orders(struct Commander command_list, int num_privates)
{
    struct Commander new_command_list;
    new_command_list.privates = command_list.privates;
    new_command_list.num_privates = command_list.num_privates;
    new_command_list.sneakers = command_list.sneakers;
    new_command_list.num_sneakers = command_list.num_sneakers;
    new_command_list.num_orders = command_list.num_orders;
    int index = 0;
    int index2 = 0;
    while (command_list.time_order[index])
    {
        if(command_list.command[index] == 2)
        {
            break;
        }
        if(index != 0)
        {
            if (command_list.command[index] != command_list.command[index - 1])
            {
                index2++;
            }
        }
        else
        {
            if(command_list.command[index] == 0)
            {
                index2++;
            }
        }
        index++;
    }
    new_command_list.command = (int*)malloc((index + 1)*sizeof(int));
    new_command_list.time_order = (int*)malloc((index + 1)*sizeof(int));

    int i = 0;
    int index3 = 0;
    while (command_list.time_order[index3])
    {
        if(command_list.command[index3] == 2)
        {
            new_command_list.command[i] = command_list.command[index3];
            new_command_list.time_order[i] = command_list.time_order[index3];
            break;
        }
        if(index3 != 0)
        {
            if (command_list.command[index3] != command_list.command[index3 - 1])
            {
                new_command_list.command[i] = command_list.command[index3];
                new_command_list.time_order[i] = command_list.time_order[index3];
                i++;
            }
        }
        else
        {
            if(command_list.command[index3] == 0)
            {
                new_command_list.command[i] = command_list.command[index3];
                new_command_list.time_order[i] = command_list.time_order[index3];
                i++;
            }
        }
        index3++;
    }
    return new_command_list;
}

void *give_command( void* arguments)
{
    struct Commander *orders = arguments;
    struct Commander command_list = simplify_orders(*orders, orders->num_privates);

    for(int i = 0; i < command_list.num_orders; i++)
    {
        if(i == 0)
        {
            usleep(command_list.time_order[i] * 1000);
        }
        else
        {
            usleep((command_list.time_order[i] - command_list.time_order[i - 1]) * 1000);
        }
        
        if(command_list.command[i] == 0)
        {

            hw2_notify(action = ORDER_BREAK, 0, 0, 0);
            for(int m = 0; m < command_list.num_privates; m++)
            {
                hw2_notify(action = GATHERER_TOOK_BREAK, command_list.privates[m].gid, 0, 0);
            }
            sem_wait(&sem_break);
        }
        else if(command_list.command[i] == 1)
        {
            hw2_notify(action = ORDER_CONTINUE, 0, 0, 0);
            for(int m = 0; m < command_list.num_privates; m++)
            {
                hw2_notify(action = GATHERER_CONTINUED, command_list.privates[m].gid, 0, 0);
            }
            sem_post(&sem_break);
        }
        else
        {
            hw2_notify(action = ORDER_STOP, 0, 0, 0);
            for(int m = 0; m < command_list.num_privates; m++)
            {
                hw2_notify(action = GATHERER_STOPPED, command_list.privates[m].gid, 0, 0);
            }
            for(int m = 0; m < command_list.num_sneakers; m++)
            {
                hw2_notify(action = SNEAKY_SMOKER_STOPPED, command_list.sneakers[m].sid, 0, 0);
            } 
            // sem_wait(&sem_break);
        }
    }
    usleep((orders->time_order[-1] - command_list.time_order[-1]) * 1000);
}


void *collect_cigg_butts( void* arguments)
{
    struct Private_struct *private_prop = arguments;
    hw2_notify(action = GATHERER_CREATED, (*private_prop).gid, 0, 0);
    
    for(int num = 0; num < (*private_prop).ng; num++)
    {
        sem_wait(&gatherers_prior[(*private_prop).queue][gatherers_job[(*private_prop).queue]]);
        hw2_notify(action = GATHERER_ARRIVED, (*private_prop).gid, (*private_prop).top_corners[num][0],(*private_prop).top_corners[num][1]);
        for(int j = 0; j < (*private_prop).sj; j++)
        {
            for(int i = 0; i < (*private_prop).si; i++ )
            {
                int coord_i = (*private_prop).top_corners[num][0] + i;
                int coord_j = (*private_prop).top_corners[num][1] + j;
                while(cig_butts[coord_i][coord_j] > 0)
                {
                    
                    for(int t = 0; t < (*private_prop).time / 10; t++)
                    {
                        sem_wait(&sem_break);
                        sem_post(&sem_break);
                        usleep(10000);
                    }


                    sem_wait(&lock);
                    cig_butts[coord_i][coord_j]--;
                    hw2_notify(action = GATHERER_GATHERED, (*private_prop).gid, coord_i,coord_j);
                    sem_post(&lock);
                }
            }
        }
        gatherers_job[(*private_prop).queue]++;
        hw2_notify(action = GATHERER_CLEARED, (*private_prop).gid, 0, 0);
    }
    hw2_notify(action = GATHERER_EXITED, (*private_prop).gid, 0, 0);
}

void *litter_cigg_butts( void* arguments)
{
    struct Sneaker_struct *sneaker_prop = arguments;
    hw2_notify(action = SNEAKY_SMOKER_CREATED, (*sneaker_prop).sid, 0, 0);
    for(int num = 0; num < (*sneaker_prop).ns; num++)
    {
        sem_wait(&sneakers_prior[(*sneaker_prop).queue][sneakers_job[(*sneaker_prop).queue]]);
        hw2_notify(action = SNEAKY_SMOKER_ARRIVED, (*sneaker_prop).sid, (*sneaker_prop).litter_centers[num][0],(*sneaker_prop).litter_centers[num][1]);
        for(int i = 0; i < (*sneaker_prop).cigg_num[num]; i++ )
        {
            int coord_i = (*sneaker_prop).litter_centers[num][0] + sneaker_cord[i % 8][0];
            int coord_j = (*sneaker_prop).litter_centers[num][1] + sneaker_cord[i % 8][1];

            usleep((*sneaker_prop).time * 1000);

            sem_wait(&lock);
            cig_butts[coord_i][coord_j]++;
            hw2_notify(action = SNEAKY_SMOKER_FLICKED, (*sneaker_prop).sid, coord_i,coord_j);
            sem_post(&lock);
        }
        sneakers_job[(*sneaker_prop).queue]++;
        hw2_notify(action = SNEAKY_SMOKER_LEFT, (*sneaker_prop).sid, 0, 0);
    }
    hw2_notify(action = SNEAKY_SMOKER_EXITED, (*sneaker_prop).sid, 0, 0);
}

void *intersection_check(void* arguments)
{
    struct Intersection_struct *info = arguments;
    struct Private_struct* gathers = info->gatherer_info;
    int num_gathers = info->num_gatherer;
    struct Sneaker_struct* sneakers = info->sneaker_info;
    int num_sneakers = info->num_sneaker;
    int* running_gatherer = (int*)malloc(sizeof(int)*num_gathers);
    while (!job_done(gatherers_job, sneakers_job, info))
    {
        for(int i = 0; i < num_gathers; i++)
        {
            if(gatherers_job[i] == gathers[i].ng)
            {
                running_gatherer[i] = 0;
                continue;
            }
            int is_Okay = 1;
            for(int j = 0; j < i; j++)
            {
                if(gatherers_job[j] == gathers[j].ng || running_gatherer[j] == 0)
                {
                    continue;
                }

                int intersection = check_intersection(gathers[i], gathers[j], gatherers_job[i], gatherers_job[j]);
                if(intersection)
                {
                    is_Okay = 0;
                }
            }
            if(is_Okay)
            {
                sem_post(&gatherers_prior[i][gatherers_job[i]]);
                running_gatherer[i] = 1;
            }
            else
            {
                running_gatherer[i] = 0;
            }
        }

        for(int i = 0; i < num_sneakers; i++)
        {
            if(sneakers_job[i] == sneakers[i].ns)
            {
                continue;
            }
            int is_Okay = 1;
            for(int j = 0; j < num_gathers; j++)
            {
                if(gatherers_job[j] == gathers[j].ng)
                {
                    continue;
                }
                int intersection = check_intersection2(gathers[j], sneakers[i], gatherers_job[j], sneakers_job[i]);
                if(intersection)
                {
                    is_Okay = 0;
                }
            }
            for(int j = 0; j < i; j++)
            {
                if(sneakers_job[j] == sneakers[j].ns)
                {
                    continue;
                }
                if(sneakers[i].litter_centers[sneakers_job[i]][0] == sneakers[j].litter_centers[sneakers_job[j]][0] && 
                sneakers[i].litter_centers[sneakers_job[i]][1] == sneakers[j].litter_centers[sneakers_job[j]][1])
                {
                    is_Okay = 0;
                }
            }


            if(is_Okay)
            {
                sem_post(&sneakers_prior[i][sneakers_job[i]]);
            }
        }

        
    }
}

int main()
{
    int gridx, gridy;
    int num_privates;
    int num_orders;
    int num_sneakers;
    char cmd[256]; 
    scanf("%d%d", &gridx, &gridy);
    cig_butts = (int**)malloc(gridx * sizeof(int*));
    for(int i = 0; i < gridx; i++)
    {
        cig_butts[i] = (int*)malloc(gridy * sizeof(int));
        for(int j = 0; j < gridy; j++)
        {
            scanf("%d", &cig_butts[i][j]);
        }
    }
    sem_init(&lock, 0, 1);
    sem_init(&sem_break, 0, 1);
    scanf("%d", &num_privates);
    struct Private_struct* privates;
    struct Commander commander_orders;
    struct Sneaker_struct* sneakers;
    struct Intersection_struct info;
    commander_orders.num_privates = num_privates;
    pthread_t private_thread[num_privates];
    pthread_t intersection_thread;
    pthread_t commander_thread;
    pthread_t trial_thread;

    privates = (struct Private_struct*)malloc(sizeof(struct Private_struct)* num_privates);
    gatherers_prior = (sem_t**)malloc(sizeof(sem_t*) * num_privates);
    for(int i = 0; i < num_privates; i++)
    {
        scanf("%d%d%d%d%d", &privates[i].gid, &privates[i].si, &privates[i].sj, &privates[i].time, &privates[i].ng);
        gatherers_prior[i] = (sem_t*)calloc(privates[i].ng, sizeof(sem_t));
        privates[i].queue = i;
        privates[i].top_corners = (int**)malloc(privates[i].ng * sizeof(int*));
        for(int j= 0; j < privates[i].ng; j++)
        {
            privates[i].top_corners[j] = (int*)malloc(2 * sizeof(int));
            scanf("%d%d", &privates[i].top_corners[j][0], &privates[i].top_corners[j][1]);
            sem_init(&gatherers_prior[i][j], 0, 0);
        }
    }
    info.num_gatherer = num_privates;
    gatherers_job = (int*)malloc(sizeof(int)* num_privates);
    commander_orders.privates = privates;
    scanf("%d", &num_orders);
    commander_orders.command = (int*)malloc(num_orders*sizeof(int));
    commander_orders.time_order = (int*)malloc(num_orders*sizeof(int));
    commander_orders.num_orders = num_orders;
    for(int i = 0; i < num_orders; i++)
    {
        scanf("%d", &commander_orders.time_order[i]);
        fgets(cmd, sizeof(cmd), stdin);
        if(cmd[1] == 'b')
        {
            commander_orders.command[i] = 0;
        }
        else if(cmd[1] == 'c')
        {
            commander_orders.command[i] = 1;
        }
        else if(cmd[1] == 's')
        {
            commander_orders.command[i] = 2;
        }
    }

    scanf("%d", &num_sneakers);
    pthread_t sneakers_thread[num_sneakers];
    sneakers = (struct Sneaker_struct*)malloc(sizeof(struct Sneaker_struct)* num_sneakers);
    sneakers_prior = (sem_t**)malloc(sizeof(sem_t*) * num_sneakers);
    sneakers_job = (int*)malloc(sizeof(int)* num_sneakers);
    commander_orders.num_sneakers = num_sneakers;
    for(int i = 0; i < num_sneakers; i++)
    {
        scanf("%d%d%d", &sneakers[i].sid, &sneakers[i].time, &sneakers[i].ns);
        sneakers_prior[i] = (sem_t*)malloc(sizeof(sem_t) * sneakers[i].ns);
        sneakers[i].litter_centers = (int**)malloc(sneakers[i].ns * sizeof(int*));
        sneakers[i].cigg_num = (int*)malloc(sneakers[i].ns * sizeof(int));
        sneakers[i].queue = i;
        for(int j= 0; j < sneakers[i].ns; j++)
        {
            sneakers[i].litter_centers[j] = (int*)malloc(2 * sizeof(int));
            scanf("%d%d%d", &sneakers[i].litter_centers[j][0], &sneakers[i].litter_centers[j][1],&sneakers[i].cigg_num[j]);
            sem_init(&sneakers_prior[i][j], 0, 0);
        }
    }
    commander_orders.threads = malloc(sizeof(pthread_t)*(num_privates + num_sneakers + 1));
    info.gatherer_info = privates;
    info.sneaker_info = sneakers;
    info.num_sneaker = num_sneakers;
    commander_orders.sneakers = sneakers;
    for(int i = 0; i < num_privates; i++)
    {
        commander_orders.threads[i] = private_thread[i];
    }
    for(int i = 0; i < num_sneakers; i++)
    {
        commander_orders.threads[num_privates + i] = sneakers_thread[i];
    }
    commander_orders.threads[num_privates + num_sneakers] = intersection_thread;
    pthread_create( &intersection_thread, NULL, &intersection_check, (void*)&info);
    pthread_create( &commander_thread, NULL, &give_command, (void*)&commander_orders);

    hw2_init_notifier();
    for(int i = 0; i < num_privates; i++)
    {
        pthread_create( &private_thread[i], NULL, &collect_cigg_butts, (void*)&privates[i]);
    }

    for(int i = 0; i < num_sneakers; i++)
    {
        pthread_create( &sneakers_thread[i], NULL, &litter_cigg_butts, (void*)&sneakers[i]);
    }

    pthread_join(commander_thread, NULL);

    pthread_join(intersection_thread, NULL);
    for(int i = 0; i < num_privates; i++)
    {
        pthread_join( private_thread[i], NULL);
    }

    return 0;

}