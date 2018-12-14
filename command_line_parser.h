#pragma once
#include <string>
#include <boost/program_options.hpp>

// The header contains command-line parser, uses Boost.ProgramOptions

/**@brief Storage for all passed and default command-line params */
class CommandLineParams {
public:

    /// @brief Should have a constructor by default
    CommandLineParams();

    /// @brief Parse raw command-line parameters
    void readParams(int argc, char* argv[]);

    /// @brief Set some logical param
    void setFlag(const boost::program_options::variables_map& vm, bool& flag, const char* str);

    /// @brief Get available options list
    boost::program_options::options_description& optionsDescription(){ return cmdOptionsDescription; }

    //////////////////////////////////////////////////////////////////////////
    /// Accessors

    /// @brief Print command line help
    bool isCmdLineHelp() const 
    {
        return _help;
    }

    /// @brief Letter of drive which NTFS journal is opened
    const std::string& drive_letter() const 
    {
        return _driveLetter;
    }

    /// @brief Enumerate NTFS filesystem journal
    bool isEnumerateNtfs() const 
    {
        return _enumerateNtfs;
    }

    /// @brief Use asynchronous overlapped IO API during erasure
    bool isUseOverlapped() const 
    {
        return _useOverlapped;
    }

    /// @brief Erase NTFS filesystem journal
    bool isEraseNtfs() const 
    {
        return _eraseNtfsJournal;
    }

private:

    /// Show help
    bool _help = false;

    /// Letter of drive which NTFS journal is opened
    std::string _driveLetter;

    /// Enumerate NTFS filesystem journal
    bool _enumerateNtfs = false;

    /// Use asynchronous overlapped IO API during erasure
    bool _useOverlapped = false;

    /// Erase NTFS filesystem journal
    bool _eraseNtfsJournal = false;

    /// Command-line params description
    boost::program_options::options_description cmdOptionsDescription;
};
