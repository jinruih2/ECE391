#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "FileSystem.h"
#include "rtc.h"
#include "keyboard.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/* test_divide_by_0
* Description: This function is used to check exception.
* Input: None
* Output: None
* Return value: return PASS for success, return FAIL for failure
* Side effect: Exception will be displayed on the screen
*/
int test_divide_by_0(){
	TEST_HEADER;
	int result = FAIL;
	int i = 0;
	int j = 5;
	printf("result %d",j/i);
	return result;
}

/* page_fault_test
* Description: This function is used to check page fault.
* Input: None
* Output: None
* Return value: return PASS for success, return FAIL for failure
* Side effect: Page Fault will be displayed on the screen
*/
int page_fault_test(){
	TEST_HEADER;
	int result = FAIL;
	int *i = 0;
	printf("result %x",*i);
	return result;
}

/* page_deref_success_test
* Description: This function is used to check a valid page.
* Input: None
* Output: None
* Return value: return PASS for success, return FAIL for failure
* Side effect: the address will be printed out
*/
int page_deref_success_test(){
	TEST_HEADER;
	int result = PASS;

	int i = 400;
	int *j = &i;
	printf("result %x",j);
	if(*j == NULL){
		result = FAIL;
	}

	if(*j != i){
		result = FAIL;
	}
	return result;
}


/* Checkpoint 2 tests */
uint8_t test_buf[30000];

/* filesystem_file_read_test
* Description: This function is used to read an open file.
* Input: None
* Output: bytes read, the contents of the read bytes
* Return value: None
* Side effect: data will be displayed on the screen
*/
int filesystem_file_read_test(){
	clear_helper();
	TEST_HEADER;
	int result = PASS;
	int i;
	int32_t testresult1,testresult2;
	//testresult1 = file_open((uint8_t*)"frame0.txt");          				//good test
	//testresult1 = file_open((uint8_t*)"verylargetextwithverylongname.tx");    //good test
	testresult1 = file_open((uint8_t*)"verylargetextwithverylongname.txt");     //bad test
	if(testresult1<0){
		result = FAIL;
		return result;
	}
	int32_t fd;
	testresult2=file_read(fd, test_buf, 200);
	if(testresult2<0){
		result = FAIL;
		return result;
	}
	printf("%d",testresult2);
	for(i = 0; i < 200; i++){
		putc_modified(test_buf[i]);
	}
	return result;
}

/* filesystem_directory_open_test
* Description: This function is used to list out all the directories in the file system.
* Input: None
* Output: directory name, file type, file size
* Return value: None
* Side effect: print the direcotry filename
*/
void filesystem_directory_open_test(){
    clear_helper();
	int32_t result = directory_open((uint8_t*)".");       //good test
	//int32_t result = directory_open((uint8_t*)"cat");       //bad test
	if(result<0){
		printf("error");
		return;
	}
    printf("directory name: %s\n", curdirectory.filename);
}

/* ls_all_directories
* Description: This function is used to list out all the directories in the file system.
* Input: None
* Output: file name, file type, file size
* Return value: None
* Side effect: print all the information
*/
void ls_all_directories(){
	clear_helper();
	int i,fd;
	cur_directory_read_idx =0;
	for(i=0;i<bootblock_ptr->num_dir_entries;i++){
		//directory_read(fd, test_buf, 32);     //the filename is 32 bytes
		directory_read(fd, test_buf, 112);    //if passed in value more than 32 bytes, we still only copy 32 bytes
		uint32_t cur_dir_inode = curdirectory.inode_num;
		inode_t* inode_ptr = (inode_t*)((void*)bootblock_ptr + BLOCK_SIZE) + cur_dir_inode;
		printf("filename: %s, filetype: %d, size: %d\n",curdirectory.filename,curdirectory.filetype,inode_ptr->length);
	}
}

