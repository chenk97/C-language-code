#include <stdio.h>
#include <stdlib.h>

#include "const.h"
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

int main(int argc, char **argv)
{
    // path_init("hello");
    // debug("%s",path_buf);
    // path_push("world");
    // path_push("hahaha");
    // path_pop();
    // debug("length after pop %d", path_length);
    // debug("%s",path_buf);
    // path_pop();
    // debug("length after 2nd pop %d", path_length);
    // debug("%s",path_buf);

    /********************/
    // mode_t mode =0;
    // mode|=0x000041ed;
    // if(S_ISDIR(mode)){
    //     debug("dirrrrrrrrrrrrr");
    // }



    // int dir_result = mkdir("rsrc/test_outt", 0700);
    // if(dir_result != 0){
    // //errors here
    //     debug("**********************error************************");
    // }
    // else{
    // //your code here
    //     debug("***************hello*****************");
    // }
    int ret;
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%x", global_options);
    if(global_options & 1)//return flase unless global_option = 1 whihc indicates -h
        USAGE(*argv, EXIT_SUCCESS);//print help menu
    if(global_options == 2){//-p /tmp will be the DIR to serialize
        debug("serialization");
        if(serialize()){return EXIT_FAILURE;}
        //return EXIT_SUCCESS;
    }
    if(global_options == 4 || global_options ==12){//p /tmp will be the DIR to deserialize
        debug("deserialization");
        if(deserialize()){return EXIT_FAILURE;}
        //return EXIT_SUCCESS;
    }
    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
