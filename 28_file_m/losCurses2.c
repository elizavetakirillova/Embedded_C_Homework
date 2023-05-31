
//#include <stdio.h>
//#include <string.h>
//#include <stdint.h>
//#include <stdlib.h>
//
//
//int main() {
//	 
//	printf("los calling is aaaa");
//	 
//	return 0;
//}

#include <sys/wait.h>
#include <time.h>
#include <ncurses.h>
#include <dirent.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>


#define __USE_XOPEN


WINDOW* createa_newwin(int height, int width, int startX, int startY);
void updateStatusWindow(char* path, char* name);
void DestroyALL();
bool updateLeftWindow(char* currentPath, bool needUpdate);
int currentFilesLeft;
int currentFilesRight;
bool checkUpPathIsFolder(char* currentPath);
char* currentFiles1; // left window
char* currentFiles2; // right window 
char* currentFilesInFolders(char* nameFoldersPath, int numberFiles);
int currentNumberFilesInFolder(char* nameDirPath);

WINDOW* my_winlos;
WINDOW* my_winlos2;
WINDOW* my_winStatus;

const char* stringDot = ".";
const char* stringTwoDot = "..";
const int finInt = 1;
char* leftListNames[100];
char* rightListNames[100];


int cSEle; // current Selected Element;
int currentLeftElementsInFolder;
int currentRightElementsInFolder;
char* cNamesSelected;
char* cwdLEFT;

char* returnFilesNames(char* path, int number);
static char* cwd;

