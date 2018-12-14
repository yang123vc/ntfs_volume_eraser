#include "command_line_parser.h"
#include <string>
#include <iostream>

namespace po = boost::program_options;
using std::string;

CommandLineParams::CommandLineParams() : cmdOptionsDescription("NTFS journal eraser console has the following options")
{}

void CommandLineParams::readParams(int argc, char* argv[])
{
    cmdOptionsDescription.add_options()
        ("help,h", "Print usage")
        ("erase,e", "Erase NTFS filesystem journal")
        ("use-overlapped,o", "Use asynchronous overlapped IO API during erasure")
        ("enumerate-ntfs,e", "Enumerate NTFS filesystem journal")
        ("drive-letter,l", po::value<string>(&_driveLetter), "Letter of drive which NTFS journal is opened")
        ;

    // command line params processing
    po::variables_map cmdVariablesMap;
    po::store(parse_command_line(argc, argv, cmdOptionsDescription), cmdVariablesMap);
    po::notify(cmdVariablesMap);

    setFlag(cmdVariablesMap, _help, "help");
    setFlag(cmdVariablesMap, _eraseNtfsJournal, "erase");
    setFlag(cmdVariablesMap, _enumerateNtfs, "enumerate-ntfs");
    setFlag(cmdVariablesMap, _useOverlapped, "use-overlapped");

    // do not check debug flags!
    if (_help && _eraseNtfsJournal && _enumerateNtfs && _useOverlapped) {
        throw std::logic_error("Use one of application command line options");
    }
}

/**@brief Set some logical param */
void CommandLineParams::setFlag(const po::variables_map& vm, bool& flag, const char* str)
{
    if (vm.count(str)) {
        flag = true;
    }
}
