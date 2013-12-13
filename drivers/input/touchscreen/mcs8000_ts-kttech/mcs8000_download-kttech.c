//--------------------------------------------------------
//
//
//	Melfas MCS8000 Series Download base v1.0 2010.04.05
//
//
//--------------------------------------------------------

/* Archicture definitions */
#include <asm/delay.h>
#include <asm/atomic.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/platform_device.h>

#include <linux/init.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/major.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/delay.h>

#include <linux/reboot.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/firmware.h>

/* Machine definitions */
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/pmic.h>
#include <mach/board.h>

#include "mcs8000_download-kttech.h"

//============================================================
//
//	Include MELFAS Binary code File ( ex> MELFAS_FIRM_bin.c)
//
//	Warning!!!!
//		Please, don't add binary.c file into project
//		Just #include here !!
//
//============================================================

#include "mcs8000_ts-fw_ra01vb15_o6.c"
#include "mcs8000_ts-fw_ra00va01_o6_legacy.c"

UINT8 ucVerifyBuffer[MELFAS_TRANSFER_LENGTH]; //	You may melloc *ucVerifyBuffer instead of this


//---------------------------------
//	Downloading functions
//---------------------------------
static int	mcsdl_download(const UINT8 *pData, const UINT16 nLength, INT8 IdxNum);

static void mcsdl_set_ready(void);
static void mcsdl_reboot_mcs(void);

static int	mcsdl_erase_flash(INT8 IdxNum);
static int	mcsdl_program_flash(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum);
static void mcsdl_program_flash_part(UINT8 *pData);

static int	mcsdl_verify_flash(UINT8 *pData, UINT16 nLength, INT8 IdxNum);

static void mcsdl_read_flash(UINT8 *pBuffer);
static int	mcsdl_read_flash_from(UINT8 *pBuffer, UINT16 unStart_addr, UINT16 unLength, INT8 IdxNum);

static void mcsdl_select_isp_mode(UINT8 ucMode);
static void mcsdl_unselect_isp_mode(void);

static void mcsdl_read_32bits(UINT8 *pData);
static void mcsdl_write_bits(UINT32 wordData, int nBits);
#if 0
static void mcsdl_scl_toggle_twice(void);
#endif

//---------------------------------
//	Delay functions
//---------------------------------
static void mcsdl_delay(UINT32 nCount);

//---------------------------------
//	For debugging display
//---------------------------------
#if MELFAS_ENABLE_DBG_PRINT
static void mcsdl_print_result(int nRet);
#endif

#if MCSDL_USE_VDD_CONTROL
struct melfas_ts_platform_data *mcsdl_pdata;

void mcsdl_vdd_on(void)
{
	regulator_enable(mcsdl_pdata->reg_lvs3);
	regulator_enable(mcsdl_pdata->reg_l4);
	regulator_enable(mcsdl_pdata->reg_mvs0);
	mdelay(100);
}

void mcsdl_vdd_off(void)
{
	regulator_disable(mcsdl_pdata->reg_mvs0);
	regulator_disable(mcsdl_pdata->reg_l4);
	regulator_disable(mcsdl_pdata->reg_lvs3);
	mdelay(100);	
}

void mcsdl_init_pdata(struct melfas_ts_platform_data *pdata) {
	mcsdl_pdata = pdata;
}
#endif

//----------------------------------
// Download enable command
//----------------------------------
#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD

void melfas_send_download_enable_command(void)
{
	// TO DO : Fill this up

}

#endif

//============================================================
//
//	Main Download furnction
//
//   1. Run mcsdl_download( pBinary[IdxNum], nBinary_length[IdxNum], IdxNum);
//       IdxNum : 0 (Master Chip Download)
//       IdxNum : 1 (2Chip Download)
//
//
//============================================================

