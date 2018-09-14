#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include "./include/epoll_client.h"
#include "./include/common_def.h"
#include "./include/cmd_def.h"
#include "./include/cmd_handler.h"

void* RunHandler(void* args)
{
    std::cout << "Start Run Client:\n";
    EpollClient* p = static_cast<EpollClient*>(args);
    printf("---------Client addr [%p]\n", p);
    if (!p)
    {
        std::cout << "null client\n";
        return nullptr;
    }

    p->Run();
    std::cout << "Finished Run Client:\n";
}

int main(int argc, char *argv[])
{
    std::cout << "Hello world!\n";
    EpollClient* p_epollclnt = new EpollClient(8888, "127.0.0.1");
    if (!p_epollclnt)
    {
        std::cout << "New Failed!\n";
        exit(-1);
    }

    pthread_t pd;
    pd = pthread_create(&pd, 0, RunHandler,(void*)(p_epollclnt));

    do
    {
        std::cout << "Please enter next operation[[q|Q]: quit, [c|C]:continue]: ";
        char c;
        scanf("%c", &c);
        if ('q' == c || 'Q' == c)
        {
            return 0;
        }
        else if ('c' == c || 'C' == c)
        {
            std::cout << "Please enter CMD: ";
            int cmd = 0;
            scanf("%d", &cmd);
            switch (cmd) {
            case CMD_TYPE_LOGIN:
            {
                printf("---------Client addr [%p]\n", p_epollclnt);
                if (p_epollclnt->GetClntPtr())
                {
                    int user_id = 0, last_read_id = 0;
                    scanf("%d%d", &user_id, &last_read_id);
                    int ret = LoginHandler::SendLogin(p_epollclnt->GetClntPtr(), user_id, last_read_id);
                    std::cout << "Input userid: [" << user_id << "], last_read_id: [" << last_read_id << "] ret [" << ret << "]\n";
                }
                else
                {
                    std::cout << "Err: without client\n";
                }
                break;
            }
            default:
                break;
            }
        }
    } while(true);

    SAVE_DELETE(p_epollclnt);
    return 0;
}