/* file_read_wholefile_test
* Description: This function is used to test for printing out the file content on the screen.
* Input: None
* Output: file data
* Return value: None
* Side effect: if the file is valid, the data will be printed; if not, print -1
*/
void file_read_wholefile_test(){
	int fd;
	const int byte_chunk = 1024;
	clear_helper();
	// printf("test file funtions\n");
	//fd = file_open((uint8_t*)"cat");                                //good test, print fish
	fd = file_open((uint8_t*)"verylargetextwithverylongname.tx");	   //bad test, print -1
	int bytes_read;
	do {
		bytes_read = file_read(fd,test_buf, byte_chunk);
		terminal_write(fd, test_buf, bytes_read);
		// printf("%d",bytes_read);
	}while(bytes_read == byte_chunk);
}

/* putc_test
* Description: This function is used to test the putc_modified function
* Input: None
* Output: characters
* Return value: None
* Side effect: characters will be displayed on the screen
*/
void putc_test(){
	clear_helper();
	int i;
	for(i=0;i<2000;i++){      //25*80 = 2000, fill the whole console
		putc_modified('a');
		}
	//putc_modified('\n');
	putc_modified('b');
	putc_modified('b');
	putc_modified('\n');
}

/* rtc_open_null_file_test
* Description: This function is used to test for opening a null named file.
* Input: None
* Output: print the content for passing the test
* Return value: None
* Side effect: teseting fail condition for rtc_open
*/
void rtc_open_null_file_test() {
	TEST_HEADER;
	const uint8_t * filename = NULL; // set named file to be null
	if (rtc_open(filename) == -1) { // rtc_open should return -1 when the named file needed to open is null
		printf("Pass test for NULL filename in rtc_open\n");
	}
}

/* rtc_write_nbytes_not4_test
* Description: This function is used to test the condition when the number of bytes written is not 4.
* Input: None
* Output: print the content for passing the test
* Return value: None
* Side effect: teseting one of fail conditions for rtc_write
*/
void rtc_write_nbytes_not4_test() {
	TEST_HEADER;
	int32_t fd = 0;
	int32_t buf = 2; // 2 Hz is a power of two so is a valid interrupt rate 
	int32_t nbytes = 5; // set the number of bytes written to be 5
	if (rtc_write(fd, &buf, nbytes) == -1) { // rtc_write should return -1 when the number of bytes written is not 4
		printf("Pass test for not-4 nbytes in rtc_write\n");
	}
}

/* rtc_write_buf_null_test
* Description: This function is used to test the condition when the buffer stored new interrupt rate value is null.
* Input: None
* Output: print the content for passing the test
* Return value: None
* Side effect: teseting one of fail conditions for rtc_write
*/
void rtc_write_buf_null_test() {
	TEST_HEADER;
	int32_t fd = 0;
	const void * buf = NULL; // set the buffer to be null
	int32_t nbytes = 4; // the number of bytes written should be 4
	if (rtc_write(fd, buf, nbytes) == -1) { // rtc_write should return -1 when the buffer is null
		printf("Pass test for NULL buffer in rtc_write\n");
	}
}

/* rtc_write_freq_not_power2_test
* Description: This function is used to test the condition when the new interrupt rate value is not a power of two.
* Input: None
* Output: print the content for passing the test
* Return value: None
* Side effect: teseting one of fail conditions for rtc_write
*/
void rtc_write_freq_not_power2_test() {
	TEST_HEADER;
	int32_t fd = 0;
	int32_t buf = 23; // 23 Hz is not a power of two so is not a valid interrupt rate 
	int32_t nbytes = 4; // the number of bytes written should be 4
	if (rtc_write(fd, &buf, nbytes) == -1) { // rtc_write should return -1 when the new interrupt rate value is not a power of two
		printf("Pass test for setting frequency which is not a power of two in rtc_write\n");
	}
}

