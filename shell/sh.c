#define _POSIX_SOURCE

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "jobs.h"

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

extern int errno;
pid_t fg_pid;
job_list_t *my_jobs;
int next_id;
int status;

/* handler for SIGCONT, which is sent by child processes the to parent when there is
 * a change in their process state
 */
void child_handler(int signum) {
  pid_t child_pid;
  if (signum == SIGCHLD) {
    while((child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
      if (WIFEXITED(status)) {
	remove_job_pid(my_jobs, child_pid);
	if (fg_pid == child_pid)
	  fg_pid = 0;
      } else if (WIFSIGNALED(status)) {
	int signal = WTERMSIG(status);
	int jid = get_job_jid(my_jobs, child_pid);
	char term_msg[100];
	char *template = "\nsh: Job [%d] (%d) terminated by signal %d\n";
	sprintf(term_msg, template, jid, child_pid, signal);
	write(STDOUT_FILENO, term_msg, strlen(term_msg));
	remove_job_pid(my_jobs, child_pid);
	if (fg_pid == child_pid) {
	  fg_pid = 0;
	}
      } else if (WIFSTOPPED(status)) {
	update_job_pid(my_jobs, child_pid, _STATE_STOPPED);
	if (fg_pid == child_pid) {
	  fg_pid = 0;
	}
      } else if (WIFCONTINUED(status)) {
	update_job_pid(my_jobs, child_pid, _STATE_RUNNING);
      }
    }
  }
}

/* handler for SIGINT, SIGTSTP, and SIGQUIT */
void handler(int signum) {
  if (fg_pid > 0) {
    kill(-fg_pid, signum);
  }
}

/* generic function that installs the handler specified for the given signal number by
 * initializing a sigaction struct and setting the appropriate mask before calling
 * sigaction()
 */
int install_handler(int signum, void (*handler)(int)) {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, signum);
  
  struct sigaction act;
  act.sa_handler = handler;
  act.sa_mask = set;
  act.sa_flags = SA_RESTART;
  
  int n = sigaction(signum, &act, NULL);
  return n;
}


/* parse_redirect searches the user input pointed to by ptr for the redirection characters and
 * stores the redirection pathname to file, which is a pointer to a string initialized in the main
 * method so that it can be accessed after it leaves the scope. It does error checking for
 * multiple redirection and usage, and returns -1 on failure, 0 on success.
 *
 * ptr - a pointer to the current char pointer in main
 * dir_string - a cstring indicating the direction (i.e. input or output);
 * is_multiple - a boolean (int) indicating whether or not there has already been a file redirection
 *     in the specified direction;
 * file - a pointer to a string initialized in the main that will be malloced in this function
 *     indicating the path of the redirection file
 * trunc - a boolean indicating whether to truncate or append the output
 */
int parse_redirect (char **ptr, char *dir_str, int *is_multiple, char **file, int *trunc) {
  char err_msg[50];

  if (*is_multiple) {
    sprintf(err_msg, "sh: Multiple %s redirects\n", dir_str);
    write(STDERR_FILENO, err_msg, strlen(err_msg));
    return -1;
  } else {
    if (**ptr == '>' && *(*ptr + 1) == '>') { // check if >> rather than >
      *trunc = 0;
      *ptr += 2;
    } else (*ptr)++;

    *is_multiple = 1;
    *ptr += strspn(*ptr, " \t");
    
    if (**ptr == '<' || **ptr == '>' || **ptr == '\0') { // no file after redirection symbol
      strcpy(err_msg, "sh: No redirection file specified\n");
      write(STDERR_FILENO, err_msg, strlen(err_msg));
      return -1;
    } else {
      size_t path_len = strcspn(*ptr, " \t<>");
      *file = (char *) malloc(path_len + 1);
      strncpy(*file, *ptr, path_len);
      *(*file + path_len) = '\0';
      *ptr += path_len; // update the char pointer to the position in buf after the filename
    }
  }
  return 0;
}

/* exec_cd executes the built-in command cd using chdir()
 *
 * argc - number of arguments
 * args - string of all the arguments
 * wd - a string representing the current working directory
 */
int exec_cd(int argc, char **args, char *wd) {
  if (argc != 1) {
    write(STDERR_FILENO, "cd: Usage: cd <dir>\n", 20);
    return -1;
  } else {
    size_t arg_len = strcspn(*args, " \t");
    char arg[arg_len + 1];
    arg[0] = '\0';
    strncat(arg, *args, arg_len);
    if(chdir(arg) < 0) {
      char err_msg[arg_len + 5];
      sprintf(err_msg, "cd: %s", arg);
      perror(err_msg);
      return -1;
    }
    getcwd(wd, BUF_SIZE);
  }
  return 0;
}

