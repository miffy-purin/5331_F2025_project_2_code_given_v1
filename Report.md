CSE 4331 Project 2 Report

Harshini Nekkanti ID-1002208177 Lucia Guerrero ID-1001756570 Team 7

Option Choice - We have chosen option A. We did not use AI.

Overall Status: We have completed zgt_tm.C and zgt_tx.C. We have compiled zgt_test.C and the code runs smoothly with no errors. It works with all input files given and gives the log outputs. We have also tested multiple times to check whether the files with threads can run multiple times.

Difficulties Faced: We had some difficulty understanding semaphores logic and how they are utilized in the transaction mananger. Another major difficulty is debugging race conditions. We faced several problems before getting the correct output. Ater compiling and testing, the output was always hannging with no error messages. We had to release the semaphores using ./ipcs_cleanup.sh, so that the hanging issue would stop and full output would be displayed.

File Descriptions: We had some issues with the tmtest file so we added our version of tmtest to ensure that it it compatible with omega to test multiple times. The tmtest file given to us was not working with omega, no matter how many times we tried to resolve it.

Division of Labor: Harshini worked on zgt_tm.C and half parts of zgt_tx.C. Lucia worked on the remaining parts of zgt_tx.C. We fixed each other issues. Both of them tested various input files. Lucia did more of testing as Harshini faced some tecnical issues. We tried our best to coordinate with each other based on our schedules. We have worked on it for three days.

Logical Errors and How We Handled Them: Logical Error 1: After compiling, the output would always hang. It took us a lot of time to figure out where we went wrong. We referred the project description, and understood about releasing semaphores. So we used ./ipcs_cleanup.sh, which helped us in resolving the hanging issue.

Logical Error 2: Input file format. One of the file had LF endings which resulted in getting the output log as .lo instead of .log. So,
                  we converted LF into CRLF as our test inputs all use CRLF endings.

Logical Error 3: Not being able to test using tmtest. The given tmtest file was not working on omega. We tried to change the permissions and resolve it, but it 
                 still wouldn't work. We had to create bash version for it work on omega.


Input of ddlk_2Txs.txt:
// 2 transactions
// classic deadlock
// will hang w/o deadlock resolution
Log ddlk_2Tx.log
BeginTx 1 W
BeginTx 2 W
Read    1 1
Read    2 2
Write   1 2
Write   2 1
Commit  1
commit 2
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T2	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	      	ReadTx		2:-1:2328		ReadLock	Granted		 P


Input of ddlk_3Txs.txt:
// possible deadlock test case
// Two write transactions
log ddlk_3Tx.log
// op	Tx#	type
BeginTx 1 W
// op	Tx#	Obj
Read 1 1
Write 1 2
Read 1 6
BeginTx 2 W
Read 2 2
Write 2 1
Read 2 7
commit 2
Commit 1
begintx 3 R
read 3 2
write 3 1
read 3 2
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	ReadTx		2:-1:2328		ReadLock	Granted		 P
T3	R 	BeginTx

Input of disj_multi_accesses.txt:
// serial history
// 2 transactions
// same disjoint objects accessed
// multiple times
Log disj_multi_accesses.log
BeginTx 1 W
Read    1 1
Read    1 2
Write   1 3
Write   1 4
read    1 1
write   1 2
write   1 4
write   1 4
commit 1
begintx 2 W
read    2 5
write   2 5
write   2 6
read    2 6
commit  2
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	ReadTx		5:-1:2328		ReadLock	Granted		 P
T2	      	WriteTx		5:0:2328		WriteLock	Granted		 P
T2	      	WriteTx		6:1:2328		WriteLock	Granted		 P
T2	      	ReadTx		6:0:2328		ReadLock	Granted		 P
T2	 	CommitTx	6 : 0, 5 : 0, 
T1	      	ReadTx		2:-1:18755		ReadLock	Granted		 P
T1	      	WriteTx		3:1:18755		WriteLock	Granted		 P
T1	      	WriteTx		4:1:18755		WriteLock	Granted		 P
T1	      	ReadTx		1:-2:18755		ReadLock	Granted		 P
T1	      	WriteTx		2:0:18755		WriteLock	Granted		 P
T1	      	WriteTx		4:2:18755		WriteLock	Granted		 P
T1	      	WriteTx		4:3:18755		WriteLock	Granted		 P
T1	 	CommitTx	4 : 3, 3 : 1, 2 : 0, 1 : -2,

