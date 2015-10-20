/* include header files */

#include <stdlib.h>
#include "common.h"
#include <string>
#include "cmd_dispatch.h"

/* this file is cmd dispatcher 
 send one cmd through UART prompt to exec a cmd
*/

#define TM_CMD_LEN 10

/* dispatcher body */

void cmd_dispatcher(void *pvParameters)
{
  UART_PRINT("Test Mode Entered, Get TM CMD in Dispatcher!\n");
  TM_CMD_type cmd = INVALID_CMD;
  cmd = get_tm_cmd();
  switch(cmd)
  {
    case TFTP:
      UART_PRINT("Enter TFTP\n");
      break;
    case CAMERA:
          UART_PRINT("Enter CAMERA\n");
      break;
    default:
          UART_PRINT("CMD is not valid, please check input");
      break;
  }
}

TM_CMD_type get_tm_cmd(void)
{
  TM_CMD_type cmd = INVALID_CMD;
  bool status = true;
  uint8 cmd_num = 0xFF;
  char* cmd_str[TM_CMD_LEN];
  do{
    status = read_UART(cmd_str);
    if(status)
    {
      cmd_num = atoi(cmd_str);
    }
  }while(cmd_num >= TM_CMD_MAX);
  cmd = (tm_cmd)cmd_num;
  return cmd;
}
