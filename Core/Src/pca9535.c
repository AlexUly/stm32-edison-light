#include "pca9535.h"

uint16_t valueRegister;
uint16_t configurationRegister;

void PC9535_init(I2C_HandleTypeDef hic, uint8_t adr){

		valueRegister = 0;
		configurationRegister = configurationRegister & ~(0);
		char data[2];
		data[0] = configurationRegister;
		data[1] = (configurationRegister >> 8) &0xFF;

        char sendData[3];
        sendData[0] = adr;
        sendData[1] = PCA9555_CONFIG_PORT_0;
        sendData[2] = data[0];


        HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);

        sendData[1] = PCA9555_CONFIG_PORT_1;
        sendData[2] = data[1];

        HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);

}


void PCA9535_Write(I2C_HandleTypeDef hic, uint8_t pin, uint8_t value , uint8_t adr){
	    if (value > 0) {

	        valueRegister = valueRegister | (1 << pin);
	    } else {

	        valueRegister = valueRegister & ~(1 << pin);
	    }

	    char sendData[3];
	    sendData[0] = adr;
	    sendData[1] = PCA9555_OUTPUT_PORT_0;
	    sendData[2] = valueRegister;

	    HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);

	    sendData[1] = PCA9555_OUTPUT_PORT_1;
	    sendData[2] = (valueRegister >> 8) &0xFF;

	    HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);
}

void PCA9535_WriteSetAll(I2C_HandleTypeDef hic, uint8_t adr){

	    valueRegister = valueRegister | 0xFFFF;
	    char sendData[3];
	    sendData[0] = adr;
	    sendData[1] = PCA9555_OUTPUT_PORT_0;
	    sendData[2] = valueRegister;

	    HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);

	    sendData[1] = PCA9555_OUTPUT_PORT_1;
	    sendData[2] = (valueRegister >> 8) &0xFF;

	    HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);
}

void PCA9535_WriteClearAll(I2C_HandleTypeDef hic, uint8_t adr){

		valueRegister = valueRegister & 0;
	    char sendData[3];
	    sendData[0] = adr;
	    sendData[1] = PCA9555_OUTPUT_PORT_0;
	    sendData[2] = valueRegister;

	    HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);

	    sendData[1] = PCA9555_OUTPUT_PORT_1;
	    sendData[2] = (valueRegister >> 8) &0xFF;

	    HAL_I2C_Master_Transmit(&hic, adr, sendData,3,50);
}
