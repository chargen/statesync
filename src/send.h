#ifndef SEND_H
#define SEND_H

/**
 * This function performs the sending of file to the remote host.
 * It will first load a list of changed files then it forks the program.
 * The parent will execute the ssh command and attempts to start the
 * "statesync receive" command on the remote host.
 * The child program will start to pipe the data to the parent ssh program,
 * which inturn forwards it to the remote host.
 * 
 * @hostname: The hostname to connect to
 * TODO: we need to extend this in order to pass the remote path as well,
 * we probably should support the standard <user>@<hostname>:<path> syntax.
 */
void perform_send(char *hostname);

#endif
