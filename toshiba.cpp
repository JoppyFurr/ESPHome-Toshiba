#include "toshiba.h.h"

#define TOSHIBA_HEADER_MARK         4380
#define TOSHIBA_HEADER_SPACE        4370
#define TOSHIBA_GAP_SPACE           5480
#define TOSHIBA_BIT_MARK             540
#define TOSHIBA_ZERO_SPACE           540
#define TOSHIBA_ONE_SPACE           1620

#define TOSHIBA_COMMAND_DEFAULT     0x01
#define TOSHIBA_COMMAND_TIMER       0x02
#define TOSHIBA_COMMAND_POWER       0x08
#define TOSHIBA_COMMAND_MOTION      0x02

#define TOSHIBA_MODE_AUTO           0x00
#define TOSHIBA_MODE_COOL           0x01
#define TOSHIBA_MODE_DRY            0x02
#define TOSHIBA_MODE_HEAT           0x03
#define TOSHIBA_MODE_FAN_ONLY       0x04
#define TOSHIBA_MODE_OFF            0x07

#define TOSHIBA_FAN_SPEED_AUTO      0x00
#define TOSHIBA_FAN_SPEED_QUIET     0x20
#define TOSHIBA_FAN_SPEED_1         0x40
#define TOSHIBA_FAN_SPEED_2         0x60
#define TOSHIBA_FAN_SPEED_3         0x80
#define TOSHIBA_FAN_SPEED_4         0xa0
#define TOSHIBA_FAN_SPEED_5         0xc0

#define TOSHIBA_POWER_HIGH          0x01
#define TOSHIBA_POWER_ECO           0x03

#define TOSHIBA_MOTION_SWING        0x04
#define TOSHIBA_MOTION_FIX          0x00

namespace esphome {
    namespace toshiba {

        static const char *TAG = "toshiba.climate";

        void ToshibaClimate::transmit_state ()
        {
            uint8_t message [16] = { 0 };
            uint8_t message_length = 9;

            /* Header */
            message [0] = 0xf2;
            message [1] = 0x0d;

            /* Message length */
            message [2] = message_length - 6;

            /* First checksum */
            message [3] = message [0] ^ message [1] ^ message [2];

            /* Command */
            message [4] = TOSHIBA_COMMAND_DEFAULT;

            /* Temperature */
            uint8_t temperature = static_cast <uint8_t> (this->target_temperature);
            if (temperature < 17)
            {
                temperature = 17;
            }
            if (temperature > 30)
            {
                temperature = 30;
            }
            message [5] = (temperature - 17) << 4;

            /* Mode and fan */
            uint8_t mode;
            switch (this->mode)
            {
                case climate::CLIMEATE_MODE_OFF:
                    mode = TOSHIBA_MODE_OFF;
                    break;

                case climate::CLIMEATE_MODE_HEAT:
                    mode = TOSHIBA_MODE_HEAT;
                    break;

                case climate::CLIMEATE_MODE_COOL:
                    mode = TOSHIBA_MODE_COOL;
                    break;

                case climate::CLIMATE_MODE_AUTO:
                default:
                    mode = TOSHIBA_MODE_AUTO;
            }

            message [6] |= mode | TOSHIBA_FAN_AUTO;

            /* Zero */
            message [7] = 0x00;

            /* If timers bit in the command is set, two extra bytes are added here */

            /* If power bit is set in the command, one extra byte is added here */

            /* The last byte is the xor of all bytes from [4] */
            for (uint8_t i = 4; i < 8; i++)
            {
                message [8] ^= message [i];
            }

            /* Transmit */
            auto transmit = this->transmitter_->transmit ();
            auto data = transmit.get_data ();
            data->set_carrier_frequency (38000);

            for (uint8_t copy = 0; copy < 2; copy++)
            {
                data->mark (TOSHIBA_HEADER_MARK);
                data->space (TOSHIBA_HEADER_SPACE);

                for (uint8_t byte = 0; byte < message_length; byte++)
                {
                    for (uint8_t bit = 0; bit < 8; bit++)
                    {
                        data->mark (TOSHIBA_BIT_MARK);
                        if (message [byte] & (1 << (7 - bit)))
                        {
                            data->space (TOSHIBA_ONE_SPACE);
                        }
                        else
                        {
                            data->space (TOSHIBA_ZERO_SPACE);
                        }
                    }
                }

                data->mark (TOSHIBA_BIT_MARK);
                data->mark (TOSHIBA_GAP_SPACE);
            }

            transmit.perform ();
        }

        bool ToshibaClimate::on_receive (remote_base::RemoteReceiveData data)
        {
            /* TODO */
            return true;
        }
    }
}