/* exec_ln executes the built-in ln command using link(). It parses the args string by
 * separating the arguments on whitespace
 *
 * argc - argument count
 * args - string of the arguments from user input
 */
int exec_ln(int argc, char **args) {
  if (argc != 2) {
    write(STDERR_FILENO, "ln: Usage: ln <src> <dest>\n", 28);
    return -1;
  } else {
    size_t arg_len = strcspn(*args, " \t");
    char arg1[arg_len + 1];
    arg1[0] = '\0';
    strncat(arg1, *args, arg_len);

    char *argptr = *args;
    argptr += arg_len;
    argptr += strspn(argptr, " \t");
    arg_len = strcspn(argptr, " \t");
    char arg2[arg_len + 1];
    arg2[0] = '\0';
    strncat(arg2, argptr, arg_len);

    int link_error = link(arg1, arg2);
    if (errno == ENOENT) {
      char err_msg[strlen(arg1) + 5];
      sprintf(err_msg, "ln: %s", arg1);
      perror(err_msg);
      return -1;
    } else if (errno == EEXIST) {
      char err_msg[strlen(arg2) + 5];
      sprintf(err_msg, "ln: %s", arg2);
      perror(err_msg);
      return -1;
    } else if (link_error < 0) {
      perror("ln");
      return -1;
    }
  }
  return 0;
}

/* exec_rm executes the built-in rm command using unlink().
 *
 * argc - argument count
 * args - string of the arguments from user input
 */
int exec_rm(int argc, char **args) {
  if (argc != 1) {
    write(STDERR_FILENO, "rm: Usage: rm <file>\n", 21);
    return -1;
  } else {
    size_t arg_len = strcspn(*args, " \t");
    char arg[arg_len + 1];
    arg[0] = '\0';
    strncat(arg, *args, arg_len);
    if (unlink(arg)) {
      char err_msg[arg_len + 5];
      sprintf(err_msg, "rm: %s", arg);
      perror(err_msg);
      return -1;
    }
  }
  return 0;
}


void reassign_tc(pid_t pid) {
  sigset_t sigttou, oldset;
  sigemptyset(&sigttou);
  sigaddset(&sigttou, SIGTTOU);

  sigprocmask(SIG_BLOCK, &sigttou, &oldset);
  tcsetpgrp(STDIN_FILENO, pid);
  sigprocmask(SIG_UNBLOCK, &sigttou, &oldset);
}

/* exec_bg sends the SIGCONT signal to a job and immediately continues so that
 * the job will run in the background
 *
 * argc - arc count
 * args - string of arguments from user input
 */
 int exec_bg(int argc, char **args) {
  if (argc != 1) {
    write(STDERR_FILENO, "bg: Usage: bg <job>\n", 20);
    return -1;
  } else {
    size_t arg_len = strcspn(*args, " \t");
    char arg[arg_len + 1];
    arg[0] = '\0';
    strncat(arg, *args, arg_len);
    int pid;
    if (*arg == '%') {
      pid = get_job_pid(my_jobs, atoi(arg + 1));
      if (pid < 0) {
	write(STDERR_FILENO, "bg: Job ID not found\n", 21);
	return -1;
      }
    } else {
      pid = atoi(arg);
      if (get_job_jid(my_jobs, pid) < 0) {
	write(STDERR_FILENO, "bg: Process ID not found\n", 25);
	return -1;
      }
    }
    fg_pid = 0;
    kill(-pid, SIGCONT);
  }
  return 0;
}

/* exec_fg sends the SIGCONT signal to a job and then executes a busy loop
 * so that the job will run in the foreground
 *
 * argc - argcount
 * args - string of arguments from user input
 */
int exec_fg(int argc, char **args) {
  if (argc != 1) {
    write(STDERR_FILENO, "fg: Usage: fg <job>\n", 20);
    return -1;
  } else {
    size_t arg_len = strcspn(*args, " \t");
 char arg[arg_len + 1];
    arg[0] = '\0';
    strncat(arg, *args, arg_len);
    int pid;
    if (*arg == '%') {
      pid = get_job_pid(my_jobs, atoi(arg + 1));
      if (pid < 0) {
	write(STDERR_FILENO, "fg: Job ID not found\n", 21);
	return -1;
      }
    } else {
      pid = atoi(arg);
      if (get_job_jid(my_jobs, pid) < 0) {
	write(STDERR_FILENO, "fg: Process ID not found\n", 25);
	return -1;
	 }
    }
    fg_pid = pid;
    reassign_tc(pid);
    kill(-pid, SIGCONT);
    while (fg_pid) sleep(1);
    reassign_tc(getpgid(getpid()));
  }
  return 0;
}

