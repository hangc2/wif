#ifdef __cplusplus
extern "C"
{
#endif

#include "uart_if.h"

typedef enum TM_CMD_type{
    TFTP,
    CAMERA,
    TM_CMD_MAX,
    INVALID_CMD = 0xFF
}TM_CMD_type;

TM_CMD_type get_tm_cmd(void);
void cmd_dispatcher(void *pvParameters);

#ifdef __cplusplus
}
#endif