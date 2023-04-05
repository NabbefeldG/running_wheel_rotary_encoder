from modules.Serial_functions import search_for_teensy_module, send_data_until_confirmation, wait_for_signal_byte
from time import time, sleep
from datetime import datetime
import os
import keyboard


# Signals Bytes for communication with ESP32 Module
DATA_REQUEST = 1
RESET_COUNTERS = 2
SET_FILE_NAME = 3

# Delay between Data requests in seconds. E.g.: 5 -> every five seconds
delay_between_data_requests_sec = 5


if __name__ == "__main__":
    # Generate the file name here since it's also send to the microcontroller
    file_name = "data" + datetime.today().strftime("%Y-%m-%d_%H-%M-%S") + ".txt"

    # Init - initialize the teensy/esp32 connection and create the data file
    teensy_name = "rotary_encoder_esp32"
    serial_obj = search_for_teensy_module(teensy_name, baudrate=115200)
    sleep(0.01)  # Give it a second to get setup
    serial_obj.read_all()
    send_data_until_confirmation(serial_obj=serial_obj, header_byte=SET_FILE_NAME)
    serial_obj.write(file_name.encode())  # I didn't want to deal with all the message length handling stuff, cause this is the only input of that type and I know that its 27 chars
    sleep(0.01)  # Give the controller time to generate the local file
    send_data_until_confirmation(serial_obj=serial_obj, header_byte=RESET_COUNTERS)
    sleep(0.01)  # A short delay until we requirest the first data points

    print(file_name)

    os.makedirs("data", exist_ok=True)
    with open(os.path.join("data", file_name), 'w') as txt_file:
        # Add header
        # txt_file.write("Time in ms;" + "Wheel1_cw_count;" + "Wheel1_ccw_count;" + "Wheel2_cw_count;" + "Wheel2_ccw_count" + '\n')
        txt_file.write("Time in ms;Wheel1_cw_count;Wheel1_ccw_count;Wheel2_cw_count;Wheel2_ccw_count\n")

        while 1:
            try:
                # sleep(0.01)
                send_data_until_confirmation(serial_obj=serial_obj, header_byte=DATA_REQUEST)
                sleep(delay_between_data_requests_sec)

                # read data from teensy
                received = 0
                data_received = ""
                st = time()
                while not received:
                    data_received = data_received + serial_obj.readline().decode()

                    if len(data_received) > 1:
                        if data_received[-2:] == '\r\n':
                            received = 1
                        #
                    #

                    # This is a catch case that never happened while setting this up, but I wanted to make sure that
                    # there isn't any weird delay and this whole script stops working cause I never get the "\r\n" and
                    # the experiment breaks at this point
                    if time() - st > 5:
                        print("Timeout")
                        data_received = "Header:Timeout\r\n"
                        received = 1
                    #
                #

                # 1. removes the linebreak chars: "\r\n"
                # 2. splits at the ':'-char (which I just as padding in case I missed the first chars transmitted)
                #    and leaves the data to be saved.
                data_received = data_received[0:-2].split(":")[1]

                # Save data to file
                # [ms-timestamp, cw counter, ccw counter)]
                print(data_received)
                txt_file.write(data_received+'\n')

                # give the user the option to
                if keyboard.is_pressed("ESC"):
                    break
                #
            except Exception as e:
                print(e)
                sleep(5)
                # break
            #
        #
    #

    try:
        serial_obj.close()
    except Exception as error_msg:
        print(error_msg)
    #
#
