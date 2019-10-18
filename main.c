#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>

bool isNumberWithThreeDigits(char *string);

int getModeInSymbolicStyle(char *string, mode_t permissions);

int countRights(const char *permission);

mode_t getPermissions(char *file);

bool isDirectory(char *string);

void changeModeRecursively(char *string, int i);

bool isForce = false;

int main(int argc, char **argv) {

    bool isDigitalFormat = false;
    bool isRecursive = false;

    if (argc < 2) {
        perror(strerror(5));
    }

    int c;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
                {"f", no_argument, 0, 0},
        };

        c = getopt_long(argc, argv, "fR",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'f':
                isForce = true;
                break;
            case 'R':
                isRecursive = true;
                break;
        }
    }

    int mode = 0, modeIndex = 1;

    if (isForce) {
        modeIndex++;
    }

    if (isRecursive) {
        modeIndex++;
    }

    if (isNumberWithThreeDigits(argv[modeIndex])) {
        isDigitalFormat = true;
        mode = (argv[modeIndex][0] - '0') * 64 | (argv[modeIndex][1] - '0') * 8 | (argv[modeIndex][2] - '0');
    }

    for (int i = modeIndex + 1; i < argc; ++i) {
        if (isDigitalFormat) {
            if (isRecursive == true && isDirectory(argv[i])) {
                changeModeRecursively(argv[i], mode);
            }
            if (chmod(argv[i], mode) == -1 && isForce == false) {
                perror(strerror(errno));
            }
        } else {
            mode = getModeInSymbolicStyle(argv[modeIndex], getPermissions(argv[i]));
            if ( isRecursive == true && isDirectory(argv[i])) {
                changeModeRecursively(argv[i], mode);
            }
            if (chmod(argv[i], mode) == -1 && isForce == false) {
                perror(strerror(errno));
            }
        }
    }

}

void changeModeRecursively(char *string, int mode) {
    DIR *dir;
    struct dirent *myFile;
    dir = opendir(string);
    if (dir) {
        while ((myFile = readdir(dir))) {
            if(myFile->d_name[0] != '.') {
                char dest[128];
                strcpy(dest, string);
                strcat(dest, "/");
                strcat(dest, myFile->d_name);
                if (isDirectory(dest)) {
                    changeModeRecursively(dest, mode);
                }
                if (chmod(dest, mode) == -1 && isForce == false) {
                    perror(strerror(errno));
                }
            }
        }
    } else if (errno == ENOENT)
        puts("This directory does not exist.");
    else if (errno == ENOTDIR)
        puts("This file is not a directory.");
    else if (errno == EACCES)
        puts("You do not have the right to open this folder.");
    else
        puts("That's a new error, check the manual.");
}

bool isDirectory(char *path) {
    struct stat st;
    if (stat(path, &st) == -1 && isForce == false) {
        perror(strerror(errno));
    }
    return S_ISDIR(st.st_mode);
}

bool isNumberWithThreeDigits(char *string) {
    int length;

    length = strlen(string);
    if (length != 3) {
        return false;
    }

    for (int i = 0; i < length; i++) {
        if (!isdigit(string[i]) || string[i] == '8' || string[i] == '9') {
            return false;
        }
    }
    return true;
}

int getModeInSymbolicStyle(char *string, mode_t permissions) {
    int length = strlen(string), counter = 0;
    int userMode = 0, groupMode = 0, otherMode = 0;
    char who[3], operation = 0, permission[3];

    for (int i = 0; i < length; i++) {
        if (string[i] == 'u' || string[i] == 'g' || string[i] == 'o') {
            who[i] = string[i];
            continue;
        }
        if (string[i] == '=' || string[i] == '-' || string[i] == '+') {
            operation = string[i];
            continue;
        }
        if (string[i] == 'r' || string[i] == 'w' || string[i] == 'x') {
            permission[counter++] = string[i];
            continue;
        }
    }

    switch (operation) {
        case '=':
            for (int j = 0; j < 3; ++j) {
                switch (who[j]) {
                    case 'u':
                        userMode = countRights(permission);
                        break;
                    case 'g':
                        groupMode = countRights(permission);
                        break;
                    case 'o':
                        otherMode = countRights(permission);
                        break;
                }
            }
            return userMode * 64 | groupMode * 8 | otherMode;
        case '+':
            for (int j = 0; j < 3; ++j) {
                switch (who[j]) {
                    case 'u':
                        userMode = countRights(permission);
                        break;
                    case 'g':
                        groupMode = countRights(permission);
                        break;
                    case 'o':
                        otherMode = countRights(permission);
                        break;
                }
            }
            return (permissions & 511) | userMode * 64 | groupMode * 32 | otherMode;
        case '-':
            for (int j = 0; j < 3; ++j) {
                switch (who[j]) {
                    case 'u':
                        userMode = countRights(permission);
                        break;
                    case 'g':
                        groupMode = countRights(permission);
                        break;
                    case 'o':
                        otherMode = countRights(permission);
                        break;
                }
            }
            return (permissions & 511) & ~(userMode * 64) & ~(groupMode * 32) & ~(otherMode);
        default:
            return 0;
    }
}

int countRights(const char *permission) {
    int rightsCounter = 0;
    for (int i = 0; i < 3; ++i) {
        if (permission[i] == 'r') {
            rightsCounter += 4;
        }
        if (permission[i] == 'w') {
            rightsCounter += 2;
        }
        if (permission[i] == 'x') {
            rightsCounter += 1;
        }
    }
    return rightsCounter;
}

mode_t getPermissions(char *file) {
    struct stat st;
    if (stat(file, &st) == -1 && isForce == false) {
        perror(strerror(errno));
    }
    return st.st_mode;
}