int mcsdl_download_binary_data(void)
{
	int nRet;
#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR(); // Disable Baseband touch interrupt ISR.
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET(); // Disable Baseband watchdog timer

	//------------------------
	// Run Download
	//------------------------
	nRet = mcsdl_download((const UINT8*) MELFAS_binary,
			(const UINT16) MELFAS_binary_nLength, 0);
#if MELFAS_2CHIP_DOWNLOAD_ENABLE
	nRet = mcsdl_download( (const UINT8*) MELFAS_binary_2, (const UINT16)MELFAS_binary_nLength_2, 1); // Slave Binary data download
#endif
	MELFAS_ROLLBACK_BASEBAND_ISR(); // Roll-back Baseband touch interrupt ISR.
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET(); // Roll-back Baseband watchdog timer

	return (nRet);
}


int mcsdl_download_binary_legacy_data(void)
{
	int nRet;
#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR(); // Disable Baseband touch interrupt ISR.
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET(); // Disable Baseband watchdog timer

	//------------------------
	// Run Download
	//------------------------
	nRet = mcsdl_download((const UINT8*) MELFAS_binary_Legacy,
			(const UINT16) MELFAS_binary_nLength_Legacy, 0);

	MELFAS_ROLLBACK_BASEBAND_ISR(); // Roll-back Baseband touch interrupt ISR.
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET(); // Roll-back Baseband watchdog timer

	return (nRet);
}

int mcsdl_download_binary_file(struct device *dev)
{
	int nRet;

	const struct firmware *fw = NULL;

	nRet = request_firmware(&fw, MCS8000_FIRMWARE, dev);

	if (nRet < 0) {
		dev_err(dev, "[TSP] Unable to open firmware %s\n", MCS8000_FIRMWARE);		
		return -ENOMEM;
	}

#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR(); // Disable Baseband touch interrupt ISR.
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET(); // Disable Baseband watchdog timer

	// Start Firmware download of etc->firmware
	nRet = mcsdl_download(fw->data,	(const UINT16)fw->size, 0);

	MELFAS_ROLLBACK_BASEBAND_ISR(); // Roll-back Baseband touch interrupt ISR.
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET(); // Roll-back Baseband watchdog timer

#if MELFAS_ENABLE_DBG_PRINT
	mcsdl_print_result( nRet );
#endif

	return (nRet == MCSDL_RET_SUCCESS);
}

//------------------------------------------------------------------
//
//	Download function
//
//------------------------------------------------------------------

static int mcsdl_download(const UINT8 *pBianry, const UINT16 unLength,
		INT8 IdxNum)
{
	int nRet;

	//---------------------------------
	// Check Binary Size
	//---------------------------------
	if (unLength > MELFAS_FIRMWARE_MAX_SIZE)
	{

		nRet = MCSDL_RET_PROGRAM_SIZE_IS_WRONG;
		goto MCSDL_DOWNLOAD_FINISH;
	}

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n - Start a new TSP Firmware download...\n");
#endif

	//---------------------------------
	// Make it ready
	//---------------------------------
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Ready!\n");
#endif

	mcsdl_set_ready();

	//---------------------------------
	// Erase Flash
	//---------------------------------
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Erase...\n");
#endif

//	preempt_disable();
	nRet = mcsdl_erase_flash(IdxNum);
//	preempt_enable();

	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	//---------------------------------
	// Program Flash
	//---------------------------------
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Program...   ");
#endif

//	preempt_disable();
	nRet = mcsdl_program_flash((UINT8*) pBianry, (UINT16) unLength, IdxNum);
//	preempt_enable();
	
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;
	//---------------------------------
	// Verify flash
	//---------------------------------

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Verify...   ");
#endif

//	preempt_disable();
	nRet = mcsdl_verify_flash((UINT8*) pBianry, (UINT16) unLength, IdxNum);
//	preempt_enable();

	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	nRet = MCSDL_RET_SUCCESS;

	MCSDL_DOWNLOAD_FINISH:

#if MELFAS_ENABLE_DBG_PRINT
	mcsdl_print_result( nRet ); // Show result
#endif

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Rebooting...\n");
	printk(" - Is done...\n");
#endif

	mcsdl_reboot_mcs();

	return nRet;
}

