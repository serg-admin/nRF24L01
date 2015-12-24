#include "nRf24l01.h"

void nRF24L01SetRegister(uint8_t reg, uint8_t b) {
  struct rec_spi_data *data = 
      spiGetBus(&NRF24L01_SCN_PORT, NRF24L01_SCN_PIN);
  data->sendBuff[0] = reg;
  data->sendBuff[1] = b;
  data->sendSize = 2;
  spiTransmit(2);
}