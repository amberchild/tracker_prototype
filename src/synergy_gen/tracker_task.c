/* generated thread source file - do not edit */
#include "tracker_task.h"

TX_THREAD tracker_task;
void tracker_task_create(void);
static void tracker_task_func(ULONG thread_input);
static uint8_t tracker_task_stack[4096] BSP_PLACE_IN_SECTION_V2(".stack.tracker_task") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const i2c_cfg_t g_sf_i2c_bmx055_mag_i2c_cfg =
{ .channel = g_sf_i2c_bus0_CHANNEL,
  .rate = g_sf_i2c_bus0_RATE,
  .slave = 0x13,
  .addr_mode = I2C_ADDR_MODE_7BIT,
  .sda_delay = g_sf_i2c_bus0_SDA_DELAY,
  .p_transfer_tx = g_sf_i2c_bus0_P_TRANSFER_TX,
  .p_transfer_rx = g_sf_i2c_bus0_P_TRANSFER_RX,
  .p_callback = g_sf_i2c_bus0_P_CALLBACK,
  .p_context = g_sf_i2c_bus0_P_CONTEXT,
  .rxi_ipl = g_sf_i2c_bus0_RXI_IPL,
  .txi_ipl = g_sf_i2c_bus0_TXI_IPL,
  .tei_ipl = g_sf_i2c_bus0_TEI_IPL,
  .eri_ipl = g_sf_i2c_bus0_ERI_IPL,
  .p_extend = g_sf_i2c_bus0_P_EXTEND, };

sf_i2c_instance_ctrl_t g_sf_i2c_bmx055_mag_ctrl =
{ .p_lower_lvl_ctrl = &g_i2c0_ctrl, };
const sf_i2c_cfg_t g_sf_i2c_bmx055_mag_cfg =
{ .p_bus = (sf_i2c_bus_t *) &g_sf_i2c_bus0, .p_lower_lvl_cfg = &g_sf_i2c_bmx055_mag_i2c_cfg, };
/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_bmx055_mag =
{ .p_ctrl = &g_sf_i2c_bmx055_mag_ctrl, .p_cfg = &g_sf_i2c_bmx055_mag_cfg, .p_api = &g_sf_i2c_on_sf_i2c };
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const i2c_cfg_t g_sf_i2c_bmx055_gyr_i2c_cfg =
{ .channel = g_sf_i2c_bus0_CHANNEL,
  .rate = g_sf_i2c_bus0_RATE,
  .slave = 0x69,
  .addr_mode = I2C_ADDR_MODE_7BIT,
  .sda_delay = g_sf_i2c_bus0_SDA_DELAY,
  .p_transfer_tx = g_sf_i2c_bus0_P_TRANSFER_TX,
  .p_transfer_rx = g_sf_i2c_bus0_P_TRANSFER_RX,
  .p_callback = g_sf_i2c_bus0_P_CALLBACK,
  .p_context = g_sf_i2c_bus0_P_CONTEXT,
  .rxi_ipl = g_sf_i2c_bus0_RXI_IPL,
  .txi_ipl = g_sf_i2c_bus0_TXI_IPL,
  .tei_ipl = g_sf_i2c_bus0_TEI_IPL,
  .eri_ipl = g_sf_i2c_bus0_ERI_IPL,
  .p_extend = g_sf_i2c_bus0_P_EXTEND, };

