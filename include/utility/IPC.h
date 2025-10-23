#ifndef IPC_H
#define IPC_H

#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <mutex>

class IPC
{
public:
    static IPC &getInstance(const std::string &shm_name = "/my_shm", size_t size = 1024)
    {
        static IPC instance(shm_name, size);
        return instance;
    }

    IPC(const IPC &) = delete;            // 禁止复制构造
    IPC &operator=(const IPC &) = delete; // 禁止赋值

    void send_message(const std::string &message);
    std::string recv_message();
    void clear_memory();

private:
    IPC(const std::string &shm_name, size_t size);
    ~IPC();

    int shm_fd;      // 共享内存文件描述符
    void *shm_ptr;   // 映射的共享内存指针
    size_t shm_size; // 共享内存大小
    std::mutex mtx;  // 互斥锁
};

#endif // IPC_H
