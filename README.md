# magic_wand
Explanation for each dir

animation_generation:
    Directory used for making the shake, flip, and rotate animations. This code as been integrated into MagicWand_RX, so it is now an archive
freeCAD:
    cad files for the wand case
gather_imu_data:
    simply reads accel & gyro data from the imu & outputs it to the terminal (this is the file you want to upload to export the data to an excel file to give you insight on the gestures)
machine_learning_attempts:
    code I used when trying to get machine learning working. This can be ignored.
MagicWand_RX:
    Prod RX code
MagicWand_TX:
    Prod TX code
manual_gesture_recognition:
    file that identifies the performed gesture. Once you figure out how to detect each gesture from the data from gather_imu_data, implement it in this file (it already has a state machine). Eventually this code will be merged into the TX to trigger an MQTT message.