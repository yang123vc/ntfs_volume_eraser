#include "command_line_parser.h"

#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <Windows.h>

static CommandLineParams& getCommandLineParam()
{
    static CommandLineParams p;
    return p;
}

void enumerateNtfsFilesystem(HANDLE volumeHandle, USN nextUsn)
{
    MFT_ENUM_DATA masterFileTableData{};
    masterFileTableData.StartFileReferenceNumber = 0;
    masterFileTableData.LowUsn = 0;
    masterFileTableData.HighUsn = nextUsn;

    char usnRecordBuffer[4096] = {};
    DWORD usnDataSize{};
    PUSN_RECORD UsnRecord{};
    std::map<DWORDLONG, std::string> frmName;
    std::map<DWORDLONG, DWORDLONG> frmPfrm;

    while (0 != DeviceIoControl(volumeHandle, 
        FSCTL_ENUM_USN_DATA, 
        &masterFileTableData, 
        sizeof(masterFileTableData), 
        usnRecordBuffer, 
        4096, 
        &usnDataSize, NULL)) {

        DWORD dwRetBytes = usnDataSize - sizeof(USN);
        UsnRecord = (PUSN_RECORD)(((PCHAR)usnRecordBuffer) + sizeof(USN));
        while (dwRetBytes > 0) {
            const int strLen = UsnRecord->FileNameLength;
            char fileName[MAX_PATH] = { 0 };
            WideCharToMultiByte(CP_OEMCP, NULL, UsnRecord->FileName, strLen / 2, fileName, strLen, NULL, FALSE);
            frmName[UsnRecord->FileReferenceNumber] = fileName;
            frmPfrm[UsnRecord->FileReferenceNumber] = UsnRecord->ParentFileReferenceNumber;
            DWORD recordLen = UsnRecord->RecordLength;
            dwRetBytes -= recordLen;
            UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + recordLen);
        }
        masterFileTableData.StartFileReferenceNumber = *(USN*)&usnRecordBuffer;
    }

    std::stack<std::string> s;
    auto iterFrm = frmName.begin();
    auto iterPfrm = frmPfrm.begin();
    for (; iterFrm != frmName.end(); ++iterFrm) {

        DWORDLONG key = iterFrm->first;
        s.push(iterFrm->second);

        while (key != 0) {

            iterPfrm = frmPfrm.find(key);
            if (iterPfrm != frmPfrm.end()) {
                key = iterPfrm->second;
            }
            else {
                key = 0;
            }

            if (key != 0) {
                auto iter = frmName.find(key);
                if (iter != frmName.end()) {
                    s.push(iter->second);
                    key = iter->first;
                }
                else {
                    key = 0;
                }
            }
        }

        while (!s.empty()) {
            std::cout << s.top() << "\\";
            s.pop();
        }
    }
}

HANDLE getVolumeHandle(char volumeLetter)
{
    static const size_t rootLetterPosition = 0;
    static const size_t volumeLetterPosition = 4;

    char volumeRootPath[] = "X:\\";
    char volumeFilename[] = "\\\\.\\X:";
    volumeRootPath[rootLetterPosition] = ::toupper(volumeLetter);
    volumeFilename[volumeLetterPosition] = ::toupper(volumeLetter);

    bool isNTFS = false;
    char sysNameBuf[MAX_PATH] = {0};
    int status = GetVolumeInformationA(volumeRootPath, NULL, 0, NULL, NULL, NULL, sysNameBuf, MAX_PATH);

    if (0 != status) {
        if (0 == strcmp(sysNameBuf, "NTFS")) {
            isNTFS = true;
        }
    }
    else {
        throw std::system_error(GetLastError(), std::generic_category(), "GetVolumeInformation error");
    }

    HANDLE retHandle = CreateFileA(volumeFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (INVALID_HANDLE_VALUE == retHandle) {
        throw std::system_error(GetLastError(), std::generic_category(), "CreateFile and opening volume error");
    }
    return retHandle;
}

void usage()
{
    std::cout << getCommandLineParam().optionsDescription() << '\n';
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    getCommandLineParam().readParams(argc, argv);

    if (getCommandLineParam().isCmdLineHelp()) {
        usage();
    }

    bool useOverlapped = getCommandLineParam().isUseOverlapped();
    bool enumerateNTFS = getCommandLineParam().isEnumerateNtfs();
    bool eraseJournal = getCommandLineParam().isEraseNtfs();
    
    HANDLE volumeHandle = INVALID_HANDLE_VALUE;
    char driveLetter = (getCommandLineParam().drive_letter().size()) ? getCommandLineParam().drive_letter()[0] : 'C';

    try{
        volumeHandle = getVolumeHandle('C');
    }
    catch (const std::system_error& e) {
        std::cerr << "Creating volume error\n";
        std::cerr << "Description: " << e.what() << '\n';
        std::cerr << "ErrorCode = " << e.code() << '\n';
    }

    DWORD br{};
    CREATE_USN_JOURNAL_DATA usnJournalData = {};
    BOOL success = DeviceIoControl(volumeHandle, FSCTL_CREATE_USN_JOURNAL, &usnJournalData, sizeof(usnJournalData), NULL, 0, &br, NULL);
    if (FALSE == success) {
        std::cerr << "DeviceIoControl (FSCTL_CREATE_USN_JOURNAL) error code = " << GetLastError() << '\n';
        return 1;
    }

    bool getBasicInfoSuccess = false;
    USN_JOURNAL_DATA UsnInfo;
    success = DeviceIoControl(volumeHandle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &UsnInfo, sizeof(UsnInfo), &br, NULL);
    if (FALSE == success) {
        std::cerr << "DeviceIoControl (FSCTL_QUERY_USN_JOURNAL) error code = " << GetLastError() << '\n';
        return 1;
    }

    if (enumerateNTFS) {
        enumerateNtfsFilesystem(volumeHandle, UsnInfo.NextUsn);
    }
    
    DELETE_USN_JOURNAL_DATA deleteUsn;
    if (eraseJournal && useOverlapped) {

        deleteUsn.UsnJournalID = UsnInfo.UsnJournalID;
        deleteUsn.DeleteFlags = USN_DELETE_FLAG_DELETE | USN_DELETE_FLAG_NOTIFY;
        OVERLAPPED ov{};
        std::cout << "NTFS journal cleaning started\n";
        success = DeviceIoControl(volumeHandle, FSCTL_DELETE_USN_JOURNAL, &deleteUsn, sizeof(deleteUsn), NULL, 0, NULL, &ov);

        if (FALSE == success) {
            std::cerr << "DeviceIoControl (FSCTL_DELETE_USN_JOURNAL) error code = " << GetLastError() << '\n';
            return 1;
        }

        ::WaitForSingleObject(ov.hEvent, INFINITE);
    }
    else if (eraseJournal && !useOverlapped) {
        deleteUsn.UsnJournalID = UsnInfo.UsnJournalID;
        deleteUsn.DeleteFlags = USN_DELETE_FLAG_NOTIFY;
        success = DeviceIoControl(volumeHandle, FSCTL_DELETE_USN_JOURNAL, &deleteUsn, sizeof(deleteUsn), NULL, 0, &br, NULL);

        if (FALSE == success) {
            std::cerr << "DeviceIoControl (FSCTL_DELETE_USN_JOURNAL) error code = " << GetLastError() << '\n';
            return 1;
        }
        else {
            std::cout << "NTFS journal cleaning started\n";
        }
    }

    std::cout << "NTFS journal cleaning completed\n";
    
    return 0;
}
