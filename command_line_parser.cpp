#include "command_line_parser.h"
#include <string>
#include <iostream>

namespace po = boost::program_options;
using std::string;

CommandLineParams::CommandLineParams() : cmd_options_description("NTFS journal eraser console has the following options")
{}

void CommandLineParams::read_params(int argc, char* argv[])
{
    cmd_options_description.add_options()
        ("help,h", "Print usage")
        ("erase,e", "")
        ("use-overlapped,o", "")
        ("enumerate-ntfs,e", "")
        ("drive-letter,l", po::value<string>(&_drive_letter), "")
        ;

    // command line params processing
    po::variables_map cmd_variables_map;
    po::store(parse_command_line(argc, argv, cmd_options_description), cmd_variables_map);
    po::notify(cmd_variables_map);

    set_flag(cmd_variables_map, _help, "help");
    set_flag(cmd_variables_map, _erase_ntfs_journal, "erase");
    set_flag(cmd_variables_map, _enumerate_ntfs, "enumerate-ntfs");
    set_flag(cmd_variables_map, _use_overlapped, "use-overlapped");

    // do not check debug flags!
    if (_help && _erase_ntfs_journal && _enumerate_ntfs && _use_overlapped) {
        throw std::logic_error("Use one of application command line options");
    }
}

/**@brief Set some logical param */
void CommandLineParams::set_flag(const po::variables_map& vm, bool& flag, const char* str)
{
    if (vm.count(str)) {
        flag = true;
    }
}