int exec_builtin(int argc, char **cmd, char **args, char *wd) {
    if (strcmp(*cmd, "cd") == 0) {
      exec_cd(argc, args, wd);
    } else if (strcmp(*cmd, "ln") == 0) {
      exec_ln(argc, args);
    } else if (strcmp(*cmd, "rm") == 0) {
      exec_rm(argc, args);
    } else if (strcmp(*cmd, "bg") == 0) {
      exec_bg(argc, args);
    } else if (strcmp(*cmd, "fg") == 0) {
      exec_fg(argc, args);
    } else if (strcmp(*cmd, "jobs") == 0) {
      jobs(my_jobs);
    } else if (strcmp(*cmd, "exit") == 0) {
      return 1;
    } else {
      return -1;
    }
  return 0;
}

/* file_redirect opens the specified redirection files, uses dup2() to replace the standard file(s), and closes
 * the newly opened file. It also frees the filename char arrays malloc-ed in parse_redirect().
 *
 * input_redirect - boolean indicating whether or not there is input redirection
 * output_redirect - boolean indicating whether or not there is output redirection
 * file_in - pointer to the buffer in which the input filename is stored
 * file_out - pointer to the buffer in which the output filename is stored
 * trunc_file - boolean indicating whether the output file should be truncated
 */
