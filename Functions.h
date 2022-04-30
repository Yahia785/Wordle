/* NEW FILE CREATED FOR ALL FUNCTIONS DECLARATION
*BY YAHIA TAWFIK
*
*/
void GuessTheWord_ShowTitleScreen (Graphics_Context* g_sContext_p, HAL* hal_p);
void GuessTheWord_ShowCreateWordScreen(HAL* hal_p,Graphics_Context* g_sContext_p, Application* app_p );
void GuessTheWord_ShowGuessWordScreen(HAL* hal_p,Graphics_Context* g_sContext_p, Application* app_p);
void GuessTheWord_handleCreateWordScreen(HAL* hal_p, Application* app_p);
void WordMaster_handleTitleScreen (HAL* hal_p, Application* app_p);
void GetChar_word (Application* app_p, HAL* hal_p);
void GuessTheWord_UpdateCreateWordScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p);
void GuessTheWord_handleGuessWordScreen(HAL *hal_p, Application *app_p);
void DrawSquares1(HAL* hal_p, Graphics_Context* g_sContext_p, Application* app_p);
void DrawSquares2(HAL *hal_p, Graphics_Context *g_sContext_p, Application* app_p);
void DrawSquares3(HAL *hal_p, Graphics_Context *g_sContext_p, Application* app_p);
void DrawSquares4(HAL *hal_p, Graphics_Context *g_sContext_p, Application* app_p);
void DrawSquares5(HAL *hal_p, Graphics_Context *g_sContext_p, Application* app_p);
void GuessTheWord_UpdateGuessWordScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p);
void GuessTheWord_ShowEndGameScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p);
void Backspace_delete (HAL* hal_p, Application* app_p);
void GuessTheWord_HandleEndScreen(HAL* hal_p, Application* app_p, Graphics_Context* g_sContext_p);
void feedbackOnCharacters_loop(HAL *hal_p, Application *app_p);
void GuessTheWord_ShowWinScreen(HAL* hal_p, Application* app_p);
void GuessTheWord_ShowLoseScreen(HAL* hal_p, Application* app_p);
void Single_Player_Mode(HAL* hal_p, Application* app_p);
