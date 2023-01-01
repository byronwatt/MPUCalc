/*******************************************************************************
*  COPYRIGHT (C) ALL RIGHTS RESERVED.
* -----------------------------------------------------------------------------
*  This software is licensed to customers pursuant to the terms and conditions
*  of the Software License Agreement contained in the text file software.lic
*  that is distributed along with this software. This software can only be
*  utilized if all terms and conditions of the Software License Agreement are
*  accepted. If there are any questions, concerns, or if the Software License
*  Agreement text file, software.lic, is missing please contact applications for
*  assistance.
*******************************************************************************/

/**
* @file
* @brief
*   This file a bunch of classes for helping capture and compare log files.
*/
#ifndef CAPTURE_AND_COMPARE_H
#define CAPTURE_AND_COMPARE_H

// Include Files
#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>

// todo: remove this using and explicitly add std::
using namespace std;

class io_redirect_t {
public:
    void redirect_start(const char *filename);
    void redirect_end();
    const char *redirect_filename();
private:
    int stdout_copy; // copy of stdout
    FILE *stdout_redirect;
    const char *_filename;
} ;

// the R"(xxxxxxxxx)" is c++11 way of inserting raw text where \ is not interpreted.
// so R"(\)" is the same as "\\"
// so R"(  \(hello\)  )" is the same as "  \\(hello\\)  "
//
//
// the workflow for matching a string is quite painful actually:
//
// todo: create script to convert from stdout to a match pattern:
//  * surround with a raw string literal: R"~()~"
//  * convert '=<number>'' with '=[0-9]+'' if the number will fluctuate from run to run
//  * convert '(' to '\('
//  * convert ')' to '\)'
//  * convert '[' to '[[]'
//  * convert '+' to '[+]'
//  * convert '*' to '[*]'
//  * convert 'usec' to '.*' since sometimes it will be usec sometimes milliseconds etc.
//  * either ignore the timestamp: '00:00:00.003710538' or i guess you could add '[0-9:.]+'
//
// so if you want to check that a string like:
//    00:00:00.023100030  HOSTMSG_INFO MUTEX_LOCK_GRANTED: task=HOSTMSG MT_MUTEX_TEST_A lock granted (was owned by DEBUG_TASK) spin_count=17 elapsed_time=102 usec ra=debug_command_mutex_thrash::action+89 
// appears in the log file you'd have to convert that to something like:
//    {
//       R"~(.*HOSTMSG_INFO MUTEX_LOCK_GRANTED: task=HOSTMSG MT_MUTEX_TEST_A lock granted \(was owned by DEBUG_TASK\) spin_count=[0-9]+ elapsed_time=[0-9.]+ .* ra=debug_command_mutex_thrash::action.*)~",
//       NULL, // provide a pointer to a std::string if you want to see the 
//       NULL // provide a pointer to an smatch variable if you want to examine specific groups in the pattern (e.g. to check actual values)
//    },
//
typedef struct {
    const char *match_pattern;
    std::string *match_line; // pointer to std::string returning the actual line that matched.
    std::smatch *matches; // pointer to smatch groups showing groups that matched if you want to extract various bits from the pattern.
} match_patterns_t;


// compare two files
// the first file is a list of match patterns
// the second file is the must match the list of match patterns.
//
// e.g. the first file might be:
// .* DEBUG MDX2_MT_MUTEX_LOCK_WAITING MUTEX_LOCK_WAITING: task=DEBUG_TASK owner=NULL MT_MUTEX_TEST_A waiting \(originally owned by NULL\) spin_count=1 pc=mutex_thrash::action\+\d* ra=mutex_thrash::action\+\d*
//
// and the second file might be:
//   00:00:00.043648430  DEBUG MDX2_MT_MUTEX_LOCK_WAITING MUTEX_LOCK_WAITING: task=DEBUG_TASK owner=NULL MT_MUTEX_TEST_A waiting (originally owned by NULL) spin_count=1 pc=mutex_thrash::action+41 ra=mutex_thrash::action+41
//   00:00:00.043649291  DEBUG MDX2_MT_MUTEX_UNLOCK MDX2_MT_MUTEX_UNLOCK: task=HOSTMSG MT_MUTEX_TEST_A unlocked old_val=2 pc=debug_command_mutex_thrash::action+67 ra=debug_command_mutex_thrash::action+67


class file_match_t {
public:
    file_match_t( const char *source_filename ):
    _source_filename(source_filename),
    source_file(_source_filename),
    string_stream(),
    input_stream(source_file),
    line_num(),
    last_match_line_num()
    {
    }
    file_match_t( std::string input_string ):
    _source_filename("from_string"),
    source_file(),
    string_stream(input_string),
    input_stream(string_stream),
    line_num(),
    last_match_line_num()
    {
    }
    bool look_for(std::string match_pattern, std::string *match_line_ptr = NULL, std::smatch *matches_ptr = NULL);
    bool compare_patterns( match_patterns_t *patterns, uint32_t size );
    bool compare_file( const char *match_filename );

private:
    const char *_source_filename;
    std::ifstream source_file;
    std::istringstream string_stream;
    std::istream &input_stream;
    uint32_t line_num;
    uint32_t last_match_line_num;
} ;


/// e.g. std::string str_val2 = "0.887";
/// EXPECT_STRING_AS_DOUBLE_NEAR( 0.88, str_val2, 0.01 );
/// converts str_val2 to a double then checks that it within +/- 0.01 of 0.88
#define EXPECT_STRING_AS_DOUBLE_NEAR( val1, str_val2, abs_error ) \
    EXPECT_NEAR( val1, stod(str_val2), abs_error ) << "string value:\"" << str_val2 << "\"\n"

#endif

// End of file