//------------------------------------------------------------------
//
//	Sub functions
//
//------------------------------------------------------------------

static int mcsdl_erase_flash(INT8 IdxNum)
{
	int i;
	UINT8 readBuffer[32];

	//----------------------------------------
	//	Do erase
	//----------------------------------------
	if (IdxNum > 0)
		mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);

	mcsdl_select_isp_mode(ISP_MODE_ERASE_FLASH);
	mcsdl_unselect_isp_mode();

	//----------------------------------------
	//	Check 'erased well'
	//----------------------------------------
	// start ADD DELAY
	mcsdl_delay(MCSDL_DELAY_40US);
	//end ADD DELAY 	   
	
	mcsdl_read_flash_from(readBuffer, 0x0000, 16, IdxNum);
	mcsdl_read_flash_from(&readBuffer[16], 0x7FF0, 16, IdxNum);

	//start ADD DELAY	   
	mcsdl_delay(MCSDL_DELAY_5MS);
	//end ADD DELAY

	// Compare with '0xFF'
	for (i = 0; i < 32; i++)
	{
		if (readBuffer[i] != 0xFF)
			return MCSDL_RET_ERASE_FLASH_VERIFY_FAILED;
	}

	return MCSDL_RET_SUCCESS;
}

static int mcsdl_program_flash(UINT8 *pDataOriginal, UINT16 unLength,
		INT8 IdxNum)
{
	int i;

	UINT8 *pData;
	UINT8 ucLength;

	UINT16 addr;
	UINT32 header;

	addr = 0;
	pData = pDataOriginal;

	ucLength = MELFAS_TRANSFER_LENGTH;

	while ((addr * 4) < (int) unLength)
	{

		if ((unLength - (addr * 4)) < MELFAS_TRANSFER_LENGTH)
		{
			ucLength = (UINT8)(unLength - (addr * 4));
		}

		//--------------------------------------
		//	Select ISP Mode
		//--------------------------------------

		// start ADD DELAY
		mcsdl_delay(MCSDL_DELAY_40US);
		//end ADD DELAY        
		if (IdxNum > 0)
			mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);

		mcsdl_select_isp_mode(ISP_MODE_SERIAL_WRITE);

		//---------------------------------------------
		//	Header
		//	Address[13ibts] <<1
		//---------------------------------------------
		header = ((addr & 0x1FFF) << 1) | 0x0;
		header = header << 14;

		//Write 18bits
		mcsdl_write_bits(header, 18);
		//start ADD DELAY      
		mcsdl_delay(MCSDL_DELAY_5MS);
		//end ADD DELAY

		//---------------------------------
		//	Writing
		//---------------------------------
		//		addr += (UINT16)ucLength;
		addr += 1;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
		printk("#");
#endif

		mcsdl_program_flash_part(pData);

		pData += ucLength;

		//---------------------------------------------
		//	Tail
		//---------------------------------------------
		MCSDL_GPIO_SDA_SET_HIGH();
		//kang
		mcsdl_delay(MCSDL_DELAY_40US);

		for (i = 0; i < 6; i++)
		{

			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_20US);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_40US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_10US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_10US);
		}
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
//		printk("\n");
#endif

		mcsdl_unselect_isp_mode();
		//start ADD DELAY
		mcsdl_delay(MCSDL_DELAY_300US);
		//end ADD DELAY
	}

	return MCSDL_RET_SUCCESS;
}

static void mcsdl_program_flash_part(UINT8 *pData)
{
	UINT32 data;

	//---------------------------------
	//	Body
	//---------------------------------

	data = (UINT32) pData[0] << 0;
	data |= (UINT32) pData[1] << 8;
	data |= (UINT32) pData[2] << 16;
	data |= (UINT32) pData[3] << 24;
	mcsdl_write_bits(data, 32);
}

