#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;
    p->exitCode=exitcode;
    p>exit=__WEXITED;
  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  //proc_destroy(p); dont destroy the process here, we might need it later
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
  *retval = curthread->t_proc->p_pid;
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;
  lock_acquire(lockForTable);
    struct proc *process=getProcessFromPid(pid); //get the process structure for the parent
    if(process->parent!=curproc) //meaning that the parent is not calling the 
    {
        status=-1;
        lock_release(lockForTable);
        return EAGAIN;
        
    }
  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */
    if(processList[process]==NULL) //that is the child process is NULL
    {
        lock_release(lockForTable);
        return 
    }
  if (options != 0) {
  lock_release(lockForTable);
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

pid_t sys_fork(struct trapframe *tf)
{
    
    const char childName="ChildProcess";
    struct proc childProcess=proc_create_runprogram(childName)   ; //Creates the process for the child
    if(childProcess==NULL)
    {
        panic("Could Not Create child process\n");
    }
    struct trapframe copy=kmalloc(sizeof((struct trapframe *tf)));
    copy=tf;
    struct addrspace *childProcessAddress;
    int flag=as_copy(childProcess->p_addrspace,&childProcessAddress);
    if(flag==ENOMEM)
    {
        //Means out of memory 
        tf->tf_v0=ENOMEM;
        tf->tf_a3=1;
        return -1;
    }
    int spl=splhigh();
    int res=thread_fork(childName,childProcess, enter_forked_process,copy,childProcessAddress);
    enter_forked_process(tf);
    //loading the success parameters
    tf->tf_v0=childProcess->p_pid;
    tf->tf_a3=0;
    splx(spl);
    return childProcess->p_pid;

    
}

