#include <iostream>

#include "filesystem.h"

FileSystem::FileSystem() {
    inodes.resize(64);
    inode_used.resize(64, false);

    inode_used[0] = true;
    inodes[0].inode_num = 0;
    inodes[0].is_directory = true;
    inodes[0].ref_count = 2;
    inodes[0].permissions = "rwxr-xr-x";
    inodes[0].entries["."]  = 0;   //（"." 指向自身）
    inodes[0].entries[".."] = 0;   //（根目录的 ".." 也指向自身）
    next_inode_num = 1;

    current_dir_inode = 0;

    // path_stack.push_back({"/", 0});
}

void FileSystem::printTree() const {
    std::cout << "/\n";
    std::string prefix = "";
    printTreeRecursive(0, prefix);
}

int FileSystem::mkdir(const std::string& name) {
    Inode& curr_inode = inodes[current_dir_inode];
    if (curr_inode.entries.find(name) != curr_inode.entries.end()) {
        std::cout << "该文件夹中已经有该名称的文件\n"; 
        return -1;
    }
    int new_file_inode = allocateInode();
    if (new_file_inode == -1) { return -1; }
    inodes[new_file_inode].inode_num = new_file_inode;
    inodes[new_file_inode].is_directory = true;
    inodes[new_file_inode].ref_count = 2;
    inodes[new_file_inode].entries["."]  = new_file_inode;
    inodes[new_file_inode].entries[".."] = current_dir_inode;

    curr_inode.entries[name] = new_file_inode;
    curr_inode.ref_count ++;

    std::string cur_path = pwd();
    if (cur_path == "/") cur_path = "";
    std::cout << "[FS] mkdir: " << cur_path << "/" << name << "(" << new_file_inode << ")\n";
    return new_file_inode;
}

int FileSystem::touch(const std::string& name) {
    Inode& curr_inode = inodes[current_dir_inode];
    if (curr_inode.entries.find(name) != curr_inode.entries.end()) {
        std::cout << "该文件夹中已经有该名称的文件\n"; 
        return -1;
    }
    int new_file_inode = allocateInode();
    if (new_file_inode == -1) { return -1; }
    inodes[new_file_inode].inode_num = new_file_inode;
    inodes[new_file_inode].is_directory = false;
    inodes[new_file_inode].ref_count = 1;

    curr_inode.entries[name] = new_file_inode;
    curr_inode.ref_count ++;

    std::string cur_path = pwd();
    std::cout << "[FS] touch: " << cur_path << "/" << name << "(" << new_file_inode << ")\n";
    return new_file_inode;
}

bool FileSystem::cd(const std::string& name) {
    if (name == "..") { cdUp(); return true; }
    Inode& name_inode = inodes[current_dir_inode];
    if (name_inode.entries.find(name) == name_inode.entries.end()) {
        std::cout << "没有这个目录\n";
        return false;
    }
    int target_inode = name_inode.entries[name];
    if (!inodes[target_inode].is_directory) {
        std::cout << "该文件不是文件夹\n";
        return false;
    }
    current_dir_inode = target_inode;
    path_stack.push_back({name, current_dir_inode});

    std::string cur_path = pwd();
    std::cout << "[FS] cd: /" << name << "\n";
    return true;
}

void FileSystem::cdUp() {
    if (path_stack.empty()) {return;}
    path_stack.pop_back();
    current_dir_inode = path_stack.empty() ? 0 : path_stack.back().second;
}

void FileSystem::ls() const {
    for (auto inode : inodes[current_dir_inode].entries) {
        if (inode.first == "." || inode.first == "..") continue;
        std::cout << inode.second;
        if (inodes[inode.second].is_directory) std::cout << " dir ";
        else std::cout << " txt ";
        std::cout << inode.first << "\n";
    }
}

std::string FileSystem::pwd() const {
    std::string cur_path;
    if (path_stack.empty()) return "/";
    for (auto inode : path_stack) {
        cur_path += "/" + inode.first;
    }
    return cur_path;
}

int FileSystem::write(const std::string& name, const std::string& content) {
    if (inodes[current_dir_inode].entries.find(name) == inodes[current_dir_inode].entries.end()) {
        std::cout << "该文件不存在！\n";
        return -1;
    }
    int inode_num = inodes[current_dir_inode].entries[name];
    inodes[inode_num].content = content;
    inodes[inode_num].size = content.size();

    std::string cur_path = pwd();
    if (cur_path == "_") cur_path = "";
    std::cout << "[FS] write: " << cur_path << "/" << name << ", 写入 " << content.size() << " 字节\n";
    return content.size();
}

std::string FileSystem::read(const std::string& name) {
    if (inodes[current_dir_inode].entries.find(name) == inodes[current_dir_inode].entries.end()) {
        std::cout << "该文件不存在！\n";
        return "";
    }
    int inode_num = inodes[current_dir_inode].entries[name];

    std::string cur_path = pwd();
    if (cur_path == "_") cur_path = "";
    std::cout << "[FS] read: " << cur_path << "/" << name << ", 读取 " << inodes[inode_num].content.size() << " 字节\n";
    return inodes[inode_num].content;
}

int FileSystem::allocateInode() {
    if (!inode_used[next_inode_num]) {
        int res = next_inode_num ++;
        inode_used[res] = true;
        while (inode_used[next_inode_num] && next_inode_num != res) 
            next_inode_num = (next_inode_num + 1) % 64;
        return res; 
    }
    std::cout << "没有多余文件空位可供分配!\n";
    return -1;
}

void FileSystem::printTreeRecursive(int inode_num, std::string& prefix) const {
    const Inode& dir = inodes[inode_num];

    int count = 0;
    int total = dir.entries.size() - 2;

    for (const auto& entry : dir.entries) {
        if (entry.first == "." || entry.first == "..") continue;

        count ++;
        bool is_last = (count == total);

        std::cout << prefix << (is_last ? "└──" : "├── ");
        if (inodes[entry.second].is_directory) {
            std::cout << entry.first << "/\n";
            std::string child_prefix = prefix + (is_last ? "    " : "│   ");
            printTreeRecursive(entry.second, child_prefix);
        } else std::cout << entry.first << "\n";
    }
}