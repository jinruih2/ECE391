#include "systemcall.h"
#include "lib.h"
#include "FileSystem.h"

file_operation_table_t null_operation = {0, 0, 0, 0};
file_operation_table_t file_operation = {file_read, file_write, file_open, file_close};
file_operation_table_t rtc_operation = {rtc_read, rtc_write, rtc_open, rtc_close};
file_operation_table_t terminal_operation = {terminal_read, terminal_write, terminal_open, terminal_close};
file_operation_table_t directory_operation = {directory_read, directory_write, directory_open, directory_close};
int8_t pid_count[6] = {0, 0, 0, 0, 0, 0};      // used to indicate the current running process, max 6, 1 means running

/* halt
* Description: This function is used to halt and terminate the process.
* Input: status -- status for the process
* Output: None
* Return value: -1 -- halt fails
*               specific value to parent process -- halt successes
* Side effect: system call to terminate certain program
*/
int32_t halt (uint8_t status){
    cli();
    // get pcb before halt
    pcb_t* pcb = get_pcb(terminal[curr_terminal_running].curr_pid);
    // restore its parent's ebp and esp
    uint32_t ebp_restore = pcb->saved_ebp;
    uint32_t esp_restore = pcb->saved_esp;
    // get parent pid
    int8_t parent_pid = pcb->parent_pid;
    // get halt pid 
    int8_t halting_pid = terminal[curr_terminal_running].curr_pid;
    uint8_t shell[] = "shell";
    // clear fd
    int32_t fd;
    for (fd = 2; fd < 8; fd++) {
        close(fd);
    } 
    // update the pid_count array
    del_pid(halting_pid); 
    // check if it is the last process in current running terminal, if it is, re-launch shell
    if(parent_pid == -1){
        terminal[curr_terminal_running].curr_pid = -1;
        terminal[curr_terminal_running].active = 0;
        execute(shell);
    }
    // remap the parent process paging
    map_program(parent_pid);
    // update necessary tss properties
    tss.esp0 = pcb -> saved_esp;
    terminal[curr_terminal_running].curr_pid = parent_pid;
    sti();
    asm volatile ("                 \n\
            movl    %0, %%eax      \n\
            movl    %1, %%ebp      \n\
            movl    %2, %%esp      \n\
            jmp exe_ret \n\
            "
            : 
            : "r"((uint32_t)status),"r"(ebp_restore), "r"(esp_restore)
            : "eax"
    );
    return 0;
}

/* execute
* Description: This function is used to load and execute new program, handing off the processor
*              to the new program until it terminates.
* Input: command -- a space-separated sequence of words needed to be execute
* Output: None
* Return value: -1 -- command could not be executed
*               0 to 255 -- program executes a halt syscall
*               256 -- program dies for exception
* Side effect: system call to execute new program
*/
int32_t execute (const uint8_t* command){
    cli();
    int32_t ret;
    if(command == NULL){          
        return -1;
    }
    //crete a buffer to store file name
    uint8_t fname[10]; 
    int8_t arg[MAX_ARGUMENT_SIZE];
    ret = parse_cmd(command, fname, arg);
    if(ret < 0){
        return -1;
    }

    // Executable check
    uint32_t entry;
    dentry_t dentry;
    if(read_dentry_by_name(fname, &dentry) < 0){
        return -1;                                      
    }
    //check the magic number (0x7f;0x45;0x4c;0x46) specified in Appendix C
    //create a buffer of size 4 to check the first 4 bytes of the file
    uint8_t buf[4];
    // fails to read bytes at the start of file 
    if(read_data(dentry.inode_num, 0, buf, VALID_NBYTES) != SUCCESS_RETURN_NBYTES){
        return -1;
    }
    // magic number is not present
    if(buf[0] != 0x7f || buf[1] != 0x45 || buf[2] != 0x4c || buf[3] != 0x46){   
        return -1;                                      
    }
    // fals to get the information of entry point into the program needed for executing program
    if(read_data(dentry.inode_num, START_BYTE_ENTRY_POINT, buf, VALID_NBYTES) != SUCCESS_RETURN_NBYTES){
        return -1;
    }
    entry = *((uint32_t*)buf);
    int8_t pid = get_pid();
    if(pid < 0){
        return -1;
    }
// Set up program paging
    map_program(pid);
    flush_tlb();
// User-level Program Loader
    uint32_t filelen_ = get_filelen(dentry.inode_num);
    read_data(dentry.inode_num, 0, (uint8_t*)0x8048000, filelen_);
// Create PCB
    pcb_t pcb;
    init_pcb(&pcb, pid);
    pcb_t* pcb_ptr = get_pcb(pid);
    strcpy(pcb.args, arg);
    if(terminal[display_terminal].active == 0){
        curr_terminal_running = display_terminal;
        pcb.parent_pid = -1;
        terminal[display_terminal].active = 1;
    }else{
        pcb.parent_pid = terminal[display_terminal].curr_pid;
    }
    terminal[display_terminal].curr_pid = pcb.pid;
    pcb.active = 1;
    asm volatile("         \n\
        movl %%ebp, %0     \n\
        movl %%esp, %1     \n\
        "
        : "=r"(pcb.saved_ebp), "=r"(pcb.saved_esp));
    memcpy(pcb_ptr, &pcb, sizeof(pcb_t));
// Context Switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = NUM_8MB - NUM_8KB * pid - 4;  //8MB- 8KB * number of processes
    sti();
    context_switch(entry);
    asm volatile ("exe_ret:");
    return 0;
}

