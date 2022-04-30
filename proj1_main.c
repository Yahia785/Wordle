/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* HAL and Application includes */
#include <Application.h>
#include <Functions.h> //New file which contains all functions declarations
#include <HAL/HAL.h>
#include <HAL/Timer.h>

#define MULTIPLYING_CONSTANT 10    //This constant is used in multiplying graphics coordinates

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void InitNonBlockingLED()
{
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void PollNonBlockingLED()
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}

/**
 * The main entry point of your project. The main function should immediately
 * stop the Watchdog timer, call the Application constructor, and then
 * repeatedly call the main super-loop function. The Application constructor
 * should be responsible for initializing all hardware components as well as all
 * other finite state machines you choose to use in this project.
 *
 * THIS FUNCTION IS ALREADY COMPLETE. Unless you want to temporarily experiment
 * with some behavior of a code snippet you may have, we DO NOT RECOMMEND
 * modifying this function in any way.
 */
int main(void)
{
    // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
    WDT_A_holdTimer();

    // Initialize the system clock and background hardware timer, used to enable
    // software timers to time their measurements properly.
    InitSystemTiming();

    // Initialize the main Application object and HAL object
    HAL hal = HAL_construct();
    Application app = Application_construct();

    // Do not remove this line. This is your non-blocking check.
    InitNonBlockingLED();

    GuessTheWord_ShowTitleScreen (&hal.g_sContext,&hal);

    // Main super-loop! In a polling architecture, this function should call
    // your main FSM function over and over.
    while (true)
    {
        // Do not remove this line. This is your non-blocking check.
        PollNonBlockingLED();
        HAL_refresh(&hal);
        Application_loop(&app, &hal);

    }
}

/**
 * A helper function which increments a value with a maximum. If incrementing
 * the number causes the value to hit its maximum, the number wraps around
 * to 0.
 */
uint32_t CircularIncrement(uint32_t value, uint32_t maximum)
{
    return (value + 1) % maximum;
}

/**
 * The main constructor for your application. This function should initialize
 * each of the FSMs which implement the application logic of your project.
 *
 * @return a completely initialized Application object
 */
Application Application_construct()
{
    Application app;

    // Initialize local application state variables here!
    app.baudChoice = BAUD_9600;
    app.firstCall = true;
    app.RXchar = 0;
    app.state = TITLE_SCREEN;
    int i;
    for ( i = 0; i < 6; i++)
    {
        app.P1word[i] = 0;
        app.P2word[i] = 0;
        app.BSword[i] = 0;
    }

    app.char_count_P1 = 0;
    app.char_count_P2 = 0;
    app.guess_count = 0;
    app.xcounter = 0;
    app.w = 0;
    app.win = false;
    return app;
}
//SWITCH STATEMENT
/*This function has a switch statement for four different states. TITLE_SCREEN is the
* first state where instructions and some information about the game are displayed.
* The CREATE_WORD_SCREEN is the state where player 1 enters their word. The
* GUESS_WORD_SCREEN is the state where player 2 tries to guess player 1's word
* END_SCREEN is the last state where the result is displayed on the screen.
*/

void GuessTheWord_loop(HAL* hal_p, Application* app_p )
{
    switch (app_p->state)
    {
        case TITLE_SCREEN:
            WordMaster_handleTitleScreen (hal_p, app_p);
            break;

        case CREATE_WORD_SCREEN:
            GuessTheWord_handleCreateWordScreen(hal_p, app_p);
            break;

        case GUESS_WORD_SCREEN:
            GuessTheWord_handleGuessWordScreen(hal_p, app_p);
            break;

        case END_SCREEN:
            GuessTheWord_HandleEndScreen(hal_p, app_p, &hal_p->g_sContext);
            break;

        default:
            break;
    }
}
/**
 * The main super-loop function of the application. We place this inside of a
 * single infinite loop in main. In this way, we can model a polling system of
 * FSMs. Every cycle of this loop function, we poll each of the FSMs one time,
 * followed by refreshing all inputs to the system through a convenient
 * [HAL_refresh()] call.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_loop(Application *app_p, HAL *hal_p)
{
    // Restart/Update communications if either this is the first time the application is
    // run or if BoosterPack S2 is pressed (which means a new baudrate is being set up)

    GuessTheWord_loop(hal_p, app_p);

    if (Button_isTapped(&hal_p->boosterpackS2) || app_p->firstCall)
    {
        Application_updateCommunications(app_p, hal_p);
    }

    GetChar_word (app_p, hal_p);
}

/**
 * Updates which LEDs are lit and what baud rate the UART module communicates
 * with, based on what the application's baud choice is at the time this
 * function is called.
 *
 * @param app_p:  A pointer to the main Application object.
 * @param hal_p:  A pointer to the main HAL object
 */
