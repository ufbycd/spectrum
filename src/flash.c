#include "STM32Lib\\stm32f10x.h"
#include "hal.h" 
#define                PAGE_ADDR                                (0x08000000 + 60 * 1024)


/***************************************************************************************************
* ????: MemReadByte()
* ????: ???????num????
* ????: *dat:?????????
*                        num        :??????
* ????: 0:????;1:????
* ????: ?
* ????: 2010?12?15?
***************************************************************************************************/
u8 MemReadByte(u16 address)                                
{
     u16 data;   
	  u16 *temp_addr = (u16 *)(PAGE_ADDR+address);
             
        data  = *temp_addr;
        
                                                                                                        
        return data;                                                                                                        
}

/***************************************************************************************************
* ????: MemWriteByte()
* ????: ???????num????
* ????: *dat:?????
*                        num        :??????
* ????: 0:????;1:????
* ????: ?
* ????: 2010?12?15?
***************************************************************************************************/
u8 MemWriteByte(u16 address,u16 data)                                
{
        FLASH_Status temp_stat;
        u32 temp_addr = PAGE_ADDR+address;
                
        FLASH_Unlock();                                                                                                        // Flash??,??????????
        temp_stat = FLASH_ErasePage(PAGE_ADDR+address);                                                        // ??????
        
        if(temp_stat != FLASH_COMPLETE)
        {
                FLASH_Lock();
                return 0;
        }
        
       
                temp_stat = FLASH_ProgramHalfWord(temp_addr,data);
                if(temp_stat != FLASH_COMPLETE)
                {
                        FLASH_Lock();
                        return 0;
                }
        
               // temp_addr += 2;
                //data++;
        
        
        FLASH_Lock();        
        return 1;
}