Input of interleaved_RW.txt:
log interleaved_RW.log
BeginTx 1 W
Read 1 1
Write 1 2
Read 1 3
Write 1 4
BeginTx 2 W
Write 2 8
Write 2 2
Read 2 1
BeginTx 3 W
Write 3 1
Write 3 2
Write 3 9
Read 3 8
Commit 1
commit 2
Commit 3
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	WriteTx		8:1:2328		WriteLock	Granted		 P
T3	W 	BeginTx
T2	      	WriteTx		2:1:2328		WriteLock	Granted		 P

Input of Mult_ROTxs.txt:
// Multiple RO Txs test case
// read only transactions
log Multi_ROTxs.log
// op	Tx#	type
// op	Tx#	Obj
BeginTx 1 R
Read 1 1
Read 1 2
Read 1 3
Read 1 8
BeginTx 2 R
Read 2 1
Read 2 8
Read 2 5
BeginTx 3 R
Read 3 1
Read 3 5
Read 3 3
read 3 7
Commit 2
commit 3
Commit 1
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	R 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	R 	BeginTx
T3	R 	BeginTx
T1	      	ReadTx		2:-1:18755		ReadLock	Granted		 P
T1	      	ReadTx		3:-1:18755		ReadLock	Granted		 P
T1	      	ReadTx		8:-1:18755		ReadLock	Granted		 P
T1	 	CommitTx	8 : -1, 3 : -1, 2 : -1, 1 : -1, 
T2	      	ReadTx		1:-2:2328		ReadLock	Granted		 P
T2	      	ReadTx		8:-2:2328		ReadLock	Granted		 P
T2	      	ReadTx		5:-1:2328		ReadLock	Granted		 P
T2	 	CommitTx	5 : -1, 8 : -2, 1 : -2, 
T3	      	ReadTx		1:-3:12205		ReadLock	Granted		 P
T3	      	ReadTx		5:-2:12205		ReadLock	Granted		 P
T3	      	ReadTx		3:-2:12205		ReadLock	Granted		 P
T3	      	ReadTx		7:-1:12205		ReadLock	Granted		 P
T3	 	CommitTx	7 : -1, 3 : -2, 5 : -2, 1 : -3, 

Input of multiple_aborts.txt:
// Multiple RW Txs test case with no deadlock
// 23 operations
log multiple_aborts.log
// op	Tx#	type
// op	Tx#	Obj
BeginTx 1 W
Read 1 1
Write 1 2
Read 1 3
Write 1 8
BeginTx 2 W
Write 2 4
Write 2 5
BeginTx 3 W
Write 3 6
Write 3 7
Read 3 9
abort 3
commit 2
abort 1
begintx 5 R
read 5 1
read 5 2
read 5 3
read 5 8
read 5 6
read 5 7
abort 5
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	WriteTx		4:1:2328		WriteLock	Granted		 P
T3	W 	BeginTx
T3	      	WriteTx		6:1:12205		WriteLock	Granted		 P
T5	R 	BeginTx
T2	      	WriteTx		5:1:2328		WriteLock	Granted		 P
T2	 	CommitTx	5 : 1, 4 : 1, 
T1	      	WriteTx		2:1:18755		WriteLock	Granted		 P
T3	      	WriteTx		7:1:12205		WriteLock	Granted		 P
T1	      	ReadTx		3:-1:18755		ReadLock	Granted		 P
T3	      	ReadTx		9:-1:12205		ReadLock	Granted		 P
T1	      	WriteTx		8:1:18755		WriteLock	Granted		 P
T3	 	AbortTx	9 : -1, 7 : 1, 6 : 1, 
T1	 	AbortTx	8 : 1, 3 : -1, 2 : 1, 1 : -1, 
T5	      	ReadTx		1:-2:10368		ReadLock	Granted		 P
T5	      	ReadTx		2:0:10368		ReadLock	Granted		 P
T5	      	ReadTx		3:-2:10368		ReadLock	Granted		 P
T5	      	ReadTx		8:0:10368		ReadLock	Granted		 P
T5	      	ReadTx		6:0:10368		ReadLock	Granted		 P
T5	      	ReadTx		7:0:10368		ReadLock	Granted		 P
T5	 	AbortTx	7 : 0, 6 : 0, 8 : 0, 3 : -2, 2 : 0, 1 : -2, 