void Application_updateCommunications(Application* app_p, HAL* hal_p)
{
    // When this application first loops, the proper LEDs aren't lit. The
    // firstCall flag is used to ensure that the
    if (app_p->firstCall) {
        app_p->firstCall = false;
    }

    // When BoosterPack S2 is tapped, circularly increment which baud rate is used.
    else
    {
        uint32_t newBaudNumber = CircularIncrement((uint32_t) app_p->baudChoice, NUM_BAUD_CHOICES);
        app_p->baudChoice = (UART_Baudrate) newBaudNumber;
    }

    // Start/update the baud rate according to the one set above.
    UART_SetBaud_Enable(&hal_p->uart, app_p->baudChoice);

    // Based on the new application choice, turn on the correct LED.
    // To make your life easier, we recommend turning off all LEDs before
    // selectively turning back on only the LEDs that need to be relit.
    // -------------------------------------------------------------------------
    LED_turnOff(&hal_p->launchpadLED2Red);
    LED_turnOff(&hal_p->launchpadLED2Green);
    LED_turnOff(&hal_p->launchpadLED2Blue);

    // TODO: Turn on all appropriate LEDs according to the tasks below.
    switch (app_p->baudChoice)
    {
        // When the baud rate is 9600, turn on Launchpad LED Red
        case BAUD_9600:
            LED_turnOn(&hal_p->launchpadLED2Red);
            break;

        // TODO: When the baud rate is 19200, turn on Launchpad LED Green
        case BAUD_19200:
            LED_turnOn(&hal_p->launchpadLED2Green);
            break;

        // TODO: When the baud rate is 38400, turn on Launchpad LED Blue
        case BAUD_38400:
            LED_turnOn(&hal_p->launchpadLED2Blue);
            break;

        // TODO: When the baud rate is 57600, turn on all Launchpad LEDs (illuminates white)
        case BAUD_57600:
            LED_turnOn(&hal_p->launchpadLED2Red);
            LED_turnOn(&hal_p->launchpadLED2Green);
            LED_turnOn(&hal_p->launchpadLED2Blue);
            break;

        // In the default case, this program will do nothing.
        default:
            break;
    }
}

/**
 * Interprets a character which was incoming and returns an interpretation of
 * that character. If the input character is a letter, it return L for Letter, if a number
 * return N for Number, and if something else, it return O for Other.
 *
 * @param rxChar: Input character
 * @return :  Output character
 */
char Application_interpretIncomingChar(char rxChar)
{
    // The character to return back to sender. By default, we assume the letter
    // to send back is an 'O' (assume the character is an "other" character)
    char txChar = 'O';

    // Numbers - if the character entered was a number, transfer back an 'N'
    if (rxChar >= '0' && rxChar <= '9') {
        txChar = 'N';
    }

    // Letters - if the character entered was a letter, transfer back an 'L'
    if ((rxChar >= 'a' && rxChar <= 'z') || (rxChar >= 'A' && rxChar <= 'Z')) {
        txChar = 'L';
    }

    if (rxChar == '\b')
    {
        txChar = '\b';
    }
    return (txChar);
}

