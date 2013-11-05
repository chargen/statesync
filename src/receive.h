//TODO: Rename this module to remote.h
//      and use functions receive-pack and upload-pack to make this more
//      like git.

/**
 * This function performs the receiving of files. It is invoked by the ssh
 * command that the remote host executes.
 * The function first reads a list of file entries from the standard input
 * and afterwards receives and stores the file information for the 
 */
void perform_receive();
