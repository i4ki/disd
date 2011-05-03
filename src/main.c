#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#define SAY(format, x...)                       \
  fprintf(stdout, format, x)

typedef void (*FUNC)(void);

sigjmp_buf inst_l;
int cd_inst = 1;

void banner(char** argv)
{
  SAY("DISD - Discovery of Instruction Set Design.\n"
      "Usage: %s -cpu <cpu family>[,<cpu family>\n",
      *argv);

  exit(1);
}

void my_handler(int s, siginfo_t *info, void* context)
{
  printf("eba, instrução ilegal...\n");
  printf("signal number: %d\n", info->si_signo);
  printf("error number: %d\n", info->si_errno);
  printf("code number %d\n", info->si_code);
  printf("si_value: %d and %p\n", info->si_value.sival_int, info->si_value.sival_ptr);
  printf("Error at address: %p\n", info->si_addr);
  siglongjmp(inst_l, cd_inst++);
}

struct inst_set {
  const unsigned char insn[4];
  char* mnemonic;
};

const static struct inst_set pentium_all[] = {
  {{ 0xff, 0xff, 0xff, 0xff}, "ADD" },
  {{ 0x83, 0xc2, 0xa, 0x00}, "ADD2" }
};

int main(int argc, char** argv)
{
  struct sigaction sigIntHandler;
  int retval, i = 0, sz = sizeof (pentium_all)/sizeof(struct inst_set);;

  sigIntHandler.sa_sigaction = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = SA_SIGINFO;

  sigaction(SIGILL, &sigIntHandler, NULL);

  if ((retval = sigsetjmp(inst_l, 1)) != 0)
    {
      printf("returned using longjmp...\n");
      i++;

      if (i >= sz)
        goto end;
    }

  printf("executando instrução %d.\n", i);

  printf("%x %x %x %x\n",
         pentium_all[i].insn[0],
         pentium_all[i].insn[1],
         pentium_all[i].insn[2],
         pentium_all[i].insn[3]);

  FUNC function = (FUNC) pentium_all[i].insn;
  function();

 end:
  printf("eba, sai com sucesso.\n");
        
  return 0;
}