//This function displays the text needed for title screen
void GuessTheWord_ShowTitleScreen (Graphics_Context* g_sContext_p, HAL* hal_p)
{
    Graphics_clearDisplay(g_sContext_p);
    Graphics_drawString(g_sContext_p, (int8_t *)"S22 Project 1", -1, 1, 0, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"WordMaster", -1, 1, 10, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"By Yahia Tawfik", -1, 1, 20, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"==INSTRUCTIONS==", -1, 2, 35, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"Player 1 makes a", -1, 1, 45, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"word", -1, 1, 55, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"Player 2 guesses", -1, 1, 65, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"BB1:confirm word", -1, 1, 85, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"BB2:change baud-", -1, 1, 95, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"rate", -1, 1, 105, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"Type to start...", -1, 1, 115, true);

}

//This function handles the title screen logic. When a character is entered the title screen is automatically changed to
//create word screen which is the screen where player 1 enters their guess.
void WordMaster_handleTitleScreen (HAL* hal_p, Application* app_p)
{
    if(UART_hasChar(&hal_p->uart) == true)
    {
        app_p->state = CREATE_WORD_SCREEN;
        GuessTheWord_ShowCreateWordScreen(hal_p, &hal_p->g_sContext, app_p);
    }
}

//This function displays the text for create word screen, which is the screen where player 1 enters their word. It also
//prints the string responsible for player 1's word.
void GuessTheWord_ShowCreateWordScreen(HAL* hal_p,Graphics_Context* g_sContext_p, Application* app_p )
{
    Graphics_clearDisplay(g_sContext_p);
    Graphics_drawString(g_sContext_p, (int8_t *)"What's your word?", -1, 8, 0, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"-----", -1, 30, 20, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) app_p->P1word, 6, 30, 17, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"BB2:Adjust Baudrate", -1, 1, 40, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"9600:Red", -1, 1, 60, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"19200:Green", -1, 1, 70, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"37400:Blue", -1, 1, 80, true);
    Graphics_drawString(g_sContext_p, (int8_t *)"57600:White", -1, 1, 90, true);
}


// This function handles the create word screen which is the screen where player 1 enters their word. The string is complete
//when 5 characters are entered to the string. After that pressing the button changes the scree to guess word screen which
//is the screen where player 2 starts entering their guesses.
void GuessTheWord_handleCreateWordScreen(HAL *hal_p, Application *app_p)
{
    if (app_p->char_count_P1 < 5 && app_p->RXchar != 0)
    {
        app_p->BSword[app_p->char_count_P1] = app_p->RXchar;
        app_p->char_count_P1++;
        app_p->RXchar = 0;
    }
    Button_refresh(&hal_p->boosterpackS1);
    if (Button_isTapped(&hal_p->boosterpackS1) && app_p->char_count_P1 == 5)
    {
        app_p->state = GUESS_WORD_SCREEN;
        GuessTheWord_ShowGuessWordScreen(hal_p, &hal_p->g_sContext, app_p);
        int i;
        for (i = 0; i < 6; i++)
        {
            app_p->P1word[i] = app_p->BSword[i];
            app_p->BSword[i] = 0;
        }
        app_p->char_count_P1 =0;
    }
    else
    {
        //Stay on CREATE_WORD_SCREEN
        GuessTheWord_UpdateCreateWordScreen(hal_p, app_p, &hal_p->g_sContext);
    }
}

//This function shows the screen responsible for player two typing the guesses. It displays text only.
void GuessTheWord_ShowGuessWordScreen(HAL* hal_p,Graphics_Context* g_sContext_p, Application* app_p)
{
    Graphics_clearDisplay(g_sContext_p);
    Graphics_drawString(g_sContext_p, (int8_t *)"Guess the word!", -1, 15, 0, true);
}