/* read
* Description: This function is used to read the file content stored in the buffer.
* Input: fd -- a file descriptor
*        buf -- a buffer provided with the value of new interrupt rate
*        nbytes -- the number of bytes written
* Output: None
* Return value: -1 -- function fails
*               return value call by read function -- function successes
* Side effect: None
*/
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    if (fd < FILE_MIN_NUM) {  // fd could not be less than 0
        return -1;
    } else if (fd > FILE_MAX_NUM) { // fd could not be larger than 7
        return -1;
    } else if (fd == STD_OUT_NUM) { // std_out is write only
        return -1;
    } else if (buf == NULL) { // buffer could not be empty
        return -1;
    } 
    else if (nbytes < 0) { // number of bytes written should not less than 0
        return -1;
    }
    pcb_t* pcb = (pcb_t*)(NUM_8MB - NUM_8KB * (terminal[curr_terminal_running].curr_pid + 1)); // get current pcb based on pid
    int32_t flag = pcb->fd_arr[fd].flags; // get the flags to find whether fd is in-use
    if (flag == 0) {
        return -1; // not in use then fails
    }
    return pcb->fd_arr[fd].file_operation_table->read(fd, buf, nbytes);
}

/* write
* Description: This function is used to write the file content stored in the buffer.
* Input: fd -- a file descriptor
*        buf -- a buffer provided with the value of new interrupt rate
*        nbytes -- the number of bytes written
* Output: None
* Return value: -1 -- function fails
*               return value call by write function -- function successes
* Side effect: None
*/
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    if (fd < FILE_MIN_NUM) { // fd could not be less than 0
        return -1;
    } else if (fd > FILE_MAX_NUM) { // fd could not be larger than 7
        return -1;
    } else if (fd == STDIN_NUM) { // std_in is read only
        return -1;
    } else if (buf == NULL) { // buffer could not be empty
        return -1;
    }  
    else if (nbytes < 0) { // number of bytes written should not less than 0
        return -1;
    }
    pcb_t* pcb = get_pcb (terminal[curr_terminal_running].curr_pid); // get current pcb based on pid
    int32_t flag = pcb->fd_arr[fd].flags; // get the flags to find whether fd is in-use
    if (flag == 0) { 
        return -1; // not in use then fails
    }
    return pcb->fd_arr[fd].file_operation_table->write(fd, buf, nbytes);
}

