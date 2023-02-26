# from modules.Serial_functions import search_for_teensy_module, send_data_until_confirmation, wait_for_signal_byte
from time import time, sleep
from datetime import datetime
import keyboard


DATA_REQUEST = 1
RESET_COUNTERS = 2


if __name__ == "__main__":
    # Init - initialize the teensy/esp32 connection and create the data file
    with open("data" + datetime.today().strftime("%Y-%m-%d_%H-%M-%S") + ".txt", 'w') as txt_file:
        # Add header
        txt_file.write("Time in ms;Wheel1_cw_count;Wheel1_ccw_count;Wheel2_cw_count;Wheel2_ccw_count\n")

        while 1:
            try:
                sleep(1)
                data_received = "123456789;0.12;0.32;1.5;5.6542"

                print(data_received)
                txt_file.write(data_received+'\n')

                # give the user the option to
                if keyboard.is_pressed("ESC"):
                    break
                #
            except:
                break
            #
        #
    #
#
