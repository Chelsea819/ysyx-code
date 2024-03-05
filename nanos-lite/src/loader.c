#include <proc.h>
#include <elf.h>

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_RISCV32__)
# define EXPECT_TYPE EM_RISCV
#elif
# error Unsupported ISA
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr Elf_header;  // ELF文件头
  Elf_Phdr Elf_proc;    // 程序头
  char *buf = NULL;     // 存放段的buffer

  // 读取ELF header
  ramdisk_read(&Elf_header, 0, sizeof(Elf_header));
  // 检查文件格式
  printf("Elf_header.e_phoff: 0x%08x\n",Elf_header.e_phoff);
  printf("Elf_header.e_phnum: 0x%08x\n",Elf_header.e_phnum);
  assert(Elf_header.e_ident[0] == '\x7f' && memcmp(&(Elf_header.e_ident[1]), "ELF", 3) == 0);
  assert(Elf_header.e_type == ET_EXEC);

  // 检查架构
  assert(Elf_header.e_machine == EXPECT_TYPE);

  // 读段
  for(int n = 0; n < Elf_header.e_phnum; n ++){
    // 读程序头
    ramdisk_read(&Elf_proc, Elf_header.e_phoff + n * Elf_header.e_phentsize, sizeof(Elf_proc));
    // 是否需要加载
    if (Elf_proc.p_type == PT_LOAD){
      buf = malloc(Elf_proc.p_memsz);
      // 读取段
      ramdisk_read(buf, Elf_proc.p_offset, Elf_proc.p_filesz);
      printf("Elf_proc.p_vaddr: 0x%08x\n",Elf_proc.p_vaddr);
      // printf("Elf_proc.p_memsz: 0x%016x\n",Elf_proc.p_memsz);
      // printf("Elf_header.e_phoff: 0x%016x\n",Elf_header.e_phoff);
      // printf("Elf_header.e_phentsize: 0x%016x\n",Elf_header.e_phentsize);
      if(Elf_proc.p_memsz > Elf_proc.p_filesz)  
        memset(buf + Elf_proc.p_filesz, 0, Elf_proc.p_memsz - Elf_proc.p_filesz);
      
      // ramdisk_write(buf, Elf_proc.p_vaddr, Elf_proc.p_memsz);
      memcpy((uintptr_t*)Elf_proc.p_vaddr, buf, Elf_proc.p_memsz);
      free(buf);
      buf = NULL;
    }
  }

  return Elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