static int mcsdl_verify_flash(UINT8 *pDataOriginal, UINT16 unLength,
		INT8 IdxNum)
{
	int i, j;
	int nRet;

	UINT8 *pData;
	UINT8 ucLength;

	UINT16 addr;
	UINT32 wordData;

	addr = 0;
	pData = (UINT8 *) pDataOriginal;

	ucLength = MELFAS_TRANSFER_LENGTH;

	while ((addr * 4) < (int) unLength)
	{

		if ((unLength - (addr * 4)) < MELFAS_TRANSFER_LENGTH)
		{
			ucLength = (UINT8)(unLength - (addr * 4));
		}

		// start ADD DELAY        
		mcsdl_delay(MCSDL_DELAY_40US);

		//--------------------------------------
		//	Select ISP Mode
		//--------------------------------------
		if (IdxNum > 0)
			mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);
		mcsdl_select_isp_mode(ISP_MODE_SERIAL_READ);

		//---------------------------------------------
		//	Header
		//	Address[13ibts] <<1
		//---------------------------------------------

		wordData = ((addr & 0x1FFF) << 1) | 0x0;
		wordData <<= 14;

		mcsdl_write_bits(wordData, 18);

		addr += 1;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
		printk("#");
#endif
		//--------------------
		// Read flash
		//--------------------
		mcsdl_read_flash(ucVerifyBuffer);

		MCSDL_GPIO_SDA_SET_HIGH();

		for (i = 0; i < 6; i++)
		{

			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_3US);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_40US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_10US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_10US);
		}
		//kang
		//--------------------
		// Comparing
		//--------------------

		for (j = 0; j < (int) ucLength; j++)
		{

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
			printk(" %02X", ucVerifyBuffer[j] );
#endif

			if (ucVerifyBuffer[j] != pData[j])
			{

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
				printk("\n [Error] Address : 0x%04X : 0x%02X - 0x%02X\n", addr, pData[j], ucVerifyBuffer[j] );
#endif

				nRet = MCSDL_RET_PROGRAM_VERIFY_FAILED;
				goto MCSDL_VERIFY_FLASH_FINISH;

			}
		}

		pData += ucLength;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
//		printk("\n");
#endif

		mcsdl_unselect_isp_mode();
	}

	nRet = MCSDL_RET_SUCCESS;

	MCSDL_VERIFY_FLASH_FINISH:

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n");
#endif

	mcsdl_unselect_isp_mode();

	return nRet;
}

static void mcsdl_read_flash(UINT8 *pBuffer)
{
	int i;

	preempt_disable();

	MCSDL_GPIO_SDA_SET_LOW();

	mcsdl_delay(MCSDL_DELAY_40US);

	for (i = 0; i < 5; i++)
	{
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_10US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_10US);
	}
	preempt_enable();

	mcsdl_read_32bits(pBuffer);
}

static int mcsdl_read_flash_from(UINT8 *pBuffer, UINT16 unStart_addr,
		UINT16 unLength, INT8 IdxNum)
{
	int i;
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
	int j;
#endif

	UINT8 ucLength;

	UINT16 addr;
	UINT32 wordData;

	if (unLength >= MELFAS_FIRMWARE_MAX_SIZE)
	{
		return MCSDL_RET_PROGRAM_SIZE_IS_WRONG;
	}

	addr = 0;
	ucLength = MELFAS_TRANSFER_LENGTH;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
	printk(" %04X : ", unStart_addr );
#endif

	for (i = 0; i < (int) unLength; i += (int) ucLength)
	{

		addr = (UINT16) i;
		if (IdxNum > 0)
			mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);

		mcsdl_select_isp_mode(ISP_MODE_SERIAL_READ);
		wordData = (((unStart_addr + addr) & 0x1FFF) << 1) | 0x0;
		wordData <<= 14;

		mcsdl_write_bits(wordData, 18);

		if ((unLength - addr) < MELFAS_TRANSFER_LENGTH)
		{

			ucLength = (UINT8)(unLength - addr);
		}

		//--------------------
		// Read flash
		//--------------------
		mcsdl_read_flash(&pBuffer[addr]);

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
		for(j=0; j<(int)ucLength; j++)
		{
			printk("%02X ", pBuffer[j] );
		}
#endif

		mcsdl_unselect_isp_mode();

	}

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT_LOW
	printk("\n");
