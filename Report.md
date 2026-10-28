CSE 4331 Project 2 Report

Harshini Nekkanti ID-1002208177
Lucia Guerrero ID-1001756570
Team 7







Option Choice - We have chosen option A. We did not use AI.

Overall Status:
    We have completed zgt_tm.C and zgt_tx.C. We have compiled zgt_test.C and the code runs smoothly with no errors. It works with all
    input files given and gives the log outputs. We have also tested multiple times to check whether the files with threads can run multiple times.

Difficulties Faced:
    We had some difficulty understanding semaphores logic and how they are utilized in the transaction mananger. Another major difficulty is
    debugging race conditions. We faced several problems before getting the correct output. Ater compiling and testing, the output was always hannging with no error messages.
    We had to release the semaphores using ./ipcs_cleanup.sh, so that the hanging issue would stop and full output would be displayed.

File Descriptions:
    We had some issues with the tmtest file so we added our version of tmtest to ensure that it it compatible with omega to test multiple times.
    The tmtest file given to us was not working with omega, no matter how many times we tried to resolve it.

Division of Labor:
    Harshini worked on zgt_tm.C and half parts of zgt_tx.C. Lucia worked on the remaining parts of zgt_tx.C. We fixed each other issues. Both of them tested
    various input files. Lucia did more of testing as Harshini faced some tecnical issues. We tries our best to coordinate with each other based on our schedules. We have worked on it for three days.

Logical Errors and How We Handled Them:
    Logical Error 1: After compiling, the output would always hang. It took us a lot of time to figure out where we went wrong. We referred the project description,
                     and understood about releasing semaphores. So we used ./ipcs_cleanup.sh, which helped us in resolving the hanging issue.

    Logical Error 2: Input file format. One of the file had LF endings which resulted in getting the output log as .lo instead of .log. So,
                      we converted LF into CRLF as our test inputs all use CRLF endings.

    Logical Error 3: Not being able to test using tmtest. The given tmtest file was not working on omega. We tried to change the permissions and resolve it, but it 
                     still wouldn't work. We had to create bash version for it work on omega.
