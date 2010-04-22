
#include <sys/syscall.h>
#include <sys/types.h>

int evilprint (char *);

static int
_write (int fd, void *buf, int count)
{
  long ret;

  __asm__ __volatile__ ("pushl %%ebx\n\t"
			"movl %%esi,%%ebx\n\t"
			"int $0x80\n\t"
			"popl %%ebx":"=a" (ret):"0" (SYS_write),
			"S" ((long) fd), "c" ((long) buf),
			"d" ((long) count));

  if (ret >= 0)
    {
      return (int) ret;
    }
  return -1;
}

int
evilprint (char *buf)
{
  char msg[200];
  msg[0] = 'f';
  msg[1] = 'r';
  msg[2] = 'e';
  msg[3] = 'd';
  msg[4] = ' ';
  msg[5] = 'l';
  msg[6] = 'e';
  msg[7] = ' ';
  msg[8] = 'b';
  msg[9] = 'e';
  msg[10] = 'a';
  msg[11] = 'u';
  msg[12] = ' ';
  msg[13] = 'g';
  msg[14] = 'o';
  msg[15] = 's';
  msg[16] = 's';
  msg[17] = 'e';
  msg[18] = '\n';
  msg[19] = 0;

  char nl[1];
  nl[0] = '\n';

  int (*origfunc) (char *p) = 0x00000000;

  /* just to demonstrate calling */
  /* a syscall from our shared lib */
  _write (1, (char *) msg, 18);

  /* pass our new arg to the original function */
//  origfunc (new_string);

  /*
   *   Remember this is an alternative way to transfer control back --
   *     __asm__ __volatile__
   *       ("movl %ebp, %esp\n" "pop %ebp\n" "movl $0x00000000, %eax\n" "jmp *%eax");
   *         */
}


void
_init ()
{
}

void
_fini ()
{
}
