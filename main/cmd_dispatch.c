/* include header files */

#include <stdlib.h>
#include "common.h"
#include <string>

/* this file is cmd dispatcher 
 send one cmd through UART prompt to exec a cmd
*/

#define TM_CMD_LEN 10

enum tm_cmd{
  TFTP,
  CAMERA,
  TM_CMD_MAX,
  INVALID_CMD = 0xFF
};

/* dispatcher body */

void cmd_dispatcher(void *pvParameters)
{
  UART_PRINT("Test Mode Entered, Get TM CMD in Dispatcher!\n");
  tm_cmd cmd = INVALID_CMD;
  cmd = get_tm_cmd();
  switch(cmd)
  {
    case TFTP:
      
      break;
    case CAMERA:
      break;
    default:
      break;
  }
}

tm_cmd get_tm_cmd(void)
{
  tm_cmd cmd = INVALID_CMD;
  bool status = true;
  uint8 cmd_num = 0xFF;
  char* cmd_str[TM_CMD_LEN];
  do{
    status = read_UART(cmd_str);
    if(status)
    {
      cmd_num = str_to_char(cmd_str);
    }
  }while(cmd_num >= TM_CMD_MAX);
  cmd = (tm_cmd)cmd_num;
  return cmd;
}