/* rtc_change_freq_test
* Description: This function is used to virtualize the frequency chage by rtc.
* Input: None
* Output: print ones with higher and higher frequency
* Return value: None
* Side effect: teseting general changing frequency in rtc
*/
void rtc_change_freq_test() {
    TEST_HEADER;
    int i;
	const uint8_t * filename = NULL;
	int32_t fd = 0;
    int32_t freq = 2; // 2 Hz is a power of two so is a valid interrupt rate 
    int32_t nbytes = 4;	// the number of bytes written should be 4
    rtc_open(filename);
	int32_t length = 10;
    while (freq <= 1024) {
        rtc_write(fd, &freq, nbytes); // write frequency to device rtc
		clear_helper();
		for(i = 0; i < length; i++) { // print (length) number of 1s
            rtc_read(fd, NULL, 0);
			putc_modified('1'); 
        }
        freq = freq * 2; // higher frequency power of two
		length = length + 20; // longer length with higher frequency
    }
}

/* terminal_read_test
* Description: This function is used to test the function of terminal_read.
* Input: None
* Output: None
* Return value: Return value by terminal_read
* Side effect: None
*/
int terminal_read_test(){
	uint32_t n = 64;
	char buf[n];
	memset(buf,0,n);
	int nbytes = 0;
	// terminal_open(&nbytes);
	nbytes = terminal_read(0,&buf,n);
	return 1;
}

/* fish_0
* Description: This function is used to print out the frame0 of a fish.
* Input: None
* Output: None
* Return value: None
* Side effect: first clean the whole screen
*/
void fish_0(){
	int fd;
	const int byte_chunk = 1024;
	clear_helper();
	// printf("test file funtions\n");
	fd = file_open((uint8_t*)"frame0.txt");                                //good test, print fish
	//fd = file_open((uint8_t*)"verylargetextwithverylongname.txt");	   //bad test, print -1
	int bytes_read;
	do {
		bytes_read = file_read(fd,test_buf, byte_chunk);
		terminal_write(fd, test_buf, bytes_read);
		// printf("%d",bytes_read);
	}while(bytes_read == byte_chunk);
}

/* fish_1
* Description: This function is used to print out the frame1 of a fish.
* Input: None
* Output: None
* Return value: None
* Side effect: first clean the whole screen
*/
void fish_1(){
	int fd;
	const int byte_chunk = 1024;
	clear_helper();
	// printf("test file funtions\n");
	fd = file_open((uint8_t*)"frame1.txt");                                //good test, print fish
	//fd = file_open((uint8_t*)"verylargetextwithverylongname.txt");	   //bad test, print -1
	int bytes_read;
	do {
		bytes_read = file_read(fd,test_buf, byte_chunk);
		terminal_write(fd, test_buf, bytes_read);
		// printf("%d",bytes_read);
	}while(bytes_read == byte_chunk);
}

/* fish_gif
* Description: This function is used to show gif by changing between frame0 and frame1 of the fish.
* Input: None
* Output: None
* Return value: None
* Side effect: using rtc_open to open one non-null file
*/
void fish_gif(){
	uint8_t file = ' ';
	rtc_open(&file);
	while(1){
		fish_0();
		rtc_read(0, &file, 0);
		fish_1();
		rtc_read(0, &file, 0);
	}
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/* checkpoint 1 */
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("test_divide_by_0", test_divide_by_0());
	// TEST_OUTPUT("page_fault_test", page_fault_test());
	// TEST_OUTPUT("page_deref_success_test", page_deref_success_test());


	/* checkpoint 2 */
	//TEST_OUTPUT("filesystem_file_read_test",filesystem_file_read_test());
	//ls_all_directories();
	//filesystem_directory_open_test();
	file_read_wholefile_test();

	//fish_gif();

	//rtc_open_null_file_test();
	//rtc_write_nbytes_not4_test();
	//rtc_write_buf_null_test();
	//rtc_write_freq_not_power2_test();
	//rtc_change_freq_test();

	//terminal_read_test();
}	
