#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_set>

#define DEF_WIZARD_MODE 1

#define COLOR_RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"


// Helper Functions
#ifdef _WIN32
    #include <windows.h>
    void delay(int seconds){ Sleep(seconds * 1000); }
#else
    #include <unistd.h>
    void delay(int seconds){ sleep(seconds); } 
#endif

void initPlatform()
{
    #ifdef _WIN32
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
    
    static bool enabled = false;
    if(!enabled)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if(hOut != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if(GetConsoleMode(hOut, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
        enabled = true;
    }
    #endif
}

void printfColor(const char *color, const char *text){ printf("%s%s%s", color, text, COLOR_RESET); fflush(stdout); }
void clearTerminal(){ printf("\033[2J\033[3J\033[H\033[?25l"); fflush(stdout); }
void showCursor(){ printf("\033[?25h"); fflush(stdout); }

// Game
struct gameStats
{
    float gameTimerProgress;
    float gameTimerRate;

    int maxNumbersBeforeSpeedIncrease;
    int maxNumbersBeforeSpeedDecrease;

    int correctNumbers;
    int incorrectNumbers;
};

std::atomic<int> userInput{0};
std::atomic<bool> isValidNumber{false};
std::atomic<bool> gameRunning{true}; 

void userInputThread()
{
    while(gameRunning)
    {
        if(isValidNumber == true){ continue; }
        
        int input;
        if(!(std::cin >> input)) 
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }else 
        {
            userInput = input;
            isValidNumber = true;
        }
    }
}

void gameLoop(gameStats& stats)
{
    printfColor(RED, "3\n"); delay(1); clearTerminal();
    printfColor(YELLOW, "2\n"); delay(1); clearTerminal();
    printfColor(GREEN, "1\n"); delay(1); clearTerminal();

    int inncorrectNumberCount = 0;
    int correctNumberCount = 0;

    std::unordered_set<int> usedNumbers;

    std::thread inputThread(userInputThread);
    while (stats.gameTimerProgress > 0) 
    {
        if(isValidNumber)
        {
            if(usedNumbers.find(userInput) != usedNumbers.end())
            {
                stats.incorrectNumbers += 1;
                stats.gameTimerProgress -= 2.0f;

                isValidNumber = false;
            }else
            {
                bool isPrime = true;
                
                if (userInput <= 1)
                {
                    isPrime = false;
                }
                else if (userInput <= 3)
                {
                    isPrime = true;
                }
                else if (userInput % 2 == 0 || userInput % 3 == 0)
                {
                    isPrime = false;
                }
                else
                {
                    for (int i = 5; i * i <= userInput; i += 6)
                    {
                        if (userInput % i == 0 || userInput % (i + 2) == 0)
                        {
                            isPrime = false;
                            break;
                        }
                    }
                }

                if(isPrime)
                {
                    usedNumbers.insert(userInput);
                    
                    stats.correctNumbers += 1;
                    correctNumberCount += 1;
    
                    stats.gameTimerProgress += (userInput / 2);
                    if(stats.maxNumbersBeforeSpeedDecrease <= correctNumberCount)
                    {
                        stats.gameTimerRate -= .1;
                        correctNumberCount = 0;
                    }
    
                    if(stats.gameTimerRate <= 0){stats.gameTimerRate = 0.5f; }
                    if(stats.gameTimerRate <= 0){stats.gameTimerRate = 0.5f;}
                }
                else
                {
                    stats.incorrectNumbers += 1;
                    inncorrectNumberCount += 1;
                    
                    if(stats.maxNumbersBeforeSpeedIncrease <= inncorrectNumberCount)
                    {
                        stats.gameTimerRate += .1;
                        inncorrectNumberCount = 0;
                    }
                }
    
                isValidNumber = false;
            }
        }

        printf("== PLAYER STATS == \n");
        printf("Time Left: %f \n", stats.gameTimerProgress );
        printf("Time Rate: %f \n\n", stats.gameTimerRate );
        
        printfColor(GREEN, std::string("Correct Prime Number count: " + std::to_string(stats.correctNumbers) + "\n").c_str());
        printfColor(RED, std::string("Incorrect Prime Number count: " + std::to_string(stats.incorrectNumbers) + "\n").c_str());
        printf("\n");

        #ifdef _WIN32
            Sleep(16);
        #else
            usleep(16000);
        #endif
        
        clearTerminal();
        stats.gameTimerProgress -= stats.gameTimerRate;
    }

    gameRunning = false;
    inputThread.detach();

    printfColor(RED, "=== GAME OVER ===\n\n");
    printfColor(GREEN, std::string("Correct Prime Number count: " + std::to_string(stats.correctNumbers) + "\n").c_str());
    printfColor(RED, std::string("Incorrect Prime Number count: " + std::to_string(stats.incorrectNumbers) + "\n\n").c_str());
    printf("Finial Score: %i", stats.correctNumbers - stats.incorrectNumbers);
    exit(0);
}

int main()
{
    // Init game and select difficultys
    gameStats stats{};
    initPlatform();
    
    printf("Please Select a difficulty \n");
    printf("1 - Normal \n");
    printf("2 - Hard \n");
    printf("3 - Hell \n");
    printf("Enter Number (1-3): ");
    
    int difficulty;
    if(!(std::cin >> difficulty)){ difficulty = 3; }
    
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (difficulty) 
    {
        case 1:
        {
            stats.gameTimerProgress = 100;
            stats.gameTimerRate = 0.1f;
            stats.maxNumbersBeforeSpeedDecrease = 10;
            stats.maxNumbersBeforeSpeedIncrease = 3;
            break;
        }
        case 2:
        {
            stats.gameTimerProgress = 100;
            stats.gameTimerRate = 0.5f;
            stats.maxNumbersBeforeSpeedDecrease = 15;
            stats.maxNumbersBeforeSpeedIncrease = 2;
            break;
        }
        case 3:
        {
            stats.gameTimerProgress = 100;
            stats.gameTimerRate = 1.0f;
            stats.maxNumbersBeforeSpeedDecrease = 20;
            stats.maxNumbersBeforeSpeedIncrease = 1;
            break;
        }
    }

    // Print rules
    clearTerminal();
    if(DEF_WIZARD_MODE)
    {
        printfColor(RED, "!! CAUTION DO NOT CLOSE THIS WINDOW !!\n");
        printf("Failing to follow the previous warning, could result in loss of soul \n");
        printf("==================================================================== \n \n");

        printf("Greetings mortal, I am the great MATH WIZARD!!!! \n");
        printf("By seeing this you have successfully signed the contract, in which should have appeared in your dreams!\n\n");

        printf("To summerize! You have been challenged too the PRIME GAME!!\n");
        printf("The rules are simple!\n");

        printf("- Every valid prime number adds that number dvided in half to the timer\n");
        printf("- For every {%i} that is a valid prime number, the speed decreases by 1\n", stats.maxNumbersBeforeSpeedDecrease);
        printf("- For every {%i} that is a invalid prime number or a repeated prime number, the speed increases\n\n", stats.maxNumbersBeforeSpeedIncrease);

        printf("That it! Good luck!\n");
        printf(" - GREAT MATH WIZARD!!!!\n\n");

        printf("PS: This will only delay the inevitability of me taking you soul, you can only loose eventually :3\n\n");
    }else
    {
        printf("== Rules ==");
        printf("- Every valid prime number adds that number dvided in half to the timer\n");
        printf("- For every {%i} that is a valid prime number, the speed decreases by 1\n", stats.maxNumbersBeforeSpeedDecrease);
        printf("- For every {%i} that is a invalid prime number or a repeated prime number, the speed increases\n\n", stats.maxNumbersBeforeSpeedIncrease);
    }

    // Wait for input and start game
    printfColor(YELLOW, "Press Enter to begin");
    std::cin.get();

    clearTerminal();
    gameLoop(stats);
}