int file_redirect(int input_redirect, int output_redirect, char **file_in, char **file_out, int trunc_file) {
  if (input_redirect) {
    int fd = open(*file_in, O_RDONLY);
    if (fd < 0) {
      char err_msg[strlen(*file_in) + 5];
      sprintf(err_msg, "sh: %s", *file_in);
      perror(err_msg);
      return -1;
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
    free(*file_in);
  }
  if (output_redirect) {
    int fd;
    if (trunc_file) fd = open(*file_out, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
    else fd = open(*file_out, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
    if (fd < 0) {
      char err_msg[strlen(*file_out) + 5];
      sprintf(err_msg, "sh: %s\n", *file_out);
      perror(err_msg);
      return -1;
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    free(*file_out);
  }
  return 0;
}

  

/* argc - argument count
 * program - a string representing the program name
 * args - a string representing the arguments from user input
 *
 * exec_extern executes an external program given by the program argument. It forks
 * the child process and waits for it to complete before resuming the parent process.
 */
int exec_extern(int argc, char **program, char **args, int in, int out, char **file_in, char **file_out, int trunc_file, int bg) {
  int argv_len = argc + 2;
  char *argv[argv_len];
  argv[0] = *program;

  int i;
  size_t argi_len;
  char *arg_ptr = *args;
  for (i = 1; i < argv_len - 1; i++) {
    arg_ptr += strspn(arg_ptr, " \t");
    argi_len = strcspn(arg_ptr, " \t");

    argv[i] = arg_ptr;
    arg_ptr += argi_len;
    *arg_ptr = '\0';
    arg_ptr ++;
  }
  argv[i] = NULL;

  // check if command exists before redirection to prevent clobbering
  int test_fd = open(argv[0], O_RDONLY);
  if (test_fd < 0) {
    if (errno == ENOENT) {
      char err_msg[strlen(*program) + 25];
      sprintf(err_msg, "sh: %s: Command not found\n", *program);
      write(STDERR_FILENO, err_msg, strlen(err_msg));
      return -1;
    }
  } else {
    close(test_fd);
  }

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &set, &oldset);
  pid_t n = fork();
  if (!bg) fg_pid = n;
  if (!n) {
    setpgid(0, 0);
    sigprocmask(SIG_UNBLOCK, &set, &oldset);

    if(file_redirect(in, out, file_in, file_out, trunc_file))
      exit(EXIT_FAILURE);

    execve(argv[0], argv, NULL);
    
    remove_job_jid(my_jobs, next_id--);
    perror(*program);
    exit(EXIT_FAILURE);
  }
  if (!bg) reassign_tc(fg_pid);
  add_job(my_jobs, next_id++, n, _STATE_RUNNING, *program);
  sigprocmask(SIG_UNBLOCK, &set, &oldset);
  while(fg_pid) sleep(1);
  reassign_tc(getpgid(getpid()));
  return 0;
}

/* cmd_line - a string of the command line input
 * wd - a string of the current working directory
 *
 * parse_command separates the command line into the command type and its arguments.
 * It checks if it is one of the built-in commands, and if it is, it calls the
 * corresponding command. Otherwise, it calls exec_exter, which is responsible for
 * executing external programs
 */
int parse_command(char *cmd_line, char **cmd, char **cmd_args) {
  char *curr_char = cmd_line + strspn(cmd_line, " \t");

  size_t cmd_len = strcspn(curr_char, " \t");
  *cmd = (char *) malloc(cmd_len + 1);
  strncpy(*cmd, curr_char, cmd_len);
  *(*cmd + cmd_len) = '\0';
  curr_char += cmd_len;
  curr_char += strspn(curr_char, " \t");

  size_t args_len = strlen(curr_char);
  *cmd_args = (char *) malloc(args_len + 1);
  strncpy(*cmd_args, curr_char, args_len);
  *(*cmd_args + args_len) = '\0';
  
  int arg_count = 0;
  while (*curr_char != '\0') {
    curr_char += strspn(curr_char, " \t");
    if (*curr_char == '\0') break;
    curr_char += strcspn(curr_char, " \t");
    arg_count++;
  }
  
  return arg_count;
}

/* read_line reads the command line until CTRL-D or newline
 *
 * buf - buffer from the main method in which read() stores its output
 */
int read_line(char *buf) {
  int n = read(STDIN_FILENO, buf, BUF_SIZE);
  
  if (n < 0) {
    perror("sh");
    return -1;
  } else if (n == 0) {
    #ifndef NO_PROMPT
    write(STDOUT_FILENO, "\n", 1); // remove this to past automatic test suite
    #endif
    return -1;
  } else if (n == 1 && buf[0] == '\n') {
    return 1;
  } else {
    if (buf[n - 1] == '\n') {
      buf[n - 1] = '\0';
    } else {
      buf[n] = '\0';
      write(STDOUT_FILENO, "\n", 1);
    }
  }
  return 0;
}

/* background checks if command line ends with & and updates bg boolean variable 
 */
void background(char *cmd_line, int *bg) {
  char *ampersand = strpbrk(cmd_line, "&");
  if (ampersand != NULL) {
    ampersand++;
    if (strspn(ampersand, " \t") == strlen(ampersand)) {
      *(cmd_line + strcspn(cmd_line, "&")) = '\0';
      *bg = 1;
    }
  }
}

/* terminate_children is called immediately before exiting the shell to kill any
 * child processes that may still be running in the background
 */
void terminate_children() {
  pid_t pid = 0;
  while (pid >= 0) {;
    pid = get_next_pid(my_jobs);
    kill(-pid, SIGKILL);
  }
}

int main() {
  my_jobs = init_job_list();
  next_id = 1;
  fg_pid = 0;
  install_handler(SIGINT, &handler);
  install_handler(SIGTSTP, &handler);
  install_handler(SIGQUIT, &handler);
  install_handler(SIGCHLD, &child_handler);

  char buf[BUF_SIZE], cmd_line[BUF_SIZE], wd[BUF_SIZE], *file_out, *file_in, *ptr, *cmd, *cmd_args;
  int quit, input_redirect, output_redirect, trunc_file, bg;
  getcwd(wd, BUF_SIZE);

  quit = 0;
  while (!quit) {
    bg = 0;
    input_redirect = 0;
    output_redirect = 0;
    trunc_file = 1;
    cmd_line[0] = '\0';

    #ifndef NO_PROMPT
    if (write(STDOUT_FILENO, wd, strlen(wd)) < 0) break;
    if (write(STDOUT_FILENO, " $ ", 3) < 0) break;
    #endif

    int read_error = read_line(buf);
    if(read_error < 0) break;
    else if (read_error > 0) continue;

    int parse_error = 0;
    ptr = buf;
    background(ptr, &bg);

    while (!parse_error) {
      if (*ptr != '<' && *ptr != '>') {
	size_t cmd_len = strcspn(ptr, "<>");
	strncat(cmd_line, ptr, cmd_len);
	ptr = strpbrk(ptr, "<>");
      }
      if (!ptr) break;

      if (*ptr == '<') {
	parse_error = parse_redirect(&ptr, "input", &input_redirect, &file_in, &trunc_file);
      } else if (*ptr == '>') {
	parse_error = parse_redirect(&ptr, "output", &output_redirect, &file_out, &trunc_file);
      }
    }
    if (parse_error) continue;

    int arg_count = parse_command(cmd_line, &cmd, &cmd_args);
    int is_builtin = exec_builtin(arg_count, &cmd, &cmd_args, wd);
    if (is_builtin == 1) break;
    if (is_builtin == -1) {
      exec_extern(arg_count, &cmd, &cmd_args, input_redirect, output_redirect, &file_in, &file_out, trunc_file, bg);
    }
    free(cmd);
    free(cmd_args);
  }

  terminate_children();
  cleanup_job_list(my_jobs);
  return 0;
}
