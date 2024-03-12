#include <fs.h>
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name; // 文件名 把目录分隔符/也认为是文件名的一部分
  size_t size; // 文件大小
  size_t disk_offset; // 文件在ramdisk中的偏移
  size_t inner_offset; // 文件偏移量
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = { // 文件记录表
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

// 为了简化实现, 我们允许所有用户程序都可以对所有已存在的文件进行读写, 
// 这样以后, 我们在实现fs_open()的时候就可以忽略flags和mode了
int fs_open(const char *pathname, int flags, int mode){
  // find the certain file
  int file_num = sizeof(file_table) / sizeof(Finfo);
  printf("file_num = %d\n",file_num);
  int fd = 0;
  for(fd = 0; fd < file_num; fd ++){
    printf("file_table[%d].name = %s\n",fd,file_table[fd].name);
    if(strcmp(file_table[fd].name, pathname) == 0){
      break;
    }
    if(fd == file_num - 1) panic("Cannot find this file!");
  }

  return fd;

}

size_t fs_read(int fd, void *buf, size_t len){
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  assert(buf != NULL);
  assert(len <= 0x7ffff000);
  assert(file_table[fd].inner_offset + len <= file_table[fd].disk_offset + file_table[fd].size);
  int ret = ramdisk_read(buf, file_table[fd].inner_offset, len);
  file_table[fd].inner_offset += ret;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len){
  if(fd == 1 || fd == 0){  // stdout stderr
    int i = 0; 
    for(i = 0; i < len; i ++){
      putch(*((char*)buf + i));
    }
    return i;
  }
  else{
    assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
    assert(buf != NULL);
    assert(len <= 0x7ffff000);
    assert(file_table[fd].inner_offset + len <= file_table[fd].disk_offset + file_table[fd].size);
    int ret = ramdisk_write(buf, file_table[fd].inner_offset, len);
    file_table[fd].inner_offset += ret;
    return ret;
  }
  
}

// enum {SEEK_SET = 0, SEEK_CUR, SEEK_END};


size_t fs_lseek(int fd, size_t offset, int whence){
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  int pre = file_table[fd].inner_offset;
  if(whence == SEEK_SET){
    assert(offset <= file_table[fd].size);
    file_table[fd].inner_offset = file_table[fd].disk_offset + offset;
  }
  else if(whence == SEEK_CUR){
    assert(file_table[fd].inner_offset + offset <= file_table[fd].disk_offset + file_table[fd].size);
    file_table[fd].inner_offset += offset;
  }
  else if(whence == SEEK_END){
    assert(file_table[fd].disk_offset + offset + file_table[fd].size <= file_table[fd].disk_offset + file_table[fd].size);
    file_table[fd].inner_offset = file_table[fd].disk_offset + offset + file_table[fd].size;
  }
  return file_table[fd].inner_offset - pre;
}

int fs_close(int fd){
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
