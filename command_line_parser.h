#pragma once
#include <string>
#include <boost/program_options.hpp>

// The header contains command-line parser, uses Boost.ProgramOptions

namespace panic {

/**@brief Storage for all passed and default command-line params */
class CommandLineParams {
public:

    /// @brief Should have a constructor by default
    CommandLineParams();

    /// @brief Parse raw command-line parameters
    void read_params(int argc, char* argv[]);

    /// @brief Set some logical param
    void set_flag(const boost::program_options::variables_map& vm, bool& flag, const char* str);

    /// @brief Get available options list
    boost::program_options::options_description& options_descript(){ return cmd_options_description; }

    //////////////////////////////////////////////////////////////////////////
    /// Accessors

    /// @brief Print command line help
    bool is_help() const 
    {
        return _help;
    }

    /// @brief 
    const std::string& drive_letter() const 
    {
        return _drive_letter;
    }

    /// @brief 
    bool is_enumerate_ntfs() const 
    {
        return _enumerate_ntfs;
    }

    /// @brief 
    bool is_use_overlapped() const 
    {
        return _use_overlapped;
    }

    /// @brief 
    bool is_erase_ntfs_journal() const 
    {
        return _erase_ntfs_journal;
    }

private:

    /// Show help
    bool _help = false;

    /// 
    std::string _drive_letter;

    /// 
    bool _enumerate_ntfs = false;

    /// 
    bool _use_overlapped = false;

    /// 
    bool _erase_ntfs_journal = false;

    /// Command-line params description
    boost::program_options::options_description cmd_options_description;
};

} // namespace panic
