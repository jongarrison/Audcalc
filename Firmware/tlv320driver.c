#include "tlv320driver.h"

#define I2C_PORT i2c1
#define I2C_SDA 2
#define I2C_SCL 3
#define TLV_RESET_PIN 21
#define MCLK_OUTPUT_PIN 16

#define TLV_I2C_ADDR            0x18
#define TLV_REG_PAGESELECT	    0
#define TLV_REG_LDOCONTROL	    2
#define TLV_REG_RESET   	    1
#define TLV_REG_CLK_MULTIPLEX   	0x04

uint8_t currentPage = -1;
bool writeRegister(uint8_t reg, uint8_t val)
{
    uint8_t data[2];
    data[0] = reg;
    data[1] = val;
	int attempt=0;
	while (1) {
		attempt++;
        
        int ret = i2c_write_blocking(I2C_PORT, TLV_I2C_ADDR, data, 2, false);
		if (ret == 2) {
            printf("TLV320 write ok, %d tries\n", attempt);
            return true;
        }
        if (attempt >= 12) {
            printf("TLV320 write failed, %d tries\n", attempt);
            return false;
        }
        sleep_us(80);
    }
}
bool readRegister(uint8_t reg)
{
	int attempt=0;
	while (1) {
		attempt++;
        
        int ret = i2c_write_blocking(I2C_PORT, TLV_I2C_ADDR, &reg, 1, true);
        sleep_ms(5);
		if (ret == 1) {
            int ret;
            uint8_t rxdata;
            attempt = 0;
            ret = i2c_read_blocking(I2C_PORT, TLV_I2C_ADDR, &rxdata, 1, false);
            if(ret == 1)
            {
                printf("reg: %x val: %x\n", reg, rxdata);
                return true;
            }
            else
            {
                printf("failed read %i\n", ret);
                return false;
            }
        }
        if (attempt >= 12) {
            printf("TLV320 read failed, %d tries\n", attempt);
            return false;
        }
        sleep_us(80);
    }
}
bool write(uint8_t page, uint8_t reg, uint8_t val)
{
    if(currentPage != page)
    {
        if(!writeRegister(0, page))
        {
            printf("failed to go to page %i\n", page);
            return false;
        }
        currentPage = page;
    }
    return writeRegister(reg, val);
}

void tlvDriverInit()
{
        // reset
    write(0, 0x01, 0x01);

    // configure pll dividers
    // our bitclock runs at 32x(sample_freq)
    // so with 44100 we use the following values
    // pllp 1 pllr 3 pllj 20 plld 0 MADC 3 NADC 5 AOSR 128 MDAC 3 NDAC 5 DOSR 128
    // 44100*32*3*20.0

    // use bitclock for pll input, pll for clock input
    write(0, 0x04, 0x07);

    // pll p & r & powerup
    write(0, 0x05, (0x01<<7)|(0x01<<4)|0x03);
    // pll j
    write(0, 0x06, 0x14);


    // ndac &p powerup
    write(0, 0x0b, 0x80|0x05);
    // mdac & powerup
    write(0, 0x0c, 0x80|0x03);

    // adc clocks should be driven by dac clocks, so no power necessary, but need to be set correctly
    // but you still need to set them correctly
    write(0, 0x12, 0x05);
    write(0, 0x13, 0x03);

    // dosr 64
    write(0, 0x0d, 0x00);
    write(0, 0x0e, 0x40);

    // adc osr 64
    write(0, 0x14, 0x40);
    // Select ADC PRB_R7
    write(0, 0x3d, 0x07);

    // word length
    // processing block? default doesn't have the beep generator :(
    // printf("setup clocks\n");
    // sleep_ms(1000);
    // printf("one second to power\n");
    // sleep_ms(1000);
    // printf("power\n");
    // power enable
    // disable internal avdd prior to powering up internal LDO
    write(1, 0x01, 0x08);
    // enable analog power blocks

    // common mode voltage
    //
    // set ref to slow powerup
    write(1, 0x7b, 0x00);
    // HP softstepping settingsfor optimal pop performance at power up
    // Rpopusedis 6k withN = 6 and softstep= 20usec.Thisshouldworkwith47uFcoupling
    // capacitor.Can try N=5, 6 or 7 time constants as well.Trade-offdelay vs “pop” sound.
    write(1, 0x14, 0x25);
    // enable LDO
    write(1, 0x02, 0x01);

    // setup headphones to be powered by ldo
    // and set LDO input to 1.8-3.3
    write(1, 0x0a, 0x0B);

    // Set MicPGA startup delay to 3.1ms
    write(1, 0x47, 0x32);
    
    /**/
    // line in
    // disable micbias
    write(1, 0x33, 0);
    
    // route IN1L to MicPGAL positive with 20k input impedance
    write(1, 0x34, 0x80);
    // route Commonmode to MicPGAL neg with 20k input impedance    
    write(1, 0x36, 0x80);

    // route IN1R to MicPGAR positive with 20k input impedance
    write(1, 0x37, 0x80);
    // route Commonmode to MicPGAR neg with 20k input impedance    
    write(1, 0x39, 0x80);
    // might need 0x3b / 0x3c gain control for MICPGA - leaving it at 0 gain for now
    write(1, 0x3b, 0x0c);
    write(1, 0x3c, 0x0c);
    

    /*   
// mic in
 // micbias disable, because we are using power directly from the 3v3 rail (for now)
 // eventually we would want to use the mic bias to power, but we need a smaller resistance on the micbias line.

    write(1, 0x33, 0x00);
    write(1, 0x34, 0x10); // IN2L to MicPGAL pos with 10k input impedance
    write(1, 0x36, 0x10); // IN2r to MicPGAL Neg with 10k input impedance
    write(1, 0x3b, 0x30); // Left MICPGA Volume Control 
    write(1, 0x3c, 0x30);
 */
    // analog bypass & dac routed to headphones
    write(1, 0x0c, 0x0a);
    write(1, 0x0d, 0x0a);

    // dac & mixer to headphones
    write(1, 0x0c, 0x0a);
    write(1, 0x0d, 0x0a);
    
    // route dac only to headphones
    //write(1, 0x0c, 0x08);
    //write(1, 0x0d, 0x08);

    
    // Set the ADC PTM Mode to PTM_R1
    write(1, 0x3d, 0xff);

    // Set the HPL gainto 0dBw 30
    // set gain to whatever the heck this is?
    write(1, 0x10, 0x00);
    write(1, 0x11, 0x00);
    
    // power up headphones & mixer amp
    write(1, 0x09, 0x33);

    // Powerup the Left and Right DAC Channels
    write(0, 0x3f, 0xd6);
    // Unmutethe DAC digital volume control 
    write(0, 0x40, 0x00);
    sleep_ms(200);
    printf("powering up adc, lets go\n");
    sleep_ms(200);
    printf("powering up adc, now\n");
    // power up ADC
    write(0, 0x51, 0xc0);
    write(0, 0x52, 0x00);
}