//The function is responsible for getting characters typed and printing them. It also assigns the character to a
//char named RXchar which is assigned to strings printed on the screen.
void GetChar_word(Application *app_p, HAL *hal_p)
{
    if (UART_hasChar(&hal_p->uart))
    {
        // The character received from your serial terminal

        char rxChar = UART_getChar(&hal_p->uart);

        char txChar = Application_interpretIncomingChar(rxChar);

        // Only send a character if the UART module can send it
        if (UART_canSend(&hal_p->uart))
            UART_sendChar(&hal_p->uart, txChar);

        //printing text from UART on screen as uppercase letters only
        if (txChar == 'L')
        {
            if (rxChar >= 'a' && rxChar <= 'z')
            {
                rxChar = rxChar - 32;
            }
            app_p->RXchar = rxChar;
        }
        //Logic for backspace
        if (txChar == '\b')
        {
            if(app_p->char_count_P1 != 0)
            {
            app_p->char_count_P1--;
            app_p->BSword[app_p->char_count_P1] = '_';
            }
        }
    }
}

//This function updates the screen when player 1 is entering their word. Every time a character is entered
//it is updated to the string
void GuessTheWord_UpdateCreateWordScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p)
{
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) app_p->BSword, 6, 30, 17, true);
}

//This function updates the screen when new guess is typed. It is responsible for printing the new guess under the
//old guess
void GuessTheWord_UpdateGuessWordScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p)
{
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) app_p->BSword, 6, 0, 12 + (app_p->guess_count * MULTIPLYING_CONSTANT), true);
}

//This function handles the logic for the guess screen. It takes characters into a string and once the
//string has 5 characters the string is complete and if button 1 is pressed the guess is submitted and
//the squares are drawn. If 6 guesses are submitted the state is switched to end screen and end screen
//is displayed
void GuessTheWord_handleGuessWordScreen(HAL *hal_p, Application *app_p)
{
    if (app_p->char_count_P1 < 5 && app_p->RXchar != 0)
    {
        app_p->BSword[app_p->char_count_P1] = app_p->RXchar;
        app_p->char_count_P1++;
        app_p->RXchar = 0;
    }
    Button_refresh(&hal_p->boosterpackS1);
    if (Button_isTapped(&hal_p->boosterpackS1) && app_p->char_count_P1 == 5)
    {
        int x;
        for (x = 0; x < 6; x++)
        {
            app_p->P2word[x] = app_p->BSword[x];
        }
        feedbackOnCharacters_loop(hal_p, app_p);
        app_p->guess_count++;
        int i;
        for (i = 0; i < 6; i++)
        {
            app_p->BSword[i] = 0;
        }
        app_p->char_count_P1 = 0;

        if (app_p->guess_count == 6 && app_p->w != 5)
        {
            app_p->state = END_SCREEN;
            GuessTheWord_ShowLoseScreen(hal_p, app_p);
        }

    }
    else
    {
        //Stay on GUESS_WORD_SCREEN
        GuessTheWord_UpdateGuessWordScreen(hal_p, app_p, &hal_p->g_sContext);
    }
}

//This function initializes the coordinates of the squares and has two functions that draws and fills
//the squares. Calling this function draws a square. When the number of guesses
//increases the coordinates of the "y" variable is multiplied by a constant to increase the coordinate
//of the "y" variable and causes other 5 squares to be drawn on the next line beside the next guess.
//There is also a counter for the x axis that increments every time a square is drawn. This counter
//is then multiplied by a constant to increase the x coordinates. When x counter reaches 5 it is reset to zero.
void DrawSquares1(HAL *hal_p, Graphics_Context *g_sContext_p, Application* app_p)
{
    Graphics_Rectangle R;
    R.xMin = 50 + (app_p->xcounter * MULTIPLYING_CONSTANT);
    R.xMax = 58 + (app_p->xcounter * MULTIPLYING_CONSTANT);
    R.yMin = 12 + (app_p->guess_count * MULTIPLYING_CONSTANT);
    R.yMax = 20 + (app_p->guess_count * MULTIPLYING_CONSTANT);
    Graphics_drawRectangle(g_sContext_p, &R);
    Graphics_fillRectangle(g_sContext_p, &R);
}

