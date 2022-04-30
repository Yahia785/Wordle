/*
 * Application.h
 *
 *  Created on: Dec 29, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <HAL/HAL.h>


enum _GameState
{
    TITLE_SCREEN, CREATE_WORD_SCREEN, GUESS_WORD_SCREEN, END_SCREEN
};

typedef enum _GameState GameState;

struct _Application
{
    // Put your application members and FSM state variables here!
    // =========================================================================
    UART_Baudrate baudChoice;
    bool firstCall;
    char RXchar;
    char P1word[6];
    int char_count_P1;
    char P2word[6];
    int char_count_P2;
    int guess_count;
    char BSword[6];
    int xcounter;
    int w;
    bool win;
    GameState state;
};
typedef struct _Application Application;

// Called only a single time - inside of main(), where the application is constructed
Application Application_construct();

// Called once per super-loop of the main application.
void Application_loop(Application* app, HAL* hal);

// Called whenever the UART module needs to be updated
void Application_updateCommunications(Application* app, HAL* hal);

// Interprets an incoming character and echoes back to terminal what kind of
// character was received (number, letter, or other)
char Application_interpretIncomingChar(char);

// Generic circular increment function
uint32_t CircularIncrement(uint32_t value, uint32_t maximum);

#endif /* APPLICATION_H_ */
