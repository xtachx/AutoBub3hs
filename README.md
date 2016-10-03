This is AutoBub version 3. It was written from scratch to work with ALL bubble chambers used by PICO. While I have tested this on 30l-16, more testing is needed to assess its performance for the other bubble chambers. LBP algorithm has been dropped temporarily in favour of the machine learning and simple profile thresholding which seems to be at par / better than just LBP or machine learning + LBP.

Installation:

cmake
make
./abub3 <location of data> <run number> <output folder>

<location of data> and <output folder> MUST have trailing slashes!

Output:

abub3_<run number>.txt in PICO recon format. You can comment out the section on saving diagnostic images under AutoBubStart.cpp to see the frames with the recon done on them.

The error codes in the recon file are as follows:

-1: Entropy did trigger but no bubble was found
-2: Entropy was massive in the first frame / usually a late trigger. Analysis not performed.
-3: Entropy did not trigger on the images of that camera series. (usually if other cameras see a trigger)
-5: Failed to make a file list of all files in the run (data corruption?)
-6: Failed to analyze / segfault (malformed images / data corruption /missing images)
-7: Failed to train on the dataset (some event folders might be empty)



Please use the issue tracker!