#endif

	return MCSDL_RET_SUCCESS;

}

static void mcsdl_set_ready(void)
{
	//--------------------------------------------
	// Tkey module reset
	//--------------------------------------------
	MCSDL_VDD_SET_LOW(); // power 

	preempt_disable();

	MCSDL_GPIO_SDA_SET_OUTPUT();
	MCSDL_GPIO_SDA_SET_LOW();

	MCSDL_GPIO_SCL_SET_OUTPUT();
	MCSDL_GPIO_SCL_SET_LOW();

	MCSDL_RESETB_SET_OUTPUT();
	MCSDL_RESETB_SET_LOW();

	preempt_enable();

	mcsdl_delay(MCSDL_DELAY_30MS); // Delay for Stable VDD

	MCSDL_VDD_SET_HIGH();

	MCSDL_GPIO_SDA_SET_HIGH();

	mcsdl_delay(MCSDL_DELAY_30MS); // Delay '25 msec'
}

static void mcsdl_reboot_mcs(void)
{
	//--------------------------------------------
	// Tkey module reset
	//--------------------------------------------

	MCSDL_VDD_SET_LOW();

	preempt_disable();

	MCSDL_GPIO_SDA_SET_OUTPUT();
	MCSDL_GPIO_SDA_SET_HIGH();

	MCSDL_GPIO_SCL_SET_OUTPUT();
	MCSDL_GPIO_SCL_SET_HIGH();

	MCSDL_RESETB_SET_OUTPUT();
	MCSDL_RESETB_SET_LOW();

	preempt_enable();

	mcsdl_delay(MCSDL_DELAY_25MS); // Delay for Stable VDD

	MCSDL_RESETB_SET_INPUT();
	MCSDL_VDD_SET_HIGH();

	MCSDL_RESETB_SET_HIGH();

	mcsdl_delay(MCSDL_DELAY_30MS); // Delay '25 msec'
	
}

//--------------------------------------------
//
//   Write ISP Mode entering signal
//
//--------------------------------------------

static void mcsdl_select_isp_mode(UINT8 ucMode)
{
	int i;

	UINT8 enteringCodeMassErase[16] =
	{ 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1 };
	UINT8 enteringCodeSerialWrite[16] =
	{ 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1 };
	UINT8 enteringCodeSerialRead[16] =
	{ 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 };
	UINT8 enteringCodeNextChipBypass[16] =
	{ 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 };
	UINT8 enteringCodeNonCode[16] = 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	UINT8 *pCode;

	preempt_disable();

	//------------------------------------
	// Entering ISP mode : Part 1
	//------------------------------------

	if (ucMode == ISP_MODE_ERASE_FLASH)
		pCode = enteringCodeMassErase;
	else if (ucMode == ISP_MODE_SERIAL_WRITE)
		pCode = enteringCodeSerialWrite;
	else if (ucMode == ISP_MODE_SERIAL_READ)
		pCode = enteringCodeSerialRead;
	else if (ucMode == ISP_MODE_NEXT_CHIP_BYPASS)
		pCode = enteringCodeNextChipBypass;
	else
		pCode = enteringCodeNonCode;

	for (i = 0; i < 16; i++)
	{

		if (pCode[i] == 1)
			MCSDL_RESETB_SET_HIGH();
		else
			MCSDL_RESETB_SET_LOW();

		//start add delay for INT
		mcsdl_delay(MCSDL_DELAY_7US);
		//end delay for INT

		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_3US);
	}

	MCSDL_RESETB_SET_HIGH(); // High

	//---------------------------------------------------
	// Entering ISP mode : Part 2	- Only Mass Erase
	//---------------------------------------------------

	if (ucMode == ISP_MODE_ERASE_FLASH)
	{
		mcsdl_delay(MCSDL_DELAY_7US);
		for (i = 0; i < 4; i++)
		{

			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_25MS);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_150US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_3US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_7US);
		}
	}

	preempt_enable();
}