//This function handles the end screen. When the result is displayed pressing the button restarts the game to title screen
//and resets all variables.
void GuessTheWord_HandleEndScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p)
{
    if (Button_isTapped(&hal_p->boosterpackS1))
    {
        app_p->state = TITLE_SCREEN;
        GuessTheWord_ShowTitleScreen(g_sContext_p, hal_p);
        app_p->char_count_P1 = 0;
        app_p->char_count_P2 = 0;
        app_p->guess_count = 0;
        app_p->w = 0;
        app_p->xcounter = 0;

        int i;
        for (i = 0; i < 6; i++)
        {
            app_p->P1word[i] = 0;
            app_p->P2word[i] = 0;
            app_p->BSword[i] = 0;
        }

    }
}

/* This function has switch statement for feedback on player 2's guess. If the letter entered by
 * player 2 is in player 1's word and in correct position green square is drawn on screen. If
 * letter entered by player 2 is present in player 1's word but not in correct position a yellow
 * square is drawn on the screen. If the letter entered by player 2 is not present in player 1's
/* word a gray square is drawn on screen. The variable "w" counts how many times green squares.
 * If W is equal to five that means player 2 wins.
 */
void feedbackOnCharacters_loop(HAL *hal_p, Application *app_p)
{
    _Bool gray;
   app_p->xcounter = 0;
    app_p->w = 0;
    for (app_p->char_count_P2 = 0; app_p->char_count_P2 < 5;
            app_p->char_count_P2++)
    {
        gray = true;
        if (app_p->P2word[app_p->char_count_P2]
                == app_p->P1word[app_p->char_count_P2])
        {
            Graphics_setForegroundColor(&hal_p->g_sContext,
            GRAPHICS_COLOR_GREEN);
            DrawSquares1(hal_p, &hal_p->g_sContext, app_p);
            Graphics_setForegroundColor(&hal_p->g_sContext,
            GRAPHICS_COLOR_WHITE);
            app_p->xcounter++;
            app_p->w++;
            continue;
        }
        for (app_p->char_count_P1 = 0; app_p->char_count_P1 < 5;
                app_p->char_count_P1++)
        {
            if (app_p->P2word[app_p->char_count_P2]
                    == app_p->P1word[app_p->char_count_P1])
            {
                Graphics_setForegroundColor(&hal_p->g_sContext,
                GRAPHICS_COLOR_YELLOW);
                DrawSquares1(hal_p, &hal_p->g_sContext, app_p);
                Graphics_setForegroundColor(&hal_p->g_sContext,
                GRAPHICS_COLOR_WHITE);
                gray = false;
                break;
            }
        }
        if (gray == true)
        {
            Graphics_setForegroundColor(&hal_p->g_sContext,
            GRAPHICS_COLOR_GRAY);
            DrawSquares1(hal_p, &hal_p->g_sContext, app_p);
            Graphics_setForegroundColor(&hal_p->g_sContext,
            GRAPHICS_COLOR_WHITE);
        }
        app_p->xcounter++;

    }
    if (app_p->w == 5)
    {
        app_p->state = END_SCREEN;
        GuessTheWord_ShowWinScreen(hal_p, app_p);
        app_p->xcounter = 0;
    }
}

//This function is the graphics display for the win screen when player 2 guesses the
//word correctly
void GuessTheWord_ShowWinScreen(HAL* hal_p, Application* app_p)
{
     Graphics_drawString(&hal_p->g_sContext, (int8_t *)"The word was:", -1, 0, 75, true);
     Graphics_drawString(&hal_p->g_sContext, (int8_t *) app_p->P1word, -1, 80, 75, true);
     Graphics_drawString(&hal_p->g_sContext, (int8_t *) "Player 2 wins!", -1, 0, 85, true);
     Graphics_drawString(&hal_p->g_sContext, (int8_t *)"Press button 1 to", -1, 0, 110, true);
     Graphics_drawString(&hal_p->g_sContext, (int8_t *)"restart", -1, 0, 120, true);
}

//This function is the graphics display for the lose screen when player 2 runs out of guesses
//and doesn't guess the word correctly.
void GuessTheWord_ShowLoseScreen(HAL* hal_p, Application* app_p)
{
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)"Game over", -1, 0, 75, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)"Player 1 wins!", -1, 0, 85, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)"Press button 1 to", -1, 0, 110, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t *)"restart", -1, 0, 120, true);
}












