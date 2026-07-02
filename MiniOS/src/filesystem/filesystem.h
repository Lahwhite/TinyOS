#pragma once

#include <string>
#include <map>
#include <vector>

struct Inode {
    int inode_num;
    bool is_directory;
    int size;
    int ref_count;
    std::string permissions;
    std::string content;
    std::map<std::string, int> entries;
};

class FileSystem {
public:
    FileSystem();

    void printTree() const;

    // 在当前目录下创建子目录，返回新 inode 号（失败返回 -1）
    int mkdir(const std::string& name);
    // 在当前目录下创建空文件，返回新 inode 号（失败返回 -1）
    int touch(const std::string& name);
    // 进入子目录（cd），返回 false 如果目录不存在
    bool cd(const std::string& name);
    // 返回上级目录（cd ..）
    void cdUp();
    // 列出当前目录内容（ls）
    void ls() const;
    // 返回当前路径字符串（用于提示符显示）
    std::string pwd() const;

    // 向当前目录下名为 name 的文件写入 content（覆盖式写入），成功返回写入字节数，失败返回 -1
    int write(const std::string& name, const std::string& content);
    // 读取当前目录下名为 name 的文件内容，成功返回内容字符串，失败返回空字符串
    std::string read(const std::string& name);
    // 查看文件的 inode 信息（stat）
    void stat(const std::string& name) const;

private:
    std::vector<Inode> inodes;     // inode 表（下标即 inode 号）
    std::vector<bool>  inode_used; // 每个 inode 是否在使用
    int next_inode_num;            // 下一个可分配的 inode 号

    int current_dir_inode;         // 内部维护：当前目录 inode 号 和 路径栈
    
    std::vector<std::pair<std::string, int>> path_stack;  // (目录名, inode号)

    // 工具函数：分配一个新 inode，返回其编号
    int allocateInode();

    void printTreeRecursive(int inode_num, std::string& prefix) const;
};