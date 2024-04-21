#ifndef ABYSS_BUTTON_H
#define ABYSS_BUTTON_H

typedef struct Button Button;

typedef enum ButtonToolTip {
    BUTTON_TOOL_TIP_NONE,
    BUTTON_TOOL_TIP_CLOSE,
    BUTTON_TOOL_TIP_OK,
    BUTTON_TOOL_TIP_BUY,
    BUTTON_TOOL_TIP_SELL,
    BUTTON_TOOL_TIP_REPAIR,
    BUTTON_TOOL_TIP_REPAIR_ALL,
    BUTTON_TOOL_TIP_LEFT_ARROW,
    BUTTON_TOOL_TIP_RIGHT_ARROW,
    BUTTON_TOOL_TIP_QUERY,
    BUTTON_TOOL_TIP_SQUELCH_CHAT
} ButtonToolTip;

typedef enum ButtonType {
    BUTTON_TYPE_WIDE,
    BUTTON_TYPE_MEDIUM,
    BUTTON_TYPE_NARROW,
    BUTTON_TYPE_CANCEL,
    BUTTON_TYPE_TALL,
    BUTTON_TYPE_SHORT,
    BUTTON_TYPE_OK_CANCEL,
} ButtonType;

Button *Button_Create(ButtonType button_type, const char *text);
void    Button_Destroy(Button **button);

#endif // ABYSS_BUTTON_H
