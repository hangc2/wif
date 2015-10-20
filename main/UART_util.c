/* UART_util provides utility for UART interaction
*/

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "UART_util.h"


bool read_UART(char* cmd_str, uint8 cmd_len , const char* screen_print)
{
  int16 ret_val = 0;
  bool received = false;
  if(cmd_str == NULL)
  {
    UART_PRINT("read_UART failed, cmd_str is NULL\n");
    return false;
  }

  if(cmd_len == 0)
  {
    UART_PRINT("read_UART failed, cmd_len is 0\n");
    return false;
  }
  
  do
  { 
    if(cmd_str == NULL)
    {
      UART_PRINT("\nScreen Print is empty, Please type#");
    }
    else
    {
      UART_PRINT("\n%s#", screen_print);
    }
    ret_val = GetCmd(cmd_str, cmd_len);
    if(ret_val > 0)
    {
      ret_val = TrimSpace(cmd_str);
      if(cmd_str!=NULL)
      {
        cmd_str[ret_val] = '\0'a
	received = true;
      }
    }
  }while(!received)
  return received;
}