static void mcsdl_unselect_isp_mode(void)
{
	int i;

	preempt_disable();

	MCSDL_RESETB_SET_LOW();

	mcsdl_delay(MCSDL_DELAY_3US);

	for (i = 0; i < 10; i++)
	{

		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_3US);
	}

	preempt_enable();
}

static void mcsdl_read_32bits(UINT8 *pData)
{
	int i, j;

	preempt_disable();

	MCSDL_GPIO_SDA_SET_INPUT();

	for (i = 3; i >= 0; i--)
	{

		pData[i] = 0;

		for (j = 0; j < 8; j++)
		{

			pData[i] <<= 1;

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_3US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_3US);

			if (MCSDL_GPIO_SDA_IS_HIGH())
				pData[i] |= 0x01;

		}
	}
	
	preempt_enable();
}

static void mcsdl_write_bits(UINT32 wordData, int nBits)
{
	int i;

	preempt_disable();

	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_OUTPUT();

	for (i = 0; i < nBits; i++)
	{

		if (wordData & 0x80000000)
		{
			MCSDL_GPIO_SDA_SET_HIGH();
		}
		else
		{
			MCSDL_GPIO_SDA_SET_LOW();
		}

		mcsdl_delay(MCSDL_DELAY_7US);

		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_3US);

		wordData <<= 1;
	}
	preempt_enable();
}

#if 0
static void mcsdl_scl_toggle_twice(void)
{

	MCSDL_GPIO_SDA_SET_HIGH();
	MCSDL_GPIO_SDA_SET_OUTPUT();

	MCSDL_GPIO_SCL_SET_HIGH();
	mcsdl_delay(MCSDL_DELAY_20US);
	MCSDL_GPIO_SCL_SET_LOW();
	mcsdl_delay(MCSDL_DELAY_20US);

	MCSDL_GPIO_SCL_SET_HIGH();
	mcsdl_delay(MCSDL_DELAY_20US);
	MCSDL_GPIO_SCL_SET_LOW();
	mcsdl_delay(MCSDL_DELAY_20US);
}
#endif

//============================================================
//
//	Delay Function
//
//============================================================
static void mcsdl_delay(UINT32 nCount)
{

	switch (nCount)
	{
	case MCSDL_DELAY_1US:
		udelay(1);
		break;
	case MCSDL_DELAY_2US:
		udelay(2);
		break;
	case MCSDL_DELAY_3US:
		udelay(3);
		break;
	case MCSDL_DELAY_5US:
		udelay(5);
		break;
	case MCSDL_DELAY_7US:
		udelay(7);
		break;
	case MCSDL_DELAY_10US:
		udelay(10);
		break;
	case MCSDL_DELAY_15US:
		udelay(15);
		break;
	case MCSDL_DELAY_20US:
		udelay(20);
		break;
	case MCSDL_DELAY_100US:
		udelay(100);
		break;
	case MCSDL_DELAY_150US:
		udelay(150);
		break;
	case MCSDL_DELAY_500US:
		udelay(500);
		break;
	case MCSDL_DELAY_800US:
		udelay(800);
		break;
	case MCSDL_DELAY_1MS:
		mdelay(1);
		break;
	case MCSDL_DELAY_5MS:
		mdelay(5);
		break;
	case MCSDL_DELAY_10MS:
		mdelay(10);
		break;
	case MCSDL_DELAY_25MS:
		mdelay(25);
		break;
	case MCSDL_DELAY_30MS:
		mdelay(30);
		break;
	case MCSDL_DELAY_45MS:
		mdelay(45);
		break;
	case MCSDL_DELAY_40MS:
		mdelay(40);
		break;
		//start ADD DELAY
	case MCSDL_DELAY_300US:
		udelay(300);
		break;
	case MCSDL_DELAY_60MS:
		mdelay(60);
		break;
	case MCSDL_DELAY_40US:
		udelay(40);
		break;
		//end del
	default:
		break;
	}// Please, Use your delay function


}