int main(void) {

    int ch;
    initscr();
    start_color();
    noecho();
    curs_set(0);
    cSEle = 1;

    DIR* folder;
    struct dirent* entry;
    int files = 0;
    folder = opendir(".");

    init_pair(1, COLOR_RED, COLOR_WHITE);

    if (folder == NULL) {
        printw(" not ok open ");
        exit(1);

    }
    else {

        while ((entry = readdir(folder)))
        {
            files++;
            currentFilesLeft++;
            currentFilesRight++;
        }

        char* lStringArrayName[currentFilesLeft];

        while ((entry = readdir(folder)))
        {
            strcpy(lStringArrayName[0], entry->d_name);
        }
    }

    closedir(folder);

    keypad(stdscr, TRUE);
    curs_set(0);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    refresh();



    int h, w; // s1, s2;
    h = LINES - 4;  //2; // LINES - 2; // was 3
    w = (COLS / 2) - 2;
    //s1 = LINES / 2;

    // LINES 29 
    // COLUMNS 100   
    //s2 = (COLS - w) / 2;
    //printw( " my lines = %d ", LINES);
    //printw(" my COLUMNS = %d \n", COLS);   
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_CYAN);
    my_winlos = createa_newwin(h, w, 0, 1);
    my_winlos2 = createa_newwin(h, w, 0, (COLS / 2) + 2);
    my_winStatus = createa_newwin(3, COLS - 2, LINES - 3, 1);
    wbkgd(my_winlos, COLOR_PAIR(2));
    wbkgd(my_winlos2, COLOR_PAIR(2));

    refresh();
    wrefresh(my_winlos2);

    //wattron(my_winlos, A_BOLD);
    wattron(my_winlos, COLOR_PAIR(2));
    attron(COLOR_PAIR(2));

    wrefresh(my_winlos);
    wattroff(my_winlos, COLOR_PAIR(0));



    wbkgd(my_winStatus, COLOR_PAIR(3));
    wattron(my_winStatus, COLOR_PAIR(3));
    wrefresh(my_winStatus);
    wattroff(my_winStatus, COLOR_PAIR(3));

    char cwdA[1024];
    if (getcwd(cwdA, sizeof(cwdA)) == NULL) {
        // no files 
        printw(" no cwd files ");
        exit(1);
    }

    //printf(" my cwdA = %d \n", strlen(cwdA));
    const int sizeCWD = strlen(cwdA) + 1;
    cwd = malloc(sizeCWD * sizeof(char));
    strcpy(cwd, cwdA);
    
    
    //printf(" my cwd == %s \n", cwd);

    //int currentFilesLeftWindow = currentNumberFilesInFolder(cwd);

    if (!updateLeftWindow(cwd, false)) {
        printw(" no create left window ! \n");
        exit(1);
    }

    attroff(COLOR_PAIR(2));
    wrefresh(my_winlos);
    attron(COLOR_PAIR(2));

    refresh();
    wrefresh(my_winlos);
    wrefresh(my_winlos2);

    while (1) {

        ch = getch();

        if (ch == KEY_F(1)) {

        }
        else {

            if (ch == KEY_LEFT) {

            }

            if (ch == KEY_UP) {

                if (cSEle - 1 <= 0) {
                    cSEle = 0;
                }
                else {
                    cSEle--;
                }

                updateLeftWindow(cwd, false);
            }

            if (ch == KEY_DOWN) {

                if (cSEle + 1 >= currentLeftElementsInFolder) {
                    cSEle = currentLeftElementsInFolder;
                }
                else {
                    cSEle++;
                }

                updateLeftWindow(cwd, false);
            }

            if (ch == 10) { // ENTER

                //char tempCWD[256];
                //strncpy(tempCWD, cwd, strlen(cwd));

                //strcat(tempCWD, "/");
                //strcat(tempCWD, cNamesSelected);
                struct stat l_info;
                const int simple2 = strlen(cwd) + 1;
                char* tempAddFolderName;

                tempAddFolderName = malloc(simple2 * sizeof(char));
                strcpy(tempAddFolderName, cwd);
                strcat(tempAddFolderName, "/");
                strcat(tempAddFolderName, cNamesSelected);
               // printw(" s %s \n", tempAddFolderName);
                //printw(" my cwd 1dd w == %s \n", tempCWD);

                if (strcmp(cNamesSelected, stringTwoDot) == 0) {

                    int curentCWDLen = strlen(cwd) + 1;
                    char* temp1CWD;
                    temp1CWD = malloc(curentCWDLen * sizeof(char));
                    strcpy(temp1CWD, cwd);
                    //char newCWD[curentCWDLen];
                    //strncpy(newCWD, cwd, strlen(cwd));
                    //printw(" \n\n");
                    //printw(" newCWD = %s \n", newCWD);
                    char* ptr1;
                    int ch = '/';
                    //ptr1 = strrchr(newCWD, ch);
                    ptr1 = strrchr(temp1CWD, ch);
                    //printw(" ptr1 == %s \n", ptr1);
                    //printw("len  ptr1 == %d \n", strlen(ptr1));

                    int currentNEwFolder = curentCWDLen - strlen(ptr1);
                    //char nCWD[currentNEwFolder]; // was 256
                    char* nCWD = malloc((currentNEwFolder + 1) * sizeof(char));
                    strncat(nCWD, temp1CWD, strlen(temp1CWD) - strlen(ptr1));
                    cSEle = 0;
                    //printw(" final  %s \n", nCWD);
                    //cwd[0] = '\0'; 
                     
                    //for (int i = 0; i < 256; i++) {
                    //    cwd[i] = '\0';
                     //}
                     
                    //printw(" final$ %s \n", cwd);
                    //printw(" fin222 %s \n", nCWD);
                    //printw(" lenght %s \n", strlen(nCWD));
                    //printw(" lengh2 %s \n", strlen(cwd));
                    free (cwd);
                    
                    cwd = malloc((currentNEwFolder + 1) * sizeof(char));
                    strcpy(cwd, nCWD);
                    //printw(" fin333 %s \n", nCWD);
                    //printw(" finalF %s \n", cwd);
                     
                    // check first element not balderdash ?&3^@. 

                    // strchr
                    char* newPtr;
                    int check = '/';
                    newPtr = strchr(cwd, check);
                    //printw(" my newPtr == %s \n", newPtr);

                    const int finalINt = strlen(newPtr) + 1;
                    char* tempFinal;
                    tempFinal = malloc(finalINt * sizeof(char));
                    
                    strcpy(tempFinal, newPtr);

                    free(cwd);

                    cwd = malloc(finalINt * sizeof(char));
                    strcpy(cwd, tempFinal);
                    //printw(" my newPtr == %d \n", strlen(newPtr));
                    //for (int i = 0; i < 256; i++) {
                    //    cwd[i] = '\0';
                    //}
                    //printw(" my size PTR = %d \n", (int) strlen(&newPtr));
                    //strcpy(cwd, newPtr);
                    //printw(" my newPRRR  == %s \n", tempFinal);
                    //printw(" my newPtr2  == %s \n", cwd);
                    free(temp1CWD);
                    free(nCWD);
                    free(tempFinal);
                    updateLeftWindow(cwd, true);

                }
                else if (stat(tempAddFolderName, l_info.st_mode & __S_IFDIR) != 0) {

                    //printw(" my cwd add w == %s \n", tempAddFolderName);
                    

                    const int intNewPath = strlen(tempAddFolderName) + 1;
                    //char* nePath;
                    free(cwd);
                    cwd = malloc(intNewPath * sizeof(char));
                    strcpy(cwd, tempAddFolderName);



                    free (tempAddFolderName);
                    updateLeftWindow(cwd, true);

                }



                //else if (l_info.st_mode & S_IFDIR) {  // was S_IFDIR
                //   // S_ISDIR

                //    //int allocate = snprintf(NULL, 0, "%s", files->d_name);
                //    //globalTime = malloc(allocate + 1);
                //   // snprintf(globalTime, allocate + 1, "%s", files->d_name);
                //     //  cwd = &tempCWD;
                //      //  strcpy(cwd, tempCWD);
                //    updateLeftWindow(tempCWD, true);

                //}
                else {
                    //  printf(" no this directory \n");
                }

            }
            else
            {
            }

            //if( stat(tempCWD, &l_info) == 0 && l_info.st_mode & S_IXUSR){

             //  pid_t forkLos;
             //  forkLos = fork();

             //if( forkLos == 0){
             //  sleep(1);
             //  char* args[] = {" test calling", NULL};
                // "/home/loskutnikovlemp3/test22/./outLos"
             //  execv(tempCWD, args);
             // }  else if (forkLos != -1){
             //wait(NULL);
             //printf(" parent  precedure is now goint exit \n");

            //  } else {
            // printf("error occured is calling fork");
            //  }
            // } 
           // } 

            if (ch == 9) {  // TAB

            }
            if (ch == 113) {

                DestroyALL();
                endwin();
                exit(1);
            }

            if (ch == 27) {

                DestroyALL();
                endwin();
                exit(1);
            }
        }
    }  // end While  
    DestroyALL();
    endwin();
    return 0;
}


