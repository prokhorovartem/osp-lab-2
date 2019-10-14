#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>

bool isNumberWithThreeDigits(char *string);

int getModeInSymbolicStyle(char *string);

int countRights(const char *permission);

int main(int argc, char **argv) {

    bool isForce = false;

    if (argc < 2) {
        perror("Number of arguments can't be less than 2");
    }

    int c;

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
                {"f", no_argument, 0, 0},
        };

        c = getopt_long(argc, argv, "f",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'f':
                isForce = true;
                break;
        }
    }

    int mode = 0, modeIndex = 1;

    if (isForce) {
        modeIndex++;
    }

    if (isNumberWithThreeDigits(argv[modeIndex])) {
        mode = (argv[modeIndex][0] - '0') * 64 | (argv[modeIndex][1] - '0') * 8 | (argv[modeIndex][2] - '0');
    } else {
        mode = getModeInSymbolicStyle(argv[modeIndex]);
    }

    for (int i = ++modeIndex; i < argc; ++i) {
        chmod(argv[i], mode);
    }

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

int getModeInSymbolicStyle(char *string) {
    int length = strlen(string), counter = 0;
    int userMode = 0, groupMode = 0, otherMode = 0;
    char who[3], operation[1], permission[3];

    for (int i = 0; i < length; i++) {
        if (string[i] == 'u' || string[i] == 'g' || string[i] == 'o') {
            who[i] = string[i];
            continue;
        }
        if (string[i] == '=') {
            operation[0] = string[i];
            continue;
        }
        if (string[i] == 'r' || string[i] == 'w' || string[i] == 'x') {
            permission[counter++] = string[i];
            continue;
        }
    }

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