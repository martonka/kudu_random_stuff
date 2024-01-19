#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>

constexpr int block_size = 4096;
//constexpr int block_size = 65536;

#define pvar(x) do { std::cout << #x << " =  " << x << std::endl;} while(0)
#define check_call(x)                                                                                         \
    do                                                                                                   \
    {                                                                                                    \
        if (int res = x; res != 0)                                                                        \
        {                                                                                                \
             std::cerr << "Error executing " << #x << " res: " << res << " errno: " << errno << std::endl;  \
        }                                                                                                \
    } while (0)

typedef struct xfs_flock64 {
        __s16           l_type;
        __s16           l_whence;
        __s64           l_start;
        __s64           l_len;          /* len == 0 means until end of file */
        __s32           l_sysid;
        __u32           l_pid;
        __s32           l_pad[4];       /* reserve area                     */
} xfs_flock64_t;

#define XFS_IOC_UNRESVSP64      _IOW ('X', 43, struct xfs_flock64)

void print_byte_at(int fd, int pos) {
    lseek(fd, pos, SEEK_SET);
    char buff[2];
    read(fd, buff, 2);
    std::cout << "Value at " << pos << " is " << int(buff[0]) << std::endl; 
}

void print_size(const std::string& fname) {
    struct stat sbuf;
    if (stat(fname.c_str(), &sbuf) != 0) {
      std::cerr << "Error: " <<  errno << std::endl;
      return;
    }
    ssize_t size = sbuf.st_blocks * 512;
    std::cout << "disk usage of file is: " << size << std::endl;    
}

void chek_file(int fd, const std::string& fname) {
    print_size(fname);
    for(int i = 10; i < int(3.5 * block_size); i += block_size / 4) {
        print_byte_at(fd, i);
    }

}

int main(int argc, const char *argv[])
{
    std::string fname = "alma.txt";
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    std::cout << "opening file" << std::endl;
    int fd = open(fname.c_str(), flags);
    chek_file(fd, fname);   
    std::cout << "After read:" << std::endl;
    print_size(fname); 

    std::cout << "Allocating file " << int(3.5 * block_size) << std::endl;
    check_call((fallocate(fd, 0, 0, int(3.5 * block_size))));
    chek_file(fd, fname);    


    std::string s(int(3.25*block_size), 'a');
    
    std   ::cout << "Write data " << int(3.25*block_size) << std::endl;
    lseek(fd, 0, SEEK_SET);
    write(fd, s.data(), s.size());
    chek_file(fd, fname);    

    xfs_flock64_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.l_start = int(0.5 * block_size);
    cmd.l_len = 2 * block_size;
    std ::cout <<  "Punch hole from: " << cmd.l_start << " length: " << cmd.l_len  << std::endl;
    check_call(ioctl(fd, XFS_IOC_UNRESVSP64, &cmd));
    chek_file(fd, fname);    

}