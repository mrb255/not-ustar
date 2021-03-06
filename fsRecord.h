#define PADDING_SIZE 26

struct fsRecord_header
{
    char eyecatch[6];

    char fName[255];
    // we don't care about security
    char uid[8];
    char gid[8];
    char fsSize[12];
    char atime[12];
    char mtime[12];

    char chksum[8];
    char isDirectory;
    char linkName[100];

    char uname[32];
    char gname[32];

    char padding[PADDING_SIZE];
};
