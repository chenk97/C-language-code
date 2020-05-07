#include "const.h"
#include "transplant.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

void putMagic();
void putDepth(int depth);
void putSize(off_t size);
void putMode(mode_t mode);
void putName(char *name);
off_t getSize();
mode_t getMode();
int stringCmr(char *str1, char *str2);
int nameSize(char *name);
int passMagic();
off_t passSize();
int passDepth();
mode_t passMode();
void passName(int strlen);
void popName();



/*
 * A function that returns printable names for the record types, for use in
 * generating debugging printout.
 */
static char *record_type_name(int i) {
    switch(i) {
    case START_OF_TRANSMISSION:
	return "START_OF_TRANSMISSION";
    case END_OF_TRANSMISSION:
	return "END_OF_TRANSMISSION";
    case START_OF_DIRECTORY:
	return "START_OF_DIRECTORY";
    case END_OF_DIRECTORY:
	return "END_OF_DIRECTORY";
    case DIRECTORY_ENTRY:
	return "DIRECTORY_ENTRY";
    case FILE_DATA:
	return "FILE_DATA";
    default:
	return "UNKNOWN";
    }
}

/*
 * @brief  Initialize path_buf to a specified base path.
 * @details  This function copies its null-terminated argument string into
 * path_buf, including its terminating null byte.
 * The function fails if the argument string, including the terminating
 * null byte, is longer than the size of path_buf.  The path_length variable
 * is set to the length of the string in path_buf, not including the terminating
 * null byte.
 *
 * @param  Pathname to be copied into path_buf.
 * @return 0 on success, -1 in case of error
 */
int path_init(char *name) {
    // To be implemented.
    //update path_length when return 0
    int i;
    char *buf = path_buf;
    for(i=0; *(name+i)!='\0'; i++){
        if(i > PATH_MAX){return -1;}
        *(buf+i) = *(name+i);
    }
    //add the null-terminate
    path_length = i;
    i+=1;
    *(buf+i) = '\0';
    return 0;
}

/*
 * @brief  Append an additional component to the end of the pathname in path_buf.
 * @details  This function assumes that path_buf has been initialized to a valid
 * string.  It appends to the existing string the path separator character '/',
 * followed by the string given as argument, including its terminating null byte.
 * The length of the new string, including the terminating null byte, must be
 * no more than the size of path_buf.  The variable path_length is updated to
 * remain consistent with the length of the string in path_buf.
 *
 * @param  The string to be appended to the path in path_buf.  The string must
 * not contain any occurrences of the path separator character '/'.
 * @return 0 in case of success, -1 otherwise.
 */
int path_push(char *name) {
    // To be implemented.
    int i;
    char *buf = path_buf;
    if(path_length < PATH_MAX){
        *(buf+path_length) = '/';
        path_length+=1;
        for(i=0; *(name+i)!='\0'; i++){
            if(path_length > PATH_MAX){return -1;}
            *(buf+path_length+i) = *(name+i);
        }
        path_length+=i;
        *(buf+path_length+1) = '\0';
        debug("path after push: %s", path_buf);
        return 0;
    }
    return -1;
}

/*
 * @brief  Remove the last component from the end of the pathname.
 * @details  This function assumes that path_buf contains a non-empty string.
 * It removes the suffix of this string that starts at the last occurrence
 * of the path separator character '/'.  If there is no such occurrence,
 * then the entire string is removed, leaving an empty string in path_buf.
 * The variable path_length is updated to remain consistent with the length
 * of the string in path_buf.  The function fails if path_buf is originally
 * empty, so that there is no path component to be removed.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int path_pop() {
    // To be implemented.
    char *buf = path_buf;
    while(path_length > 0){
        if(*(buf+path_length) == '/'){
            *(buf+path_length) = '\0';
            debug("path after pop: %s", path_buf);
            return 0;
        }
        *(buf+path_length) = '\0';
        path_length--;
        if(path_length == 0){
            *buf = '\0';
            debug("path after pop: %s", path_buf);
            return 0;
        }
    }
    return -1;
}

/*
 * @brief Deserialize directory contents into an existing directory.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory.  It reads (from the standard input) a sequence of DIRECTORY_ENTRY
 * records bracketed by a START_OF_DIRECTORY and END_OF_DIRECTORY record at the
 * same depth and it recreates the entries, leaving the deserialized files and
 * directories within the directory named by path_buf.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * each of the records processed.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including depth fields in the records read that do not match the
 * expected value, the records to be processed to not being with START_OF_DIRECTORY
 * or end with END_OF_DIRECTORY, or an I/O error occurs either while reading
 * the records from the standard input or in creating deserialized files and
 * directories.
 */