sf_i2c_instance_ctrl_t g_sf_i2c_bmx055_gyr_ctrl =
{ .p_lower_lvl_ctrl = &g_i2c0_ctrl, };
const sf_i2c_cfg_t g_sf_i2c_bmx055_gyr_cfg =
{ .p_bus = (sf_i2c_bus_t *) &g_sf_i2c_bus0, .p_lower_lvl_cfg = &g_sf_i2c_bmx055_gyr_i2c_cfg, };
/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_bmx055_gyr =
{ .p_ctrl = &g_sf_i2c_bmx055_gyr_ctrl, .p_cfg = &g_sf_i2c_bmx055_gyr_cfg, .p_api = &g_sf_i2c_on_sf_i2c };
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const i2c_cfg_t g_sf_i2c_bmx055_acc_i2c_cfg =
{ .channel = g_sf_i2c_bus0_CHANNEL,
  .rate = g_sf_i2c_bus0_RATE,
  .slave = 0x19,
  .addr_mode = I2C_ADDR_MODE_7BIT,
  .sda_delay = g_sf_i2c_bus0_SDA_DELAY,
  .p_transfer_tx = g_sf_i2c_bus0_P_TRANSFER_TX,
  .p_transfer_rx = g_sf_i2c_bus0_P_TRANSFER_RX,
  .p_callback = g_sf_i2c_bus0_P_CALLBACK,
  .p_context = g_sf_i2c_bus0_P_CONTEXT,
  .rxi_ipl = g_sf_i2c_bus0_RXI_IPL,
  .txi_ipl = g_sf_i2c_bus0_TXI_IPL,
  .tei_ipl = g_sf_i2c_bus0_TEI_IPL,
  .eri_ipl = g_sf_i2c_bus0_ERI_IPL,
  .p_extend = g_sf_i2c_bus0_P_EXTEND, };

sf_i2c_instance_ctrl_t g_sf_i2c_bmx055_acc_ctrl =
{ .p_lower_lvl_ctrl = &g_i2c0_ctrl, };
const sf_i2c_cfg_t g_sf_i2c_bmx055_acc_cfg =
{ .p_bus = (sf_i2c_bus_t *) &g_sf_i2c_bus0, .p_lower_lvl_cfg = &g_sf_i2c_bmx055_acc_i2c_cfg, };
/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_bmx055_acc =
{ .p_ctrl = &g_sf_i2c_bmx055_acc_ctrl, .p_cfg = &g_sf_i2c_bmx055_acc_cfg, .p_api = &g_sf_i2c_on_sf_i2c };
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const i2c_cfg_t g_sf_i2c_bme280_i2c_cfg =
{ .channel = g_sf_i2c_bus0_CHANNEL,
  .rate = g_sf_i2c_bus0_RATE,
  .slave = 0x77,
  .addr_mode = I2C_ADDR_MODE_7BIT,
  .sda_delay = g_sf_i2c_bus0_SDA_DELAY,
  .p_transfer_tx = g_sf_i2c_bus0_P_TRANSFER_TX,
  .p_transfer_rx = g_sf_i2c_bus0_P_TRANSFER_RX,
  .p_callback = g_sf_i2c_bus0_P_CALLBACK,
  .p_context = g_sf_i2c_bus0_P_CONTEXT,
  .rxi_ipl = g_sf_i2c_bus0_RXI_IPL,
  .txi_ipl = g_sf_i2c_bus0_TXI_IPL,
  .tei_ipl = g_sf_i2c_bus0_TEI_IPL,
  .eri_ipl = g_sf_i2c_bus0_ERI_IPL,
  .p_extend = g_sf_i2c_bus0_P_EXTEND, };

sf_i2c_instance_ctrl_t g_sf_i2c_bme280_ctrl =
{ .p_lower_lvl_ctrl = &g_i2c0_ctrl, };
const sf_i2c_cfg_t g_sf_i2c_bme280_cfg =
{ .p_bus = (sf_i2c_bus_t *) &g_sf_i2c_bus0, .p_lower_lvl_cfg = &g_sf_i2c_bme280_i2c_cfg, };
/* Instance structure to use this module. */
const sf_i2c_instance_t g_sf_i2c_bme280 =
{ .p_ctrl = &g_sf_i2c_bme280_ctrl, .p_cfg = &g_sf_i2c_bme280_cfg, .p_api = &g_sf_i2c_on_sf_i2c };
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void tracker_task_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */

    UINT err;
    err = tx_thread_create (&tracker_task, (CHAR *) "Tracker Task", tracker_task_func, (ULONG) NULL,
                            &tracker_task_stack, 4096, 10, 10, 100, TX_AUTO_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&tracker_task, 0);
    }
}

static void tracker_task_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    tracker_task_entry ();
}