Input of no_conflists_2Txs.txt:
// serializable history
// 2 transactions (no conflicts)
// same object accessed
// multiple times
Log no_conflicts_2Txs.log
BeginTx 1 R
Read    1 1
Read    1 2
BeginTx 2 W
Read    2 8
Read    2 7
Write   2 6
Write   2 5
Commit  2
read    1 3
read    1 4
Commit  1
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	R 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	ReadTx		8:-1:2328		ReadLock	Granted		 P
T2	      	ReadTx		7:-1:2328		ReadLock	Granted		 P
T2	      	WriteTx		6:1:2328		WriteLock	Granted		 P
T2	      	WriteTx		5:1:2328		WriteLock	Granted		 P
T1	      	ReadTx		2:-1:18755		ReadLock	Granted		 P
T2	 	CommitTx	5 : 1, 6 : 1, 7 : -1, 8 : -1, 
T1	      	ReadTx		3:-1:18755		ReadLock	Granted		 P
T1	      	ReadTx		4:-1:18755		ReadLock	Granted		 P
T1	 	CommitTx	4 : -1, 3 : -1, 2 : -1, 1 : -1, 


Input of RW_disjoint.txt:
// Multiple RW Txs test case with no deadlock
log RW_disjoint.log
// op	Tx#	type
// op	Tx#	Obj
BeginTx 1 W
Read 1 1
Write 1 2
Read 1 3
BeginTx 2 W
Write 2 4
Write 2 5
BeginTx 3 W
Write 3 6
Write 3 7
read 3 8
Commit 3
commit 2
Commit 1
begintx 5 R
read 5 9
read 5 10
read 5 11
read 5 12
read 5 13
read 5 1
commit 5
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	WriteTx		4:1:2328		WriteLock	Granted		 P
T3	W 	BeginTx
T3	      	WriteTx		6:1:12205		WriteLock	Granted		 P
T5	R 	BeginTx
T5	      	ReadTx		9:-1:10368		ReadLock	Granted		 P
T2	      	WriteTx		5:1:2328		WriteLock	Granted		 P
T2	 	CommitTx	5 : 1, 4 : 1, 
T5	      	ReadTx		10:-1:10368		ReadLock	Granted		 P
T1	      	WriteTx		2:1:18755		WriteLock	Granted		 P
T3	      	WriteTx		7:1:12205		WriteLock	Granted		 P
T5	      	ReadTx		11:-1:10368		ReadLock	Granted		 P
T5	      	ReadTx		12:-1:10368		ReadLock	Granted		 P
T1	      	ReadTx		3:-1:18755		ReadLock	Granted		 P
T3	      	ReadTx		8:-1:12205		ReadLock	Granted		 P
T5	      	ReadTx		13:-1:10368		ReadLock	Granted		 P
T3	 	CommitTx	8 : -1, 7 : 1, 6 : 1, 
T1	 	CommitTx	3 : -1, 2 : 1, 1 : -1, 
T5	      	ReadTx		1:-2:10368		ReadLock	Granted		 P
T5	 	CommitTx	1 : -2, 13 : -1, 12 : -1, 11 : -1, 10 : -1, 9 : -1, 