//char* currentFiles1; // left window
//char* currentFiles2; // right window

char* currentFilesInFolders(char* nameFoldersPath, int numberFiles) {

    char* myName[numberFiles];
    struct dirent* files;
    DIR* dir = opendir(nameFoldersPath); // was "."
    if (dir == NULL) {
        printw(" not open DIR 1 ");
        return myName[0];
    }

    int i = 0;
    while ((files = readdir(dir)) != NULL) {

        myName[i] = files->d_name;
        i++;

    }
    closedir(dir);
    return myName[0];
}




char* returnFilesNames(char* path, int number) {

    // char* returnNamesString[1];  
    // char* globalMemoryString = calloc(2, 16);
    // if(globalMemoryString == NULL ){
      // return ""; 
   // }  


    //int allocate = snprintf(NULL, 0, "%s", ) 

    char* globalTime;
    struct dirent* files;
    DIR* dir = opendir(path);
    if (dir == NULL) {
        printw(" not open DIR 2 %s \n", path);
        return "";
    }

    //memset(globalMemoryString, '\0', 16);

     //char* globalMemoryString;  
    int ii = 0;
    while ((files = readdir(dir)) != NULL) {

        if (ii == number) {

            // size_t mySize = strlen(files->d_name);
            // char returnNamesString[mySize];
             //globalMemoryString = calloc(mySize + 1, sizeof(char));
           //  if(globalMemoryString == NULL ){
             //  return "";
            //}

            //int allocate = snprintf(NULL, 0, "%s\n", files->d_name);
            //printw("allocate my == %d  \n", allocate);
            //printw("filqes d name my == %s  \n", files->d_name);
            const int sizeCurrentFileNames = strlen(files->d_name) + 1;
            globalTime = malloc(sizeCurrentFileNames * sizeof(char));
            if (globalTime == NULL) {
            } else {
                //printw(" files my == %ln \n", sizeof(globalTime));
                snprintf(globalTime, sizeCurrentFileNames, "%s\n", files->d_name);
            }
         
             //  strcpy(globalMemoryString, files->d_name);
             //  memcpy(globalMemoryString,files->d_name, sizeof(files->d_name));
        }

        ii++;
    }
    closedir(dir);
    return globalTime;
    //return globalMemoryString; 
}


