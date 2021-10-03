
This project is designed to capture frames from the Ingenic T20 camera and write them to a V4L2 device.



The main stream must be channel 0, group 0.
The secondary stream must be channel 1, group 1.


One group only supports one resolution, and different resolutions need to be in distinct groups.
However a single group can support both H264 and JPEG capture formats.


Config options in settings.json

flip_vertical:
- 0 no flip
- 1 vertical flip

flip_horizontal:
- 0 no flip
- 1 horizontal flip

timestamp_24h:
- 0 12h display
- 1 24h display

timestamp_location:
- 0 top left corner
- 1 top roght corner
- 2 bottom left corner
- 3 bottom right corner

show_timestamp:
- 0 disable timestampt
- 1 enable timestamp

enable_audio:
- 0 disable audio
- 1 enable audio

