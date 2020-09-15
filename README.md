
This project is designed to capture frames from the Ingenic T20 camera and write them to a V4L2 device.



The main stream must be channel 0, group 0.
The secondary stream must be channel 1, group 1.


One group only supports one resolution, and different resolutions need to be in distinct groups.
However a single group can support both H264 and JPEG capture formats.