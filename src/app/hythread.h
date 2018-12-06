//
// Created by chenh on 2018/12/5.
//

#ifndef AICAST_SDK_HYTHREAD_H
#define AICAST_SDK_HYTHREAD_H

#include <thread>
extern int hylength;
namespace hythread{

    std::thread& createThread();
    void startCountThread();
    void stopCountThread();
}












#endif //AICAST_SDK_THREAD_H
