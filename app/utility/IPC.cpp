#include "IPC.h"

IPC::IPC(const std::string &shm_name, size_t size) : shm_size(size)
{
    // 创建共享内存
    shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        throw std::runtime_error("Failed to create shared memory");
    }

    // 设置共享内存大小
    if (ftruncate(shm_fd, shm_size) == -1)
    {
        throw std::runtime_error("Failed to set shared memory size");
    }

    // 映射共享内存
    shm_ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        throw std::runtime_error("Failed to map shared memory");
    }
}

IPC::~IPC()
{
    // 解除映射和关闭共享内存
    munmap(shm_ptr, shm_size);
    close(shm_fd);
    shm_unlink("/my_shm"); // 删除共享内存对象
}

void IPC::send_message(const std::string &message)
{
    std::lock_guard<std::mutex> lock(mtx); // 保护共享内存写入
    std::memset(shm_ptr, 0, shm_size);     // 清空共享内存
    std::strncpy(static_cast<char *>(shm_ptr), message.c_str(), shm_size - 1);
}

std::string IPC::recv_message()
{
    std::lock_guard<std::mutex> lock(mtx); // 保护共享内存读取
    return std::string(static_cast<char *>(shm_ptr));
}

void IPC::clear_memory()
{
    std::lock_guard<std::mutex> lock(mtx); // 保护共享内存清空
    std::memset(shm_ptr, 0, shm_size);     // 清空共享内存
}
