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

// Include Files
#include <iostream>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "capture_and_compare.h"
#include "gtest/gtest.h"

// just returns "stdout.txt" for now.
const char *io_redirect_t::redirect_filename()
{
    return _filename;
}

void io_redirect_t::redirect_start(const char *filename)
{
    // e.g. stdout_copy should be '4'
    stdout_copy = dup(STDOUT_FILENO);

     // eventually assume filename is supposed to be an argument to mktemp
    _filename = filename;
    // perhaps use mktemp to create a temporary file...
    // for now using popen('tee <filename>').
    // todo: replace with mktemp and open().
    ostringstream buf;
    buf << "tee ";
    buf << filename;

    std::cout << "redirecting stdout to \"" << buf.str() << "\"\n";
    stdout_redirect = popen(buf.str().c_str(),"w");
    if(stdout_redirect == NULL)
    {
        close(stdout_copy);
        return;
    }
    int stdout_redirect_pathno;
    stdout_redirect_pathno = fileno(stdout_redirect);
    dup2(stdout_redirect_pathno,STDOUT_FILENO);
    printf("output redirected to tee stdout.txt\n");
}

void io_redirect_t::redirect_end()
{
    fflush(stdout);
    cout << std::flush;
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    pclose(stdout_redirect);
    printf("stdout restored ?\n");
}

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


/**
 * @brief look for
 */
bool file_match_t::look_for(std::string match_pattern, std::string *match_line_ptr, std::smatch *matches_ptr)
{
    std::cout << "looking for " << match_pattern << "\n";
    std::regex regular_expression(match_pattern);
    std::string source_line;
    std::smatch matches;

    // continue reading from the source file line by line until we find the first match
    while (std::getline(input_stream, source_line))
    {
        line_num++;
        if (std::regex_match(source_line, matches, regular_expression))
        {
            std::cout << "matched: " << source_line << " (line " << line_num << ")\n";

            last_match_line_num = line_num;
            if (match_line_ptr != NULL)
            {
                *match_line_ptr = source_line;
            }
            if (matches_ptr != NULL)
            {
                *matches_ptr = matches;
            }
            return true;
        }
        //std::cout << "no match: " << source_line << "\n";
    }
    ADD_FAILURE() << "did not find " << match_pattern << " in " << _source_filename << "\n"
                  << "last match was at line " << last_match_line_num << "\n"
                  << "try sed 1," << last_match_line_num-1 << "d " << _source_filename << " | grep '" << match_pattern << "'\n";

    return false;
}

bool file_match_t::compare_file( const char *match_filename )
{
    std::ifstream match_file;
    match_file.open(match_filename);
    std::string match_line;

    while (std::getline(match_file, match_line))
    {
        if (!(look_for(match_line,NULL,NULL)))
        {
            //ADD_FAILURE() << "did not find " << match_line << " in " << _source_filename << "\n";
            return false;
        }
    }
    return true;
}

bool file_match_t::compare_patterns( match_patterns_t *patterns, uint32_t size )
{
    for (uint32_t i=0;i<size;i++)
    {
        match_patterns_t *pattern = &(patterns[i]);
        if (!look_for( pattern->match_pattern, pattern->match_line, pattern->matches ))
        {
            return false;
        }
    }
    return true;
}

// End of file