/* open
* Description: This function is used to open the corresponding file.
* Input: filename -- the name of the file to open
* Output: None
* Return value: -1 -- function fails
*               return value the fd number
* Side effect: Open the file and fill in the fd array
*/
int32_t open (const uint8_t* filename){
    pcb_t* PCB = get_pcb(terminal[curr_terminal_running].curr_pid);                      //Get the current pcb structure
    int32_t index = 2;                                  //2 because we don't want stdin and stdout
    dentry_t dentry;                                    //dentry structure
    if (filename == NULL || read_dentry_by_name(filename, &dentry) == -1) {     //check if the file exists and the file name is valid 
        return -1;
    }
    while (index < 8 && PCB->fd_arr[index].flags == 1) {            //8 is the max file number supported, flag represents whether the file is in use
        index++;                                                    //find the next unused file
    }
    if (index == 8) {                                               //if reaches the max number do noting and return -1
        return -1;
    }
    if (dentry.filetype == 0) {                                    //0 is the file type number for rtc
        if (rtc_open(filename) == -1) {                            // if cannot rtc open, do noting and return -1
            return -1;
        }
       
        PCB->fd_arr[index].file_operation_table = &rtc_operation;       //fill in the file op table with rtc op
        PCB->fd_arr[index].inode = 0;                                   //initialize inode
        PCB->fd_arr[index].file_pos = 0;                                //initialize file pos
        PCB->fd_arr[index].flags = 1;                                   //set flag to in use
    } 
    
    else if (dentry.filetype == 1) {                                    //1 is the file type number of directory 
        if (directory_open(filename) == -1) {                           //if cannot directory open, do nothing and return -1
            return -1;
        }
       
        PCB->fd_arr[index].file_operation_table = &directory_operation; //fill in the file op table with directory op
        PCB->fd_arr[index].inode = 0;                                    //initialize inode
        PCB->fd_arr[index].file_pos = 0;                                //initialize file pos
        PCB->fd_arr[index].flags = 1;                                    //set flag to in use
    }
   
    else if (dentry.filetype == 2) {                                    //2 is the file type number of regular file
        if (file_open(filename) == -1) {                                // if cannot regular file open, do noting and return -1
            return -1;
        }
        PCB->fd_arr[index].file_operation_table = &file_operation;      //fill in the file op table with file op
        PCB->fd_arr[index].inode = dentry.inode_num;                     //initialize inode
        PCB->fd_arr[index].file_pos = 0;                                 //initialize file pos
        PCB->fd_arr[index].flags = 1;                                   //set flag to in use
    }
    return index;                                                       //retunr fd number 
}

/* close
* Description: This function is used to close the corresponding file.
* Input:fd -- the fd number for the file to close 
* Output: None
* Return value: -1 -- function fails
*               return 0 if the function successes
* Side effect:close the file and set the file to not in use
*/
int32_t close (int32_t fd){
    if(fd <= 1 || fd > FILE_MAX_NUM){                                   // 1 because we don't want it to be stdin and stdout, max num to check it is out of bound
        return -1;
    }
    pcb_t *PCB = get_pcb(terminal[curr_terminal_running].curr_pid);                                     //get the currently used pcb
    if(PCB->fd_arr[fd].flags == 0){                                     //check if the file is  in use
        return -1;
    }
    PCB->fd_arr[fd].flags = 0;                                          //set the flag to be not in use
    return PCB->fd_arr[fd].file_operation_table->close(fd);             //call corresponding devices' close and return 
}

/* getargs
* Description: This function is used to read the program's commandl line arguments into a user-level buffer
* Input:buf -- the user-level buffer
        nbytes -- buffer size
* Output: None
* Return value: -1 -- function fails
*               return 0 if the function successes
* Side effect:None
*/
int32_t getargs(uint8_t* buf, int32_t nbytes){
    if(!buf || nbytes < 0) return -1;
    pcb_t* pcb = get_pcb(terminal[curr_terminal_running].curr_pid);
    //if there are no arguments, or if the arguments and a terminal NULL, or do not fit in the buffer, return -1
    if(*(pcb -> args) == NULL || *(pcb -> args) == '\0' || strlen(pcb -> args) > nbytes) return -1;
    //copy to buf
    strcpy((int8_t*)buf,pcb->args);
    return 0;
}

/* vidmap
* Description: This function is used to map the text-mode video memory into user space at a pre-set virtual address
* Input:screen_start -- the user-level buffer
* Output: None
* Return value: -1 -- function fails
*               return 0 if the function successes
* Side effect:None
*/
int32_t vidmap(uint8_t** screen_start){
    //sanity check
    if(!screen_start || (int32_t)screen_start<NUM_128MB || (int32_t)screen_start>=NUM_132MB) return -1;
    vidmap_paging();
    *screen_start = (uint8_t*) NUM_132MB;
    return 0;
}

/* set_handler (int32_t signum, void* handler_address)
* Description: This function is the system call set_handler         
* Input: signum, handler_address
* Output: None
* Return value: -1
* Side effect: None
*/
int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}

/* sigreturn(void)
* Description: This function is the system call sigreturn        
* Input: None
* Output: None
* Return value: -1
* Side effect: None
*/
int32_t sigreturn(void){
    return -1;
}



/* get_pid
* Description: This function is a helper function which is used to get current pid.         
* Input: None
* Output: None
* Return value: -1 -- number of pid uses reach maximum
*               current pid -- successes
* Side effect: None
*/
int8_t get_pid (){
    int32_t i;
    for(i = 0; i < MAX_PID_NUM; i++){
        if(pid_count[i] == 0){  // indicator indicates pid not in-use
            pid_count[i] = 1;   // set indicator to 1 for in-use 
            return i;           // return value for current pid to use
        }
    }
    return -1;                  // could not get pid to use then fails
}

