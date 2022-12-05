/*!
	@file   lib_jmp.c
	@brief  <brief description here>
	@t.odo	-
	---------------------------------------------------------------------------

	MIT License
	Copyright (c) 2018 Ioannis Deligiannis | Devcoons Blog

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
/******************************************************************************
* Preprocessor Definitions & Macros
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/

#include "lib_jmp.h"
#ifdef LIB_JMP_ENABLED

/******************************************************************************
* Enumerations, structures & Variables
******************************************************************************/

/******************************************************************************
* Declaration | Static Functions
******************************************************************************/

/******************************************************************************
* Definition  | Static Functions
******************************************************************************/

__STATIC_INLINE void ISCB_DisableDCache (void)
{
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U) && defined(JMP_APPLICATION_USE_ISCB_DisableDCache)
	register uint32_t ccsidr;
	register uint32_t sets;
	register uint32_t ways;

	SCB->CSSELR = 0U; /*(0U << 1U) | 0U;*/  /* Level 1 data cache */
	__DSB();
	SCB->CCR &= ~(uint32_t)SCB_CCR_DC_Msk;  /* disable D-Cache */
	__DSB();
	ccsidr = SCB->CCSIDR;
	/* clean & invalidate D-Cache */
	sets = (uint32_t)(CCSIDR_SETS(ccsidr));
	do {
		ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
		do {
			SCB->DCCISW = (((sets << SCB_DCCISW_SET_Pos) & SCB_DCCISW_SET_Msk) |
			((ways << SCB_DCCISW_WAY_Pos) & SCB_DCCISW_WAY_Msk)  );
			#if defined ( __CC_ARM )
			__schedule_barrier();
			#endif
		} while (ways-- != 0U);
	} while(sets-- != 0U);

	__DSB();
	__ISB();
#endif
}

/******************************************************************************
* Definition  | Public Functions
******************************************************************************/

void jmp_goto_address(const uint32_t address)
{
	//HAL_IWDG_Refresh(&hiwdg);
	__disable_irq();
	for(int i = 0;i < 8;i++) 
		NVIC->ICER[i] = 0xFFFFFFFF;
    for(int i = 0;i < 8;i++) 
		NVIC->ICPR[i] = 0xFFFFFFFF;
    
	ISCB_DisableDCache();

	__set_CONTROL(0);
	HAL_DeInit();

	__set_MSP(*(__IO uint32_t*)address);
	uint32_t JumpAddress = *((volatile uint32_t*) (address + 4));
	__ISB();
	__DSB();

	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
	void (*reset_handler)(void) = (void*)JumpAddress;

	while(1)
		reset_handler();
}

#ifdef JMP_VALIDATE_PARTITION
i_status jmp_validate_partition(uint32_t address)
{
	if((*(__IO uint32_t*)address) == JMP_APPICATION_VALIDATION_VALUE)
	{
		#if !defined(JMP_DEBUG_MODE_AUTO_ENABLED) && !defined(JMP_DEBUG_MODE_MANUAL_ENABLED)

		uint8_t buffer[4];
		uint8_t buffer_sz = 0;
		uint32_t app_firmware_size = 0;
		uint16_t app_firmware_crc  = 0;

		if(ifm_area_is_valid(IFM_APPINFO) != I_OK)
			goto validate_app_partition_invalid;

		if(ifm_retrieve(IFM_APPINFO_FS,buffer,&buffer_sz) != I_OK)
			goto validate_app_partition_invalid;

		app_firmware_size = (buffer[0] << 27) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3] << 0);

		if(ifm_retrieve(IFM_APPINFO_FV,buffer,&buffer_sz) != I_OK)
			goto validate_app_partition_invalid;

		app_firmware_crc = (buffer[0] << 8) | (buffer[1] << 0);

		uint16_t calculated_app_crc = crc16_ccitt(0xffff, (uint8_t*)address, app_firmware_size);

		if(calculated_app_crc != app_firmware_crc)
			goto validate_app_partition_invalid;

		#endif
		return I_OK;
	}

	#if !defined(DEBUG_MODE_AUTO_ENABLED) && !defined(DEBUG_MODE_MANUAL_ENABLED)
	validate_app_partition_invalid:
	#endif
	return I_INVALID;
}
#endif

i_status jmp_validate_firmware(uint32_t address)
{
	if((*(__IO uint32_t*)address) == JMP_APPICATION_VALIDATION_VALUE)
		return I_OK;
	return I_INVALID;
}

/******************************************************************************
* EOF - NO CODE AFTER THIS LINE
******************************************************************************/
#endif