int deserialize_directory(int depth) {
    // To be implemented.
    int type;
    off_t dir_size;
    int nsize;
    mode_t mode;
    if(passMagic()){//check magic sequence-3bytes
        if(getchar() == START_OF_DIRECTORY){//check type-1byte
            if(passDepth() == depth){
                if(passSize() == HEADER_SIZE){
                    if(passMagic()){
                        while(getchar() != END_OF_DIRECTORY){
                                if(passDepth() == depth){
                                    dir_size = passSize();
                                    nsize = dir_size-HEADER_SIZE-12;
                                    if(nsize>0){
                                        //metadata
                                        mode = passMode();
                                        passSize();
                                        passName(nsize);//store name in name buf
                                        path_push(name_buf);//push the name for file to name_buf
                                        DIR *dir = opendir(path_buf);
                                        popName();
                                        if(S_ISDIR(mode)){
                                            if(!dir || global_options == 12){
                                                mkdir(path_buf, 0700);
                                            }//else{return -1;}
                                            deserialize_directory(depth+1);
                                        }else if(S_ISREG(mode)){
                                            FILE *f = fopen(path_buf, "r");
                                            if(f == NULL || global_options == 12){
                                                deserialize_file(depth);
                                            }else{return -1;}
                                        }else{
                                            return -1;
                                        }
                                        if(!passMagic()){
                                            return -1;
                                        }
                                        path_pop();
                                    }
                                }
                            }////end while
                            //pass END_OF_DIR
                            if(passDepth() == depth){
                                if(passSize() == HEADER_SIZE){
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    return -1;

}

/*
 * @brief Deserialize the contents of a single file.
 * @details  This function assumes that path_buf contains the name of a file
 * to be deserialized.  The file must not already exist, unless the ``clobber''
 * bit is set in the global_options variable.  It reads (from the standard input)
 * a single FILE_DATA record containing the file content and it recreates the file
 * from the content.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * the FILE_DATA record.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including a depth field in the FILE_DATA record that does not match
 * the expected value, the record read is not a FILE_DATA record, the file to
 * be created already exists, or an I/O error occurs either while reading
 * the FILE_DATA record from the standard input or while re-creating the
 * deserialized file.
 */
int deserialize_file(int depth){
    // To be implemented.
    //if(global_option == 12)
    if(passMagic()){
        if(getchar()==FILE_DATA){
            FILE *f = fopen(path_buf, "w+");
            int fsize;
            if(passDepth() == depth){
                fsize = passSize()-HEADER_SIZE;
                debug("%d", fsize);
                while(fsize != 0){
                    fputc(getchar(),f);
                    fsize--;
                }
                fclose(f);
                return 0;
            }
        }
    }

    return -1;
}

/*
 * @brief  Serialize the contents of a directory as a sequence of records written
 * to the standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory to be serialized.  It serializes the contents of that directory as a
 * sequence of records that begins with a START_OF_DIRECTORY record, ends with an
 * END_OF_DIRECTORY record, and with the intervening(sandwiched) records all of
 * type DIRECTORY_ENTRY.
 *
 * @param depth  The value of the depth field that is expected to occur in the
 * START_OF_DIRECTORY, DIRECTORY_ENTRY, and END_OF_DIRECTORY records processed.
 * Note that this depth pertains only to the "top-level" records in the sequence:
 * DIRECTORY_ENTRY records may be recursively followed by similar sequence of
 * records describing sub-directories at a greater depth.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open files, failure to traverse directories, and I/O errors
 * that occur while reading file content and writing to standard output.
 */
//use putchar(c) to write to stdout
int serialize_directory(int depth) {
    // To be implemented.
    DIR *dir = opendir(path_buf);
    if(dir){
        struct dirent *de;
        putMagic();
        putchar(START_OF_DIRECTORY);
        putDepth(depth);
        putSize(HEADER_SIZE);
        while((de = readdir(dir))!= NULL){//read all file and dir in current dir
            if(stringCmr(de->d_name, "..") == 0 && stringCmr(de->d_name, ".") == 0){
                debug("%s", path_buf);
                debug("%s", de->d_name);
                putMagic();
                putchar(DIRECTORY_ENTRY);
                putDepth(depth);
                //HEADER(16)+META+(12)+de->d_name
                putSize(HEADER_SIZE+12+nameSize(de->d_name));
                //meta starts here
                path_push(de->d_name);
                putMode(getMode());
                putSize(getSize());
                putName(de->d_name);
                if(S_ISDIR(getMode())){
                    serialize_directory(depth+1);
                }else if(S_ISREG(getMode())){
                    serialize_file(depth,getSize()+HEADER_SIZE);
                }else{return -1;}
                path_pop();
            }
        }
        //no more file or dir in current dir
        putMagic();
        putchar(END_OF_DIRECTORY);
        putDepth(depth);
        putSize(HEADER_SIZE);
        closedir(dir);
        return 0;
    }
    return -1;
}

/*
 * @brief  Serialize the contents of a file as a single record written to the
 * standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * file to be serialized.  It serializes the contents of that file as a single
 * FILE_DATA record emitted to the standard output.
 *
 * @param depth  The value to be used in the depth field of the FILE_DATA record.
 * @param size  The number of bytes of data in the file to be serialized.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open the file, too many or not enough data bytes read
 * from the file, and I/O errors reading the file data or writing to standard output.
 */
int serialize_file(int depth, off_t size) {
    // To be implemented.
    FILE *f = fopen(path_buf, "r");
    if(f!= NULL){
        putMagic();
        putchar(FILE_DATA);
        putDepth(depth);
        putSize(size);
        char c;
        while((c = fgetc(f))!=EOF){
            putchar(c);
        }
        fclose(f);
        return 0;
    }
    return -1;
}

/**
 * @brief Serializes a tree of files and directories, writes
 * serialized data to standard output.
 * @details This function assumes path_buf has been initialized with the pathname
 * of a directory whose contents are to be serialized.  It traverses the tree of
 * files and directories contained in this directory (not including the directory
 * itself) and it emits on the standard output a sequence of bytes from which the
 * tree can be reconstructed.  Options that modify the behavior are obtained from
 * the global_options variable.
 *
 * @return 0 if serialization completes without error, -1 if an error occurs.
 */
int serialize() {
    // To be implemented.
    //path_init("rsrc/testdir");
    int depth = 0;
    DIR *dir = opendir(path_buf);
    if(dir){
        struct dirent *de = readdir(dir);
        debug("%s", path_buf);
        debug("%s", de->d_name);
        if(S_ISDIR(getMode())){//test type of dir
            putMagic();//3bytes
            putchar(START_OF_TRANSMISSION);//1byte
            putDepth(depth);
            putSize(HEADER_SIZE);
            serialize_directory(depth+1);
        // }else if(S_ISREG(getMode())){
        //      serialize_file(depth+1,getSize());
        }else{return -1;}
        putMagic();//3bytes
        putchar(END_OF_TRANSMISSION);//1byte
        putDepth(depth);
        putSize(HEADER_SIZE);
        fflush(stdout);
        return 0;
    }
    return -1;
}

/**
 * @brief Reads serialized data from the standard input and reconstructs from it
 * a tree of files and directories.
 * @details  This function assumes path_buf has been initialized with the pathname
 * of a directory into which a tree of files and directories is to be placed.
 * If the directory does not already exist, it is created.  The function then reads
 * from from the standard input a sequence of bytes that represent a serialized tree
 * of files and directories in the format written by serialize() and it reconstructs
 * the tree within the specified directory.  Options that modify the behavior are
 * obtained from the global_options variable.
 *
 * @return 0 if deserialization completes without error, -1 if an error occurs.
 */
int deserialize() {
    // To be implemented.
    //path_init("rsrc/test_out");
    debug("******************deserialize here*****************");
    int depth;
    if(passMagic()){//pass magic sequence-3bytes
        if(getchar() == START_OF_TRANSMISSION){//check type-1byte
            depth = passDepth();//check depth-4bytes
            if(passSize() == HEADER_SIZE){//check header size-8bytes
                deserialize_directory(depth+1);//start deserialize(1)
                /*****back from deserialize*****/
                if(passMagic()){//check magic sequence-3bytes
                    if(getchar() == END_OF_TRANSMISSION){//check type-1byte
                        if(passDepth() == depth){//check depth-4bytes
                            if(passSize() == HEADER_SIZE){//check size-8bytes
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
    return -1;
}


int passMagic(){
    int magic0 = getchar();
    int magic1 = getchar();
    int magic2 = getchar();
    if(magic0 == MAGIC0 && magic1 == MAGIC1 && magic2 == MAGIC2){
        return 1;//IS MAGIC
    }
    return 0;
}


off_t passSize(){//pass 8 byte size record
    off_t size = 0;
    int i;
    for(i = 7; i>=0 ; i--){
        size += getchar()<<8*i;
    }
    return size;
}


int passDepth(){
    int depth = 0;
    int i;
    for(i = 3; i>=0; i--){
        depth += getchar()<<8*i;
    }
    return depth;
}


mode_t passMode(){
    mode_t mode = 0;
    int i;
    for(i = 3; i>=0; i--){
        mode += getchar()<<8*i;
    };
    return mode;
}


void passName(int strlen){
    char *name = name_buf;
    while(strlen!=0){
        *name = getchar();
        name++;
        strlen--;
    }
}


void popName(){//clear all char in name_buf
    int i = 0;
    char *nbuf = name_buf;
    while(*(nbuf+i) != '\0'){
        *(nbuf+i)='\0';
        i++;
    }
}


/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */

//return -1 as:
//Invalid number of arguments (too few or too many)
//Invalid ordering of arguments
//A missing parameter to an option that requires one (e.g. -p with no DIR specification).

//If the -h flag is specified, the least significant bit (bit 0) is 1.
//The second-least-significant bit (bit 1) is 1 if -s is passed
//(i.e. the user wants serialization mode).
//The third-least-significant bit (bit 2) is 1 if -d is passed
//(i.e. the user wants deserialization mode).
//The fourth-least-significant bit (bit 3) is 1 if -c is passed
//(i.e. the user wants to "clobber" existing files).
int validargs(int argc, char **argv){//pointer to the pointer to the char
    // To be implemented.
    if(argc > 1){//if there exists a flag
        char *arg;
        //check first flag
        arg = *(argv+1);//skip the first arg
        debug("%s", arg);
        debug("%c", *arg);
        debug("%c", *(arg+1));
        debug("%c", *(arg+2));
        if(*arg == '-'){
            //******case -h
            if(*(arg+1) == 'h' && *(arg+2) == '\0'){
                global_options |= 0x1;
                return 0;
            //******case -s
            }else if(*(arg+1) == 's' && *(arg+2) == '\0'){
                if(argc == 2){
                    path_init(".");
                    global_options |= 0x2;
                    return 0;
                }else if(argc == 4){//the case of -p with /DIR
                    //check if the second flag is -p
                    arg = *(argv+2);
                    debug("%s", arg);
                    if(*arg == '-'){
                        if(*(arg+1) == 'p' && *(arg+2) == '\0'){
                            //check the third arg if the second flag is -p
                            arg = *(argv+3);
                            debug("%s", arg);
                            if(*arg != '\0'){
                                path_init(arg);
                            // DIR* dir = opendir(*(argv+3));
                            // if(dir){
                            //     closedir(dir);
                                global_options |= 0x2;
                                return 0;
                            }
                        }
                    }
                    return -1;
                }
            //******case -d
            }else if(*(arg+1) == 'd' && *(arg+2) == '\0'){
                if(argc == 2){//when there is only -d
                    path_init(".");
                    global_options |= 0x4;
                    return 0;
                }else if(argc == 3){//when there is a -c flag only
                    //check if the second flag is -c
                    path_init(".");
                    arg = *(argv+2);
                    debug("%s", arg);
                    if(*arg == '-'){
                        if(*(arg+1) == 'c' && *(arg+2) == '\0'){
                            global_options |= 0xc;
                            return 0;
                        }
                    }
                    return -1;
                }else if(argc == 4){//case -p with /DIR
                    //check if the second flag is -p
                    arg = *(argv+2);
                    debug("%s", arg);
                    if(*arg == '-'){
                        if(*(arg+1) == 'p' && *(arg+2) == '\0'){
                            //check the third arg if the second flag is -p
                            arg = *(argv+3);
                            debug("%s", arg);
                            if(*arg != '\0'){
                                DIR* dir = opendir(arg);
                                if(!dir){
                                    mkdir(arg, 0700);
                                }
                                path_init(arg);
                            // DIR* dir = opendir(*(argv+3));
                            // if(dir){
                            //     closedir(dir);
                                global_options |= 0x4;
                                return 0;
                            }
                        }
                    }
                    return -1;
                }else if(argc == 5){//combination of -c and -p
                    //second flag can be either -c or -p
                    arg = *(argv+2);
                    if(*arg == '-'){
                        //check if the second flag is -p
                        if(*(arg+1) == 'p' && *(arg+2) == '\0'){
                            // check if the third arg is /DIR
                            arg = *(argv+3);
                            debug("%s", arg);
                            if(*arg != '\0'){
                                DIR* dir = opendir(arg);
                                if(!dir){
                                    mkdir(arg, 0700);
                                }
                                path_init(arg);
                            // DIR* dir = opendir(*(argv+3));
                            // if(dir){
                            //     closedir(dir);
                                arg = *(argv+4);
                                debug("%s", arg);
                                //check if the fourth flag is -c
                                if(*arg == '-'){
                                    if(*(arg+1) == 'c' && *(arg+2) == '\0'){
                                        global_options |= 0xc;
                                        return 0;
                                    }
                                }
                            }
                            return -1;
                        //check if the second flag is -c
                        }else if(*(arg+1) == 'c' && *(arg+2) == '\0'){
                            arg = *(argv+3);
                            debug("%s", arg);
                            if(*arg == '-'){
                                //check if the third flag is -p
                                if(*(arg+1) == 'p' && *(arg+2) == '\0'){
                                    // check if the fourth arg is /DIR
                                    arg = *(argv+4);
                                    debug("%s", arg);
                                    if(*arg != '\0'){
                                        DIR* dir = opendir(arg);
                                        if(!dir){
                                            mkdir(arg, 0700);
                                        }
                                        path_init(arg);
                                    // DIR* dir = opendir(*(argv+4));
                                    // if(dir){
                                    //     closedir(dir);
                                        global_options |= 0xc;
                                        return 0;
                                    }
                                }
                            }
                            return -1;
                        }
                    }
                    return -1;
                }
            }
        }//if-statement for first flag
        return -1;
    }//outer if-statement ends
    return -1;
}


void putMagic(){
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
}


off_t getSize(){
    struct stat stat_buf;
    if(stat(path_buf, &stat_buf)==0){
        return stat_buf.st_size;
    }
    return -1;
}


mode_t getMode(){
    struct stat stat_buf;
    if(stat(path_buf, &stat_buf)==0){
        return stat_buf.st_mode;
    }
    return -1;
}

void putMode(mode_t mode){
    putchar(mode>>24);
    putchar(mode>>16);
    putchar(mode>>8);
    putchar(mode);
}

void putSize(off_t size){
    putchar(size>>56);
    putchar(size>>48);
    putchar(size>>40);
    putchar(size>>32);
    putchar(size>>24);
    putchar(size>>16);
    putchar(size>>8);
    putchar(size);
}

void putName(char *name){
    int i;
    for(i = 0; *(name+i) != '\0'; i++){
        putchar(*(name+i));
    }
}


void putDepth(int depth){
    putchar(depth>>24);
    putchar(depth>>16);
    putchar(depth>>8);
    putchar(depth);
}


int stringCmr(char *str1, char *str2){
    while(*str1 == *str2){
        if(*str1 == '\0' && *str2 == '\0'){
            return 1;
        }
        str1++;
        str2++;
    }
    return 0;
}


int nameSize(char *name){
    int i = 0;
    while(*(name+i)!='\0'){
        i++;
    }
    return i;
}