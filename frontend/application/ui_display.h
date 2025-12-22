/******************************************************************************
 * File: ui_display.h
 * Module: UI Display (Application Layer)
 * Description: LCD display helper functions
 ******************************************************************************/

#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include <stdint.h>

/**
 * @brief Display a message on the LCD (2 lines)
 * @param line1 First line text (or NULL)
 * @param line2 Second line text (or NULL)
 */
void showMessage(const char *line1, const char *line2);

#endif /* UI_DISPLAY_H */
