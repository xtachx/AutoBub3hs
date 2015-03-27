This is AutoBub version 2. It was written from scratch to work with ALL bubble chambers used by PICO. While I have tested this on 30l-13 and a run of 2l-15, more testing is needed to assess its performance for the other bubble chambers. It uses background subtraction fromt he Jean-Marc-Odobez MultiBGS package.

Installation:

cmake
make
./abub <location of data> <run number> <output folder>


Output:

abub2_<run number>.txt in PICO recon format.

You can comment out the section on saving diagnostic images under AutoBubStart.cpp to see the frames with the recon done on them. Crosshairs are drawn (uncomment the line to draw crosshairs on TplMatchLocalizer.cpp).

Please use the issue tracker!