Input of RW_pot_ddlk.txt:
// Multiple RW Txs test case with no deadlock
log RW_pot_ddlk.log
// op	Tx#	type
// op	Tx#	Obj
BeginTx 1 W
Read 1 1
Write 1 2
Read 1 3
Write 1 8
BeginTx 2 W
Write 2 4
Write 2 5
BeginTx 3 W
Write 3 6
Write 3 7
Read 3 9
Commit 3
commit 2
Commit 1
begintx 5 R
read 5 1
read 5 2
read 5 3
read 5 8
read 5 6
read 5 7
commit 5
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		1:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T5	R 	BeginTx
T3	W 	BeginTx
T3	      	WriteTx		6:1:12205		WriteLock	Granted		 P
T2	      	WriteTx		4:1:2328		WriteLock	Granted		 P
T2	      	WriteTx		5:1:2328		WriteLock	Granted		 P
T1	      	WriteTx		2:1:18755		WriteLock	Granted		 P
T2	 	CommitTx	5 : 1, 4 : 1, 
T3	      	WriteTx		7:1:12205		WriteLock	Granted		 P
T1	      	ReadTx		3:-1:18755		ReadLock	Granted		 P
T3	      	ReadTx		9:-1:12205		ReadLock	Granted		 P
T3	 	CommitTx	9 : -1, 7 : 1, 6 : 1, 
T1	      	WriteTx		8:1:18755		WriteLock	Granted		 P
T1	 	CommitTx	8 : 1, 3 : -1, 2 : 1, 1 : -1, 
T5	      	ReadTx		1:-2:10368		ReadLock	Granted		 P
T5	      	ReadTx		2:0:10368		ReadLock	Granted		 P
T5	      	ReadTx		3:-2:10368		ReadLock	Granted		 P
T5	      	ReadTx		8:0:10368		ReadLock	Granted		 P
T5	      	ReadTx		6:0:10368		ReadLock	Granted		 P
T5	      	ReadTx		7:0:10368		ReadLock	Granted		 P
T5	 	CommitTx	7 : 0, 6 : 0, 8 : 0, 3 : -2, 2 : 0, 1 : -2, 


Input of test_abort.txt:
// simple deadlock test case
// Two write transactions
log test_abort.log
// op	Tx#	type
// op	Tx#	Obj
BeginTx 1 W
read 1 6
write 1 7
write 1 7
read 1 6
begintx 2 W
read 2 8
write 2 7
abort 2
begintx 3 R
read 3 4
write 3 5
read 3 9
commit 3
commit 1
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		6:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	ReadTx		8:-1:2328		ReadLock	Granted		 P
T3	R 	BeginTx
T3	      	ReadTx		4:-1:12205		ReadLock	Granted		 P
T2	      	WriteTx		7:1:2328		WriteLock	Granted		 P
T2	 	AbortTx	7 : 1, 8 : -1, 
T3	      	WriteTx		5:1:12205		WriteLock	Granted		 P
T1	      	WriteTx		7:2:18755		WriteLock	Granted		 P
T3	      	ReadTx		9:-1:12205		ReadLock	Granted		 P
T3	 	CommitTx	9 : -1, 5 : 1, 4 : -1, 
T1	      	WriteTx		7:3:18755		WriteLock	Granted		 P
T1	      	ReadTx		6:-2:18755		ReadLock	Granted		 P
T1	 	CommitTx	7 : 3, 6 : -2, 


Input of unlikely_ddlk.txt:
// serializable history
// ddlk unlikely
log unlikely_ddlk.log
BeginTx 1 W
Read    1 3
Read    1 2
BeginTx 2 W
Read    2 1
Write   2 3
Write   1 3
Write   1 2
Write   2 2
Commit  1
commit 2
end all

Output:
---------------------------------------------------------------------------
TxId	Txtype	Operation	ObId:Obvalue:optime	LockType	Status		TxStatus
T1	W 	BeginTx
T1	      	ReadTx		3:-1:18755		ReadLock	Granted		 P
T2	W 	BeginTx
T2	      	ReadTx		1:-1:2328		ReadLock	Granted		 P
T1	      	ReadTx		2:-1:18755		ReadLock	Granted		 P
T1	      	WriteTx		3:0:18755		WriteLock	Granted		 P
T1	      	WriteTx		2:0:18755		WriteLock	Granted		 P
T1	 	CommitTx	2 : 0, 3 : 0, 
T2	      	WriteTx		3:1:2328		WriteLock	Granted		 P
T2	      	WriteTx		2:1:2328		WriteLock	Granted		 P
T2	 	CommitTx	2 : 1, 3 : 1, 1 : -1, 

