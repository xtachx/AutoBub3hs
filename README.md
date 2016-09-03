This is AutoBub version 3. It was written from scratch to work with ALL bubble chambers used by PICO. While I have tested this on 30l-13 and a run of 2l-15, more testing is needed to assess its performance for the other bubble chambers. It uses the LBP algorithm for background subtraction.

Installation:

cmake
make
./abub3 <location of data> <run number> <output folder>


Output:

abub3_<run number>.txt in PICO recon format.

You can comment out the section on saving diagnostic images under AutoBubStart.cpp to see the frames with the recon done on them. Crosshairs are drawn (uncomment the line to draw crosshairs on TplMatchLocalizer.cpp).

Please use the issue tracker!



