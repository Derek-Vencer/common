


//#include "rt_TypeDef.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "simulate_i2c.h"
//#include "rt_Time.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "bt_common_define.h"

#define TITLE_STR(x)        #x
#define SimulateI2cLog(num,title,value,...) do{DEBUG_INFO(num, title ":" value,##__VA_ARGS__);}while(0)
#define SimulateI2cLogI(num,x,...)  SimulateI2cLog(num,TITLE_STR(SimulateI2cLogI),x,##__VA_ARGS__ )
#define SimulateI2cLogD(num,x,...)  SimulateI2cLog(num,TITLE_STR(SimulateI2cLogD),x,##__VA_ARGS__ )
#define SimulateI2cLogW(num,x,...)  SimulateI2cLog(num,TITLE_STR(SimulateI2cLogW),x,##__VA_ARGS__ )
#define SimulateI2cLogE(num,x,...)  SimulateI2cLog(num,TITLE_STR(SimulateI2cLogE),x,##__VA_ARGS__ )


#ifdef BOARD_HW_GPIO_I2C_MODULE

static bool i2c_init_flag = false;

void i2c_delay(int delay_us){ // delay 4us
    hal_sys_timer_delay_us(delay_us);
}

void i2c_sda(u8 state)
{
    if (state&0x01)
        hal_gpio_pin_set(I2C_SDA_PIN);
    else
        hal_gpio_pin_clr(I2C_SDA_PIN);
}


void  i2c_sck(u8 state)
{
    if (state&0x01)
        hal_gpio_pin_set(I2C_SCK_PIN);
    else
        hal_gpio_pin_clr(I2C_SCK_PIN);
}

void i2c_init(void)
{
#if 0
    struct HAL_GPIO_I2C_CONFIG_T I2C_cfg = {I2C_IOMUX_SCL, I2C_IOMUX_SDA}; 
    hal_gpio_i2c_initialize(&I2C_cfg);
#else
    if(i2c_init_flag == true)
        return;
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_codex[] = {
        {I2C_IOMUX_SCL, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL}, //clk
        {I2C_IOMUX_SDA, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL}, // data
    };
    hal_iomux_init(pinmux_codex, ARRAY_SIZE(pinmux_codex));
    hal_gpio_pin_set_dir(I2C_SCK_PIN, HAL_GPIO_DIR_OUT, 1); // clk
    hal_gpio_pin_set_dir(I2C_SDA_PIN, HAL_GPIO_DIR_OUT, 1); // data
    i2c_init_flag = true;
    //osDelay(20);
    SimulateI2cLogI(0,"i2c_init \r\n");
#endif
}

void i2c_start(void)
{
#if 0
    hal_gpio_i2c_start();
#else
    i2c_sda(1);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(1);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sda(0);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
#endif
}

void i2c_stop(void)
{
#if 0
    hal_gpio_i2c_stop();
#else
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sda(0);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(1);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sda(1);
    i2c_delay(I2C_SLEEP_TIME);
#endif
}

void i2c_send_ack(u8 state)
{
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sda((state?0:1));
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(1);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sda(0);
    i2c_delay(I2C_SLEEP_TIME);
}

u8 i2c_read(void){
    i2c_delay(I2C_SLEEP_TIME);
    i2c_delay(I2C_SLEEP_TIME);
    return hal_gpio_pin_get_val(I2C_SDA_PIN);
}

u8 i2c_check_ack(void)
{
    u8 ack;
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    hal_gpio_pin_set_dir(I2C_SDA_PIN,HAL_GPIO_DIR_IN,1);
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(1);
    i2c_delay(I2C_SLEEP_TIME);
    ack = i2c_read(); 
    i2c_delay(I2C_SLEEP_TIME);
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    hal_gpio_pin_set_dir(I2C_SDA_PIN,HAL_GPIO_DIR_OUT,0);
    i2c_delay(I2C_SLEEP_TIME);
    return (ack?0:1);
}

u8 i2c_readByte(void)
{
    u8 data = 0;
    u8 data_bit;
    u8 i =8;
    
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    hal_gpio_pin_set_dir(I2C_SDA_PIN,HAL_GPIO_DIR_IN,1);
    i2c_delay(I2C_SLEEP_TIME);

    while(i--)
    {
        data<<=1;
        i2c_sck(0);
        i2c_delay(I2C_SLEEP_TIME);
        i2c_sck(1);
        i2c_delay(I2C_SLEEP_TIME);
        data_bit = hal_gpio_pin_get_val(I2C_SDA_PIN);
        if(data_bit){
            data|=0x01;
        }
    }
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
    hal_gpio_pin_set_dir(I2C_SDA_PIN,HAL_GPIO_DIR_OUT,0);
    i2c_delay(I2C_SLEEP_TIME);
    return data;
}

void i2c_writeByte(u8 data)
{
    u8 i=8;
    while(i--)
    {
            i2c_sck(0);
            i2c_delay(I2C_SLEEP_TIME);
            if(data&0x80)
                    i2c_sda(1); 
            else
                    i2c_sda(0);
            data<<=1;
            i2c_delay(I2C_SLEEP_TIME);
            i2c_sck(1);
            i2c_delay(I2C_SLEEP_TIME);
    }
    i2c_sck(0);
    i2c_delay(I2C_SLEEP_TIME);
}

int i2c_write_reg_data(u8 device_addr,u8 reg_addr,u8 *buf,int len){
    int ret = 0,retry=0;
    
    i2c_init();
loop1:  
    i2c_start();
    i2c_writeByte((device_addr<<1)&0xFE); // write device addr
    ret = i2c_check_ack();
    if(!ret){
        i2c_delay(500);
        if(retry++>100){
            i2c_stop();
            SimulateI2cLogE(0,"write device_addr fail\r\n");
            return -1;
        }
        goto loop1;
    }
    i2c_writeByte(reg_addr);
    ret = i2c_check_ack();
    if(!ret){
        i2c_stop();
        SimulateI2cLogE(0,"write reg_addr no ack \r\n");
        return -1;
    }
    for(int i=0;i<len;i++){
        i2c_writeByte(buf[i]);
        ret = i2c_check_ack();
        if(!ret){
            i2c_stop();
            SimulateI2cLogE(1,"write data no ack %d \r\n",i);
            return -1;
        }
    }
    i2c_stop();
    return ret;
}

int i2c_read_reg_data(u8 device_addr,u8 reg_addr,u8 *buf,int len){
    int ret = 0,retry=0;
    i2c_init();
loop1:
    i2c_start();
    i2c_writeByte((device_addr<<1)&0xFE); // write device addr
    ret = i2c_check_ack();
    if(!ret){
        i2c_delay(500);
        if(retry++>1000){
            SimulateI2cLogE(0,"read device_addr fail\r\n");
            return -1;
        }
        goto loop1;
    }
    i2c_writeByte(reg_addr);
    ret = i2c_check_ack();
    if(!ret){
        i2c_stop();
        SimulateI2cLogE(0,"read reg_addr no ack \r\n");
        return -1;
    }
    i2c_stop();
    retry = 0;
loop2:
    i2c_start();
    i2c_writeByte((device_addr<<1)|0x01); // write device addr
    ret = i2c_check_ack();
    if(!ret){
        i2c_delay(500);
        if(retry++>100){
            SimulateI2cLogE(0,"write device read addr fail \r\n");
            return -1;
        }
        goto loop2;
    }
    for(int i=0;i<len;i++){
        buf[i] = i2c_readByte();
        if(i==(len-1))
            i2c_send_ack(0);
        else
            i2c_send_ack(1);
    }
    i2c_stop();
    return ret;
}

#endif
