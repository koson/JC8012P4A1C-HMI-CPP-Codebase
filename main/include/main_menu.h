#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the main menu screen with list of all demos
 */
void create_main_menu(void);

/**
 * @brief Return to main menu from any demo
 */
void return_to_main_menu(void);

#ifdef __cplusplus
}
#endif

#endif // MAIN_MENU_H