bool  updateLeftWindow(char* currentPath, bool needUpdate) {


    int nStep = 0;

    if (needUpdate) {

        // currentLeftElementsInFolder

        attron(COLOR_PAIR(2));
        for (int d = 0; d < currentLeftElementsInFolder; d++) {
            mvprintw(1 + nStep, 2, "%s", "");
            nStep++;
        }

        wbkgd(my_winlos, COLOR_PAIR(2));
        attroff(COLOR_PAIR(2));
        wrefresh(my_winlos);
        refresh();
    }
    nStep = 0;



    if (checkUpPathIsFolder(currentPath)) {
        leftListNames[0] = "..";


        if (cSEle == 0) {
            attron(COLOR_PAIR(1)); // was my_winlos  
            mvprintw(1 + nStep, 2, "%s", leftListNames[0]);
            attroff(COLOR_PAIR(1));

            cNamesSelected = leftListNames[0];
            updateStatusWindow(currentPath, cNamesSelected);

        }
        else {
            attron(COLOR_PAIR(2));
            mvprintw(1 + nStep, 2, "%s", leftListNames[0]);
            attroff(COLOR_PAIR(2));
        }
        nStep++;
    }


    int number = currentNumberFilesInFolder(currentPath);

    // cSEle
    // currentLeftElementsInFolder 

    //printw(" my number %d and %d  \n", number , currentLeftElementsInFolder);
    //printw("     \n      my number   %d \n", number);
    for (int i = 0; i < number; i++) {
        
        char* currentNa;
        //if (!needUpdate) {
            currentNa = returnFilesNames(currentPath, i);
      //      printw("\n my current %s \n", currentNa);
       // }
       // else {
       //     currentNa = "sdfsdf";
       // }

         
        //  = returnFilesNames(currentPath, i);
        // char* currentNa = "sdfsdf";
        if (strcmp(currentNa, stringTwoDot) == 0) {
        }
        else if (strcmp(currentNa, stringDot) == 0) {
        }
        else {

            if (nStep == cSEle) {
                attron(COLOR_PAIR(1)); // was my_winlos
                mvprintw(1 + nStep, 2, "%s", currentNa);
                attroff(COLOR_PAIR(1));
               // printw(" MMMmy this  %d  \n", nStep);
               // 
                cNamesSelected = currentNa;
                updateStatusWindow(currentPath, currentNa);
                //printw(" my selected names = %s \n", cNamesSelected);
                
                // free(currentNa);
            }
            else {
                attron(COLOR_PAIR(2));
                mvprintw(1 + nStep, 2, "%s", currentNa);
                attroff(COLOR_PAIR(2));
                //printw(" my this  %d  \n", nStep);
                // free(currentNa);
            }

            if (currentNa != NULL) {
                free(currentNa);
            }
          

            nStep++;
        }
    }

    currentLeftElementsInFolder = nStep - 1;
    refresh();
    wrefresh(my_winlos);


    return true;
}

int currentNumberFilesInFolder(char* nameDirPath) {

    int numberFile = 0;
    struct dirent* files;
    DIR* dir = opendir(nameDirPath);
    if (dir == NULL) {
        printw(" not open DIR 3 %s \n", nameDirPath);
        return 0;
    }

    while ((files = readdir(dir)) != NULL) {

        //myName[numberFile] = files->d_name;
        //printf("%s \n", files->d_name);
        numberFile++;
    }


    closedir(dir);
    return numberFile;
}

void DestroyALL() {

    wrefresh(my_winlos);
    wrefresh(my_winlos2);
    wrefresh(my_winStatus);

    delwin(my_winlos);
    delwin(my_winlos2);
    delwin(my_winStatus);
}





void updateStatusWindow(char* cwd, char* name)
{

    attron(COLOR_PAIR(3));

    char* cwdTemp;

  /*  if (cwd == NULL) {
        printw(" call is NULL ");
    }
    else {
        printw(" ok work is bets");
    }*/

    const int LInt = strlen(cwd) + 1;
    cwdTemp = malloc(LInt * sizeof(char));
    //printw(" my values string == %s \n", strlen(&cwd));
    strcpy(cwdTemp, cwd);

    char* ptr2 = strcat(cwdTemp, "/");
    char* ptr3 = strcat(cwdTemp, name);
    // printf(" my final files == %s \n" , cwd);

    char date[20];
    struct stat ta2;
    stat(ptr3, &ta2);

    strftime(date, 20, "%d-%m-%y", localtime(&(ta2.st_ctime)));
    mvprintw(LINES - 2, 2, " %s    %ldKB    %s ", name, ta2.st_size, date);

    attroff(COLOR_PAIR(3));
    //free (ptr2);
    //free (ptr3);
    free (cwdTemp);
    wrefresh(my_winStatus);
}


WINDOW* createa_newwin(int height, int width, int startX, int startY) {

    WINDOW* local_win;
    local_win = newwin(height, width, startX, startY);
    box(local_win, 0, 0);

    wrefresh(local_win);
    return local_win;
}




bool checkUpPathIsFolder(char* currentPath) {


    return true;
}
