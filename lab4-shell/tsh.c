
/* 
 * tsh - A tiny shell program with job control
 * 
 * Marco Drochner
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
volatile int nextjid = 1;   /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */




struct job_t {              /* The ,job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);
int isValidChar(char c, int ifFirstDigit);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */


    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
    	default:
            usage();
    	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {
    	/* Read command line */
    	if (emit_prompt) {
    	    printf("%s", prompt);
    	    fflush(stdout);
    	}
    	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
    	    app_error("fgets error");
    	if (feof(stdin)) { /* End of file (ctrl-d) */
    	    fflush(stdout);
    	    exit(0);
    	}

    	/* Evaluate the command line */
    	eval(cmdline);
    	fflush(stdout);
    	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; //argument list for execve
    int bg; // if background
    pid_t pid; // return value of fork

    bg = parseline(cmdline, argv);
    if (argv[0] ==  NULL) // if line empty
        return;          // then ignore

    if (!builtin_cmd(argv)) {
        /* block SIGCHLD to avoid race */
            sigset_t mask, originalSet;
            sigemptyset(&mask);
            sigaddset(&mask, SIGCHLD);
            // Call and check return val for error
            if (sigprocmask(SIG_BLOCK, &mask, &originalSet))
                unix_error("System call error: sigprocmask");

        // If this process is the created child
        if ((pid = fork()) == 0) {
            /* unblock SIGCHLD for child */
            if (sigprocmask(SIG_SETMASK, &originalSet, NULL))
                unix_error("System call error: sigprocmask");
            if (setpgid(0, 0))
                unix_error("System call error: setpgid");

            if (execve(argv[0],argv, environ) < 0) 
                printf("%s: Command not found.\n", argv[0]);
        }
        assert(pid > 0);
        int thisJobID = nextjid;
        if (!addjob(jobs,  pid,  bg ? BG : FG,  cmdline))
            return;
        if (bg) printf("[%d] (%d) %s", thisJobID, pid, cmdline);
        /* unblock SIGCHLD */
        if (sigprocmask(SIG_SETMASK, &originalSet, NULL))
            unix_error("System call error: sigprocmask");
        if (!bg) waitfg(pid);
    }
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    if      (0 == strcmp("quit", argv[0]))
    {
        exit(0);
    }
    else if (0 == strcmp("fg",   argv[0]))
    {
        do_bgfg(argv);
    }
    else if (0 == strcmp("bg",   argv[0]))
    {
        do_bgfg(argv);
    }
    else if (0 == strcmp("jobs", argv[0]))
    {
        listjobs(jobs);
    }
    else return 0; // none match, so not a builtin command
    return 1; // was builtin command
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv)
{
    /**********************************************
     * Argument validation and extract pid, jid
     *********************************************/
    /* set to 0 to check later if pid or jid remain unset */
    int pid = 0;
    int jid = 0;

    /* ✅ ❎ validation check: has argument  */
    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument", argv[0]);
        return;
     }
    // named pointer to pid/jid arg for the sake of higher abstraction
    char *numberStr = argv[1]; 
    if (*(argv[1]) == '%') // if first char is '%'
        numberStr += sizeof(char); // chop off '%'
    /* ✅ ❎ validate format of each char */
    for (int i = 0; i < strlen(numberStr); i++)
        // Pass !i so that isValidChar handles the first digit differently
        // as only the first digit of a number may not be '0'
        if (!isValidChar(numberStr[i], !i)) {
            printf("%s: argument must be a PID or %%jobid", argv[0]);
            return;
        }
    // if arg is of a job id
    if (*(argv[1]) == '%') {
        /* convert numberStr to int */
        jid = atoi(numberStr);
        /* ✅ ❎ check if job exists */
        /* ✅ ❎ check if job exists */

        struct job_t *jobStruct = getjobjid(jobs, jid);
        if (jobStruct == NULL) {
            printf("(%s): No such job", argv[1]);
            return;
        }
        pid = jobStruct->pid;
    } else { // arg is of a pid
        /* convert numberStr to int */
        pid = atoi(numberStr);
        /* ✅ ❎ check if job exists */
        jid = pid2jid(pid); // returns 0 if non-existant
        if (!jid) {
            printf("(%s): No such process", argv[1]);
            return;
        }
    }
    // always a good practice to use asserts
    assert(jid && pid); 
    /********************************************
     * End argument validation and extract pid, jid
     ********************************************/
    
    /*********************
     * Put job in fg or bg
     *********************/
    /*
     * Block SIGCHLD, SIGINT, and SIGTSTP so they can't 
     * race the job into changing to a new state before 
     * the below code gets around to modifying state in jobs[]
     */
    sigset_t mask, originalSet;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);
    if (sigprocmask(SIG_BLOCK, &mask, &originalSet))
        unix_error("System call error: sigprocmask");

    if (0 == strcmp("fg", argv[0])) {
        /* resume job in foreground */

        // If error returned
        if (kill(-pid, SIGCONT)) {
            // If ernno == ESRCH, no problem, the processes
            // naturally terminated before it was put into fg
            if (errno != ESRCH)
                unix_error("System call error: kill");
        } else {
            jobs[jid].state = FG;
            waitfg(pid);
        }
    }
    else {
        /* resume job in background */

        jobs[jid].state = BG;
        if (kill(-pid, SIGCONT)) {
            // If error is ESRCH, no problem. Processes was
            // already in bg and naturally terminated sometime
            // between checking if the process exists and kill()
            if (errno != ESRCH)
                unix_error("System call error: kill");
        }
    }

    /* unblock signals */
    if (sigprocmask(SIG_SETMASK, &originalSet, NULL))
        unix_error("System call error: sigprocmask");
}

/*
 * Custom alternative to the confusing isdigit()
 */
int isValidChar(char c, int isFirstDigit) {
    /* jump table that returns 0 if and 
     * only if c is not a number or if 
     * it's a first digit and '0' */
    switch (c) {
        /* numbers may not start with 0 */
        case ('0'): if (isFirstDigit) return 0;    
        case ('1'):
        case ('2'):
        case ('3'):
        case ('4'):
        case ('5'):
        case ('6'):
        case ('7'):
        case ('8'):
        case ('9'): return 1;
        default:
            return 0;
    }
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    // The "fgpid(jobs)" returns pid of fg process
    while(fgpid(jobs) == pid)
        ;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    int olderrno = errno;
    switch (sig) {
        case 2:
            for (int i = 0; i < MAXJOBS; ++i)
            {
                sigset_t mask, originalSet;
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
                sigaddset(&mask, SIGTSTP);
                int thePID = jobs[i].pid;
                int theJID = jobs[i].jid;
                int status; // what the book had?
                if (thePID && waitpid(thePID, &status, WNOHANG)) {
                    /* block sigs that might affect jobs[] datastructure */
                    
                    if (sigprocmask(SIG_BLOCK, &mask, &originalSet))
                        unix_error("System call error: sigprocmask");

                    deletejob(jobs, thePID);

                    int snpRetVal = snprintf(sbuf, sizeof(sbuf),
                        "Job [%d] (%d) terminated by signal 2\n", theJID, thePID);
                    write(1, sbuf, snpRetVal);


                    
                    
                }
                /* unblock */
                if (sigprocmask(SIG_SETMASK, &originalSet, NULL))
                        unix_error("System call error: sigprocmask");
            }
            break;
        case 15: //sigterm

            break;
        default:
    }
    
    
    
    errno = olderrno;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    int olderrno = errno;

    // for ()
    // holds pid value that can't be changed by a signal
    // between checking its value and executing kill
    int fgpidAtSomePointInPast;
    // do not kill if fgpid() ret val is 0
    if ((fgpidAtSomePointInPast = fgpid(jobs)))
        if (kill(-fgpidAtSomePointInPast, SIGKILL))
            unix_error("System call error: kill");

    errno = olderrno;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    int olderrno = errno;

    //block sigint
    /*
     * Block SIGCHLD, SIGINT because they can cause the job to change
     * to a new state before this finishes modifying state in the
     * struct in jobs[], resetting state with an outdated one
     */
    sigset_t mask, originalSet;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    if (sigprocmask(SIG_BLOCK, &mask, &originalSet))
        unix_error("System call error: sigprocmask");

    int theFGPID;
    /* send sigtstp it there's a process in fg */
    if ((theFGPID = fgpid(jobs)) != 0) {
        if (kill(-theFGPID, SIGTSTP))
            unix_error("System call error: kill");
        getjobpid(jobs, theFGPID)->state = ST;
    }

    if (sigprocmask(SIG_SETMASK, &originalSet, NULL))
        unix_error("System call error: sigprocmask");
    errno = olderrno;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



