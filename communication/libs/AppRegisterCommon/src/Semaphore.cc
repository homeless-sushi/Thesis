#include "AppRegisterCommon/Semaphore.h"

#include <fstream>
#include <iostream>

#include <cstring> 
#include <cerrno>

#include <sys/sem.h>

int binarySemaphoreWait(int semId)
{
    struct sembuf operations[1];
    operations[0].sem_num = 0;
    operations[0].sem_op  = -1;
    operations[0].sem_flg = SEM_UNDO;
    int error = semop(semId, operations, 1);
    if (error == -1){
        std::cerr << "ERROR: During semaphore wait, " << std::strerror(errno) << std::endl;
        return -1;
    }

    return 0;
}

int binarySemaphorePost(int semId) 
{
    struct sembuf operations[1];
    operations[0].sem_num = 0;
    operations[0].sem_op  = 1;
    operations[0].sem_flg = SEM_UNDO;
    int error = semop (semId, operations, 1);
    if (error == -1){
        std::cerr << "ERROR: During semaphore post, " << std::strerror(errno) << std::endl;
        return -1; 
    }

    return 0;
}
