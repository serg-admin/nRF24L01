#ifndef __ERROR_H_
#define __ERROR_H_ 1

#define ERR_ALARM_CRC_BASE 0x0100
#define ERR_ALARM_CRC(num)          (ERR_ALARM_CRC_BASE | num) //Ошибка CRC при загрузки будильника

#define ERR_I2C_BASE                0xA100
#define ERR_I2C(state)              (ERR_I2C_BASE | state)

#define ERR_I2C_BASE2               0xA300
#define ERR_I2C_BUSY                (ERR_I2C_BASE2 | 0x00)

#define ERR_COM_PARSER_BASE 0xC100
#define ERR_COM_PARSER_PARS_ERR     (ERR_COM_PARSER_BASE | 0x01)
#define ERR_COM_PARSER_UNKNOW_COM   (ERR_COM_PARSER_BASE | 0x02)

#endif
