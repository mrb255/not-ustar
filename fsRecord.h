
struct fsRecord_header
{

    char fName[255];
    // we don't care about security
    char uid[8]
    char gid[8]
    char fsSize[12]
    char mtime[12]

    char chksum[8];
    char isDirectory;
    char linkName[100];

    char uname[32];
    char gname[32];

    char padding[44];
};
