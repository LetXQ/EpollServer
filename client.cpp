#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include "./include/epoll_client.h"
#include "./include/common_def.h"

void* RunHandler(void* args)
{
    EpollClient* p = static_cast<EpollClient*>(args);
    if (!p)
        return;
    p->Run();
}

int main(int argc, char *argv[])
{
    EpollClient* p_epollclnt = new EpollClient(8888, "127.0.0.1");
    if (!p_epollclnt)
    {
        std::cout << "New Failed!\n";
        exit(-1);
    }

    pthread_t pd;
    pd = pthread_create(&pd, 0, RunHandler, static_cast<void*>(p_epollclnt));

    char c;
    scanf("%c", &c);
    if ('q' == c || 'Q' == c)
    {
        return 0;
    }
    else if ('c' == c || 'C' == c)
    {

    }


    SAVE_DELETE(p_epollclnt);
    return 0;
}