//============================================================
//
//	Debugging print functions.
//
//============================================================

#ifdef MELFAS_ENABLE_DBG_PRINT

static void mcsdl_print_result(int nRet)
{
	if( nRet == MCSDL_RET_SUCCESS )
	{

		printk(" - TSP Firmware download was Successful!\n");

	}
	else
	{

		printk(" - TSp Firmware download Failed! ");

		switch( nRet )
		{

			case MCSDL_RET_SUCCESS : printk("MCSDL_RET_SUCCESS\n" ); break;
			case MCSDL_RET_ERASE_FLASH_VERIFY_FAILED : printk("MCSDL_RET_ERASE_FLASH_VERIFY_FAILED\n" ); break;
			case MCSDL_RET_PROGRAM_VERIFY_FAILED : printk("MCSDL_RET_PROGRAM_VERIFY_FAILED\n" ); break;

			case MCSDL_RET_PROGRAM_SIZE_IS_WRONG : printk("MCSDL_RET_PROGRAM_SIZE_IS_WRONG\n" ); break;
			case MCSDL_RET_VERIFY_SIZE_IS_WRONG : printk("MCSDL_RET_VERIFY_SIZE_IS_WRONG\n" ); break;
			case MCSDL_RET_WRONG_BINARY : printk("MCSDL_RET_WRONG_BINARY\n" ); break;

			case MCSDL_RET_READING_HEXFILE_FAILED : printk("MCSDL_RET_READING_HEXFILE_FAILED\n" ); break;
			case MCSDL_RET_FILE_ACCESS_FAILED : printk("MCSDL_RET_FILE_ACCESS_FAILED\n" ); break;
			case MCSDL_RET_MELLOC_FAILED : printk("MCSDL_RET_MELLOC_FAILED\n" ); break;

			case MCSDL_RET_WRONG_MODULE_REVISION : printk("MCSDL_RET_WRONG_MODULE_REVISION\n" ); break;

			default : printk("UNKNOWN ERROR. [0x%02X].\n", nRet ); break;
		}

		printk("\n");
	}

}

#endif

#if MELFAS_ENABLE_DELAY_TEST

//============================================================
//
//	For initial testing of delay and gpio control
//
//	You can confirm GPIO control and delay time by calling this function.
//
//============================================================

void mcsdl_delay_test(INT32 nCount)
{
	INT16 i;

	MELFAS_DISABLE_BASEBAND_ISR(); // Disable Baseband touch interrupt ISR.
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET(); // Disable Baseband watchdog timer

	//--------------------------------
	//	Repeating 'nCount' times
	//--------------------------------

	MCSDL_SET_GPIO_I2C();
	MCSDL_GPIO_SCL_SET_OUTPUT();
	MCSDL_GPIO_SDA_SET_OUTPUT();
	MCSDL_RESETB_SET_OUTPUT();

	MCSDL_GPIO_SCL_SET_HIGH();

	for( i=0; i<nCount; i++ )
	{
		MCSDL_GPIO_SCL_SET_LOW();

		mcsdl_delay(MCSDL_DELAY_20US);

		MCSDL_GPIO_SCL_SET_HIGH();

		mcsdl_delay(MCSDL_DELAY_100US);
	}

	MCSDL_GPIO_SCL_SET_HIGH();

	MELFAS_ROLLBACK_BASEBAND_ISR(); // Roll-back Baseband touch interrupt ISR.
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET(); // Roll-back Baseband watchdog timer
}

#endif
