#include <fs.h>
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name; // 文件名 把目录分隔符/也认为是文件名的一部分
  size_t size; // 文件大小
  size_t disk_offset; // 文件在ramdisk中的偏移
  ReadFn read;  // 读函数指针
  WriteFn write; // 写函数指针
} Finfo;

static size_t* file_offset = NULL;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENT, FD_DISPINFO};

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
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENT] = {"/dev/event", 0, 0, events_read, invalid_write},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, invalid_write},  // 支持写操作和lseek
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write}, // 屏幕信息 支持读操作
#include "files.h"
};

void init_file_offset(){
  int file_num = sizeof(file_table) / sizeof(Finfo);
  file_offset = malloc(file_num * sizeof(size_t));
  for(int i = 0; i < file_num; i ++){
    file_offset[i] = file_table[i].disk_offset;
  }
}

void free_file_offset(){
  free(file_offset);
  file_offset = NULL;
}

char *get_filename(int fd){
  return file_table[fd].name;
}

// 为了简化实现, 我们允许所有用户程序都可以对所有已存在的文件进行读写, 
// 这样以后, 我们在实现fs_open()的时候就可以忽略flags和mode了
int fs_open(const char *pathname, int flags, int mode){
  if(file_offset == NULL)
    init_file_offset();
  // find the certain file
  int file_num = sizeof(file_table) / sizeof(Finfo);
  // printf("pathname = %s file_num = %d\n",pathname,file_num);
  int fd = 0;
  for(fd = 0; fd < file_num; fd ++){
    printf("file_table[%d].name = %s\n",fd,file_table[fd].name);
    if(strcmp(file_table[fd].name, pathname) == 0){
      break;
    }
    if(fd == file_num - 1) panic("Cannot find this file!  [%s]",pathname);
  }
  // printf("fd = %d\n",fd);
  return fd;

}

size_t fs_read(int fd, void *buf, size_t len){
  
  if(file_table[fd].write == NULL){
    assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  assert(buf != NULL);
  assert(len <= 0x7ffff000);
  if(file_table[fd].disk_offset + len > file_offset[fd] + file_table[fd].size)
    len = file_offset[fd] + file_table[fd].size - file_table[fd].disk_offset;
  // printf("len = 0x%08x file_table[%d].disk_offset = 0x%08x file_offset[%d] = 0x%08x file_table[%d].size = 0x%08x",len,fd,file_table[fd].disk_offset,fd,file_offset[fd],fd,file_table[fd].size);
  // printf("file_table[fd].disk_offset + len = 0x%08x, file_offset[fd] + file_table[fd].size = 0x%08x\n",file_table[fd].disk_offset + len,file_offset[fd] + file_table[fd].size);
  int ret = ramdisk_read(buf, file_table[fd].disk_offset, len);
  // assert(file_table[fd].disk_offset + ret <= file_offset[fd] + file_table[fd].size);
 
  file_table[fd].disk_offset += ret;
  return ret;
  // printf("fs_write fd = %d len = %d\n",fd,len);
  }
  else{
    return file_table[fd].read(buf,file_offset[fd],len);
  }
}

size_t fs_write(int fd, const void *buf, size_t len){
  // printf("fs_write fd = %d len = %d\n",fd,len);
  if(file_table[fd].write == NULL){
    assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
    assert(buf != NULL);
    assert(len <= 0x7ffff000);
    assert(file_table[fd].disk_offset + len <= file_offset[fd] + file_table[fd].size);
    int ret = ramdisk_write(buf, file_table[fd].disk_offset, len);
    file_table[fd].disk_offset += ret;
    return ret;
  }
  else{
    return file_table[fd].write(buf,file_offset[fd],len);
  }
  
}

// size_t fs_write(int fd, const void *buf, size_t len){
//   // printf("fs_write fd = %d len = %d\n",fd,len);
//   if(fd == 1 || fd == 2){  // stdout stderr
//     int i = 0; 
//     for(i = 0; i < len; i ++){
//       putch(*((char*)buf + i));
//     }
//     // printf("i = %d\n",i);
//     return i;
//   }
//   else{
//     assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
//     assert(buf != NULL);
//     assert(len <= 0x7ffff000);
//     assert(file_table[fd].disk_offset + len <= file_offset[fd] + file_table[fd].size);
//     int ret = ramdisk_write(buf, file_table[fd].disk_offset, len);
//     file_table[fd].disk_offset += ret;
//     return ret;
//   }
  
// }

// enum {SEEK_SET = 0, SEEK_CUR, SEEK_END};


size_t fs_lseek(int fd, size_t offset, int whence){
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  // printf("offset=0x%08x whence=%d file_table[%d].disk_offset=0x%08x\n",offset,whence,fd,file_table[fd].disk_offset);
  // printf("file_offset[fd] = 0x%08x file_table[fd].size = 0x%08x\n",file_offset[fd],file_table[fd].size);
  if(whence == SEEK_SET){
    assert(offset <= file_table[fd].size);
    file_table[fd].disk_offset = file_offset[fd] + offset;
  }
  else if(whence == SEEK_CUR){
    assert(file_table[fd].disk_offset + offset <= file_offset[fd] + file_table[fd].size);
    file_table[fd].disk_offset += offset;
  }
  else if(whence == SEEK_END){
    assert(file_offset[fd] + offset + file_table[fd].size <= file_offset[fd] + file_table[fd].size);
    file_table[fd].disk_offset = file_offset[fd] + offset + file_table[fd].size;
  }
  // printf("offset=%d whence=%d file_table[%d].disk_offset=0x%08x\n",offset,whence,fd,file_table[fd].disk_offset);
  return file_table[fd].disk_offset - file_offset[fd];
}

int fs_close(int fd){
  assert(fd >= 0 && fd < sizeof(file_table) / sizeof(Finfo));
  free_file_offset();
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