/* init_pcb
* Description: This function is a helper function which is used to initialize pcb 
*              when creating pcb.
* Input: pcb -- current pcb
*        pid -- current pid 
* Output: None
* Return value: None
* Side effect: Help for creating new pcb
*/
void init_pcb(pcb_t* pcb, uint32_t pid){
    uint8_t i;
    pcb -> pid = pid; // set current pid
    pcb -> parent_pid = -1;
    // initialize all of fd_t struct in pcb
    for(i = FILE_MIN_NUM; i <= FILE_MAX_NUM; i++){
        pcb -> fd_arr[i].file_operation_table = &null_operation;
        pcb -> fd_arr[i].inode = 0;
        pcb -> fd_arr[i].file_pos = 0;
        pcb -> fd_arr[i].flags = 0;
    }
    // stdin
    pcb -> fd_arr[0].file_operation_table = &terminal_operation;
    pcb -> fd_arr[0].flags = 1;
    // stdout
    pcb -> fd_arr[1].file_operation_table = &terminal_operation;
    pcb -> fd_arr[1].flags = 1;
    // initialize rest of things in pcb
    pcb -> saved_ebp = 0;
    pcb -> saved_esp = 0;
    pcb -> return_ebp = 0;
    pcb -> return_esp = 0;
    pcb -> active = 0;
    for(i=0;i<MAX_ARGUMENT_SIZE;i++){
        pcb->args[i] = '\0';
    }
}

/* get_pcb
* Description: This function is a helper function which is used to get current pcb based on given pid.         
* Input: pid -- current pid
* Output: None
* Return value: current pcb value
* Side effect: provide current pcb for other functions
*/
pcb_t* get_pcb (uint8_t pid){
    // each pcb starts at the top of a 8KB block inside the kernel
    // shell be loaded at physical 8MB
    pcb_t* pcb = (pcb_t*)(NUM_8MB - NUM_8KB * (pid + 1));
    return pcb;
}


/* parse_command
* Description: This function is a helper function which is used for parse arguments in execute function.         
* Input: command -- a space-separated sequence of words needed to be execute
*        fname -- name of a file
*        arg -- arguments
* Output: None
* Return value: -1 -- fails
*               1 -- successes
* Side effect: None
*/
int32_t parse_cmd (const uint8_t* command, uint8_t* fname, int8_t* arg){
    int i;
    // set max length of file name
    for(i=0;i<MAX_FNAME_NUM;i++){
        fname[i] ='\0';
    }
    // set max length of arguments
    for(i=0;i<MAX_ARGUMENT_SIZE;i++){
        arg[i] = '\0';
    }
    int idx = 0;
    // check whether command is null
    if(command == NULL){
        return -1;
    }
    // increase index when command has blank
    while(command[idx] == ' '){
        idx++;
    }
    // for command whether reaches the end
    for(i = idx; i < MAX_COMMAND_NUM; i++){
        if(command[i] == '\0' || command[i] == ' '){
            fname[i - idx] = '\0';
            idx = i + 1;
            break;
        }
        fname[i - idx] = command[i];
    }
    // increase index when next blank in command
    while(command[idx] == ' '){
        idx++;
    }
    for(i = idx; i < strlen((int8_t*)command); i++){
        if(command[i] == '\0' || command[i] == ' '){
            arg[i - idx] = '\0';
            break;
        }
        arg[i - idx] = command[i];
    }
    return 0;

}

/* del_pid
* Description: This function is a helper function which is used to delete process id.         
* Input: pid -- current pid
* Output: None
* Return value: current pcb value
* Side effect: delete pid
*/
int8_t del_pid(int8_t pid){
    if(pid >= MAX_PID_NUM){ // check whether pid is larger than largest pid
        return -1;
    }
    if(pid < 0){ // check whether pid is smaller than 0
        return -1;
    } else{
        pid_count[(uint8_t)pid] = 0; // clear pid
    }
    return 0;
}

/* get_curr_pcb
* Description: This function is a helper function to get the pcb pointer for the current running process.         
* Input: None
* Output: None
* Return value: pcb pointer for current running terminal
* Side effect: None
*/
pcb_t* get_curr_pcb (){
    return get_pcb(terminal[curr_terminal_running].curr_pid);
}
