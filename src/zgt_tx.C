/***************** Transaction class **********************/
/*** Implements methods that handle Begin, Read, Write, ***/
/*** Abort, Commit operations of transactions. These    ***/
/*** methods are passed as parameters to threads        ***/
/*** spawned by Transaction manager class.              ***/
/**********************************************************/

/* Required header files */
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include "zgt_def.h"
#include "zgt_tm.h"
#include "zgt_extern.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <pthread.h>

//Modified at 6:35 PM 09/29/2022 by Jay D. Bodra.
// Fall 2016[jay]. Removed the TxType. Now is it initialized once in the constructor

extern void *start_operation(long, long);  //start an op with mutex lock and cond wait
extern void *finish_operation(long);        //finish an op with mutex unlock and con signal

extern void *do_commit_abort(long, char);   //commit/abort based on char value
extern void *process_read_write(long, long, int, char);

extern zgt_tm *ZGT_Sh;          // Transaction manager object

/* Transaction class constructor */
/* Initializes transaction id and status and thread id */
/* Input: Transaction id, status, thread id */

//Fall 2016[jay]. Modified zgt_tx() in zgt_tx.h
zgt_tx::zgt_tx( long tid, char Txstatus,char type, pthread_t thrid){
  this->lockmode = (char)' ';  //default
  this->Txtype = type; //Fall 2022[jay] R = read only, W=Read or Write
  this->sgno =1;
  this->tid = tid;
  this->obno = -1; //set it to a invalid value
  this->status = Txstatus;
  this->pid = thrid;
  this->head = NULL;
  this->nextr = NULL;
  this->semno = -1; //init to  an invalid sem value
}

/* Method used to obtain reference to a transaction node      */
/* Inputs the transaction id. Makes a linear scan over the    */
/* linked list of transaction nodes and returns the reference */
/* of the required node if found. Otherwise returns NULL      */

zgt_tx* get_tx(long tid1){  
  zgt_tx *txptr, *lastr1;
  
  if(ZGT_Sh->lastr != NULL){    // If the list is NOT empty
      lastr1 = ZGT_Sh->lastr;   // Initialize lastr1 to first node's ptr
      for  (txptr = lastr1; (txptr != NULL); txptr = txptr->nextr)
        if (txptr->tid == tid1)         // if required id is found                                  
           return txptr; 
      return (NULL);            // if not found in list returns NULL
   }
  return(NULL);             // if list is empty returns NULL
}

/* Method that handles "BeginTx tid" in test file     */
/* Inputs a pointer to transaction id, obj pair as a struct. Creates a new  */
/* transaction node, initializes its data members and */
/* adds it to transaction list */

void *begintx(void *arg){
  //intialise a transaction object. Make sure it is 
  //done after acquiring the semaphore for the tm and making sure that 
  //the operation can proceed using the condition variable. when creating
  //the tx object, set the tx to TR_ACTIVE and obno to -1; there is no 
  //semno as yet as none is waiting on this tx.
  
  struct param *node = (struct param*)arg;// get tid and count
  start_operation(node->tid, node->count); 
    zgt_tx *tx = new zgt_tx(node->tid,TR_ACTIVE, node->Txtype, pthread_self()); // Create new tx node
 
    //Fall 2016[jay]. writes the Txtype to the file.
  
    zgt_p(0);               // Lock Tx manager; Add node to transaction list
  
    tx->nextr = ZGT_Sh->lastr;
    ZGT_Sh->lastr = tx;   
    zgt_v(0);           // Release tx manager 
  fprintf(ZGT_Sh->logfile, "T%ld\t%c \tBeginTx\n", node->tid, node->Txtype);    // Write log record and close
    fflush(ZGT_Sh->logfile);
  finish_operation(node->tid);
  pthread_exit(NULL);               // thread exit
}

/* Method to handle Readtx action in test file    */
/* Inputs a pointer to structure that contans     */
/* tx id and object no to read. Reads the object  */
/* if the object is not yet present in hash table */
/* or same tx holds a lock on it. Otherwise waits */
/* until the lock is released */

void *readtx(void *arg){
  struct param *node = (struct param*)arg;// get tid and objno and count

  //do the operations for reading. Write YOUR code
  // wait until the previous transaction operation of the given Tx is completed
  start_operation(node->tid, node->count);
  //obtain a shared lock on object and perform read operation
  process_read_write(node->tid, node->obno, node->count, 'S');
  // this read operation will continue
  finish_operation(node->tid);
  pthread_exit(NULL); // thread exit
  //return(0);
}


void *writetx(void *arg){ //do the operations for writing; similar to readTx
  struct param *node = (struct param*)arg;  // struct parameter that contains
  
  //do the operations for writing; similar to readTx. Write your code
  start_operation(node->tid, node->count);
  process_read_write(node->tid, node->obno, node->count, 'X');
  finish_operation(node->tid);
  pthread_exit(NULL); // thread exit
  //return(0);

}

//common method to process read/write: just a useful suggestion

void *process_read_write(long tid, long obno,  int count, char mode){
  // 'S' indicates read and 'X' indicates write. The tm semaphore is
  // held only when the table is modified and released before
  // so that other transactions can continue the process
  zgt_p(0);
  zgt_tx *tx = get_tx(tid);
  if (tx == NULL){
    fprintf(ZGT_Sh->logfile, 
            "Error: Transaction %ld does not exist\n", tid);
    fflush(ZGT_Sh->logfile);
    printf("Tx:%ld does not exist, operation executed on object %ld is ignored\n", tid, obno);
    fflush(stdout);
    zgt_v(0);
    return(0);
  }
  if (tx->status == TR_ABORT){
    // aborted transactions should not continue
    zgt_v(0);
    return(0);
  }

  int rc = tx->set_lock(tid, 1, obno, count, mode);
  zgt_v(0);
  if (rc == 0){
    tx->perform_readWrite(tid, obno, mode);
  }
  return(0);
}

void *aborttx(void *arg)
{
  struct param *node = (struct param*)arg;// get tid and count  

  //write your code
  start_operation(node->tid, node->count);
  do_commit_abort(node->tid, TR_ABORT);
  finish_operation(node->tid);

  //release allocated operation parameters
  delete node;
  pthread_exit(NULL);                   // thread exit
  return NULL;
  
}

void *committx(void *arg)
{
 
    //remove the locks/objects before committing
  struct param *node = (struct param*)arg;// get tid and count

  //write your code
  start_operation(node->tid, node->count);
  do_commit_abort(node->tid, TR_END);
  finish_operation(node->tid);

  //release allocated operation parameters
  delete node;
  pthread_exit(NULL);           // thread exit
  return NULL;
}

//suggestion as they are very similar

// called from commit/abort with appropriate parameter to do the actual
// operation. Make sure you give error messages if you are trying to
// commit/abort a non-existant tx

void *do_commit_abort(long t, char status){

  // write your code
  zgt_p(0);
  zgt_tx *tx = get_tx(t);
  if (tx == NULL){
    fprintf(ZGT_Sh->logfile, "Trying to %s a Tx:%ld that does not exist\n",
            (status == TR_END) ? "commit" : "abort", t);
    fflush(ZGT_Sh->logfile);
    printf("Trying to %s a Tx:%ld that does not exist\n",
            (status == TR_END) ? "commit" : "abort", t);
    fflush(stdout);
    zgt_v(0);
    return(0);
  }

  tx->status = status;   // TR_END for commit and TR_ABORT for abort
  fprintf(ZGT_Sh->logfile, "T%ld\t \t%s\t", t, (status == TR_END) ? "CommitTx" : "AbortTx");
  fflush(ZGT_Sh->logfile);

  tx->free_locks();  //removes objects from the hash table and releases Tx's

  if (tx->semno != -1){
    int numWait = 0;
    for (zgt_tx *w = ZGT_Sh->lastr; w != NULL; w = w->nextr){
      if (w->status == TR_WAIT) numWait++;
    }
     for (int i = 0; i < numWait; i++){
      zgt_v(tx->semno);
    }
  }

  tx->remove_tx(); // remove the Tx
  delete tx;
  zgt_v(0);
  return(0);
}

int zgt_tx::remove_tx ()
{
  //remove the transaction from the TM
  
  zgt_tx *txptr, *lastr1;
  lastr1 = ZGT_Sh->lastr;
  for(txptr = ZGT_Sh->lastr; txptr != NULL; txptr = txptr->nextr){  // scan through list
      if (txptr->tid == this->tid){     // if correct node is found          
         lastr1->nextr = txptr->nextr;  // update nextr value; done
         //delete this;
         return(0);
      }
      else lastr1 = txptr->nextr;           // else update prev value
   }
  fprintf(ZGT_Sh->logfile, "Trying to Remove a Tx:%ld that does not exist\n", this->tid);
  fflush(ZGT_Sh->logfile);
  printf("Trying to Remove a Tx:%ld that does not exist\n", this->tid);
  fflush(stdout);
  return(-1);
}

/* this method sets lock on objno1 with lockmode1 for a tx*/

int zgt_tx::set_lock(long tid1, long sgno1, long obno1, int count, char lockmode1){
  //if the thread has to wait, block the thread on a semaphore from the
  //sempool in the transaction manager. Set the appropriate parameters in the
  //transaction list if waiting.
  //if successful  return(0); else -1
  
    //write your code
  
  //DEBUG 1: Confirm that set_lock() was entered
  fprintf(stderr, "[DEBUG] set_lock entered: tid=%ld, obj=%ld\n", tid1, obno1);
  fflush(stderr);

  //caller holds semaphore 0, so transaction list and lock table can be examined safely
  zgt_tx *currentTx = get_tx(tid1);

  //stop if transaction doesn't exist
  if(currentTx == NULL)
  {
    return -1;
  }

  //doesnt't allow aborted or completed transaction to acquire any additional locks
  if(currentTx->status == TR_ABORT || currentTx->status == TR_END)
  {
    return -1;
  }

  //find hash table bucket for requested object
  int bucket = ZGT_Ht->hashing(sgno1, obno1);

  //remember holder we're waiting for across semaphore calls
  long blockingTid = -1;
  while(true)
  {
    zgt_hlink *link = ZGT_Sh->head[bucket];
    zgt_tx *blockingTx = NULL;
    zgt_hlink *ownLock = NULL;

    //inspect every lock entry in the object's hash bucket
    while(link != NULL)
    {
      //only entries for the requested object and segment matter
      if(link->obno == obno1 && link->sgno == sgno1)
      {
        //remember lock already held by the transaction
        if(link->tid == tid1)
        {
          ownLock = link;
        }
        //another transaction conflicts if either lock is X
        else if (lockmode1 == 'X' || link->lockmode == 'X')
        {
          blockingTx = get_tx(link->tid);

          //ignore entries whose transaction no longer exists or already finished
          if(blockingTx != NULL && blockingTx->status != TR_ABORT && blockingTx->status != TR_END)
          {
            break;
          }

          blockingTx = NULL;
        }
      }
      link = link->next;
    }

    //if transaction already has compatible lock,
    //reuse than inserting duplicate entries for repeated object accesses
    if(ownLock != NULL)
    {
      if(ownLock->lockmode == 'X' || ownLock->lockmode == lockmode1)
      {
        currentTx->status = TR_ACTIVE;
        return 0;
      }

      //allow S to X upgrade only when no other transaction hold conflicting lock the object
      if(ownLock->lockmode == 'S' && lockmode1 == 'X' && blockingTx == NULL)
      {
        ownLock->lockmode = 'X';
        currentTx->lockmode = 'X';
        currentTx->status = TR_ACTIVE;
        return 0;
      }
    }

    //if no conflicting lock exists add requested lock
    if(blockingTx == NULL)
    {
      int rc = ZGT_Ht->add(currentTx, sgno1, obno1, lockmode1);

      //has table returns -1 if allocation fails
      if(rc != 0)
      {
        fprintf(ZGT_Sh->logfile,
                "Error: could not add lock for transaction "
                "%ld on object %ld\n", tid1, obno1);
        fflush(ZGT_Sh->logfile);
        return -1;
      }

      //transaction now holds the requested lock
      currentTx->status = TR_ACTIVE;
      currentTx->sgno = sgno1;
      currentTx->obno = obno1;
      currentTx->lockmode = lockmode1;
      return 0;
    }

    //requested lock is held incompatibly by abother transaction
    blockingTid = blockingTx->tid;

    //DEBUG 2: Show which transaction is blocking this request
    fprintf(stderr, "[DEBUG] tid=%ld blocked by tid=%ld; blocker semno=%d\n", tid1, blockingTx->tid, blockingTx->semno);
    fflush(stderr);
    
    //register this transaction as waiting for the blocker, must happen before releasing semaphore 0
    currentTx->status = TR_WAIT;
    currentTx->sgno = sgno1;
    currentTx->obno = obno1;
    currentTx->lockmode = lockmode1;

    //record that blockingTx is being waited on by tid1, existing implementation
    //only supports one waiter per blocker
    if(blockingTx->semno != -1 && blockingTx->semno != (int)tid1)
    {
      fprintf(stderr, "[DEBUG] blocker %ld already has waiter %d\n", blockingTid, blockingTx->semno);
      currentTx->status = TR_ACTIVE;
      return -1;
    }
    blockingTx->setTx_semno(blockingTid, (int)tid1);

    //check whether following wait for chain leads back to transaction requesting lock
    zgt_tx *cursor = blockingTx;
    bool deadlock = false;
    while(cursor != NULL)
    {
      //if the wait chain leads back to us then the cycle exists
      if(cursor->tid == tid1)
      {
        deadlock = true;
        break;
      }

      //semno stores transaction ID this transaction is waiting for because
      //of the existing implementation
      if(cursor->semno == -1)
      {
        break;
      }
      cursor = get_tx(cursor->semno);
    }

    //if cycle exists then abort the current transaction, do_commit_abort() aquires semaphore 0 itself
    //so release it before calling the function
    if(deadlock)
    {
      fprintf(stderr,
              "Deadlock detected: aborting transaction %ld\n", tid1);
      fflush(stderr);
      zgt_v(0);
      do_commit_abort(tid1, TR_ABORT);
      
      //do not access currentTx after this point, aborting the routine may have
      //deleted the transaction object
      return -1;
    }

    //no cycle detected, release semaphore 0 and wait
    fprintf(stderr, "[DEBUG] tid=%ld going to sleep\n", tid1);
    fflush(stderr);
    zgt_v(0);
    zgt_p((int)tid1);

    //wake up and reaquire semaphore 0
    fprintf(stderr, "[DEBUG] tid=%ld woke up\n", tid1);
    fflush(stderr);
    zgt_p(0);

    currentTx = get_tx(tid1);
    if(currentTx == NULL)
    {
      return -1;
    }

    if(currentTx->status == TR_ABORT || currentTx->status == TR_END)
    {
      return -1;
    }

  }
}

int zgt_tx::free_locks()
{
  
  // this part frees all locks owned by the transaction
  // that is, remove the objects from the hash table
  // and release all Tx's waiting on this Tx

  zgt_hlink* temp = head;  //first obj of tx
  
  for(temp;temp != NULL;temp = temp->nextp){    // SCAN Tx obj list

      fprintf(ZGT_Sh->logfile, "%ld : %d, ", temp->obno, ZGT_Sh->objarray[temp->obno]->value);
      fflush(ZGT_Sh->logfile);
      
      if (ZGT_Ht->remove(this,1,(long)temp->obno) == 1){
       printf(":::ERROR:node with tid:%ld and onjno:%ld was not found for deleting", this->tid, temp->obno);        // Release from hash table
       fflush(stdout);
      }
      else {
#ifdef TX_DEBUG
       printf("\n:::Hash node with Tid:%ld, obno:%ld lockmode:%c removed\n",
                            temp->tid, temp->obno, temp->lockmode);
       fflush(stdout);
#endif
      }
    }
  fprintf(ZGT_Sh->logfile, "\n");
  fflush(ZGT_Sh->logfile);
  
  return(0);
}       

// CURREntly Not USED
// USED to COMMIT
// remove the transaction and free all associate dobjects. For the time being
// this can be used for commit of the transaction.

int zgt_tx::end_tx()  //2016: not used
{
  zgt_tx *linktx, *prevp;
  
  // USED to COMMIT 
  //remove the transaction and free all associate dobjects. For the time being 
  //this can be used for commit of the transaction.
  
  linktx = prevp = ZGT_Sh->lastr;
  
  while (linktx){
    if (linktx->tid  == this->tid) break;
    prevp  = linktx;
    linktx = linktx->nextr;
  }
  if (linktx == NULL) {
    printf("\ncannot remove a Tx node; error\n");
    fflush(stdout);
    return (1);
  }
  if (linktx == ZGT_Sh->lastr) ZGT_Sh->lastr = linktx->nextr;
  else {
    prevp = ZGT_Sh->lastr;
    while (prevp->nextr != linktx) prevp = prevp->nextr;
    prevp->nextr = linktx->nextr;    
  }
  return(0);
}

// CURRENTLY not used
int zgt_tx::cleanup()
{
  return(0);
  
}

//initial implementation by  sharma chakravarthy as part of Sentinel-oodbms project 
// routine to print the tx list
// TX_DEBUG should be defined in the Makefile to print
void zgt_tx::print_tm(){
  
  zgt_tx *txptr;
  
#ifdef TX_DEBUG
  printf("printing the tx  list \n");
  printf("Tid\tTxType\tThrid\t\tobjno\tlock\tstatus\tsemno\n");
  fflush(stdout);
#endif
  txptr=ZGT_Sh->lastr;
  while (txptr != NULL) {
#ifdef TX_DEBUG
    printf("%ld\t%c\t%ld\t%ld\t%c\t%c\t%d\n", txptr->tid, txptr->Txtype, txptr->pid, txptr->obno, txptr->lockmode, txptr->status, txptr->semno);
    fflush(stdout);
#endif
    txptr = txptr->nextr;
  }
  fflush(stdout);
}

//need to be called for printing
void zgt_tx::print_wait(){

  //route for printing for debugging
  
  printf("\n    SGNO        TxType       OBNO        TID        PID         SEMNO   L\n");
  printf("\n");
  return;
}

void zgt_tx::print_lock(){
  //routine for printing for debugging
  
  printf("\n    SGNO        OBNO        TID        PID   L\n");
  printf("\n");
  return;
}

// routine to perform the acutual read/write operation
// based  on the lockmode

void zgt_tx::perform_readWrite(long tid,long obno, char lockmode){
  
  //write your code

//find the transaction performing the operation, the caller should have
//already obtained the required lock
zgt_tx *tx = get_tx(tid);

//stop if transaction doesn't exist
if(tx == NULL)
{
  fprintf(ZGT_Sh->logfile,
          "Error: transaction %ld doesn't exist\n", tid);
  fflush(ZGT_Sh->logfile);
  return;
}

//check that the object number is within the valid range
//objarray contains MAX_ITEMS objects, indexed from 0 to MAX_ITEMS - 1
if(obno < 0 || obno >= MAX_ITEMS)
{
  fprintf(ZGT_Sh->logfile,
          "Error: invalid obkect number %ld for transaction %ld\n", obno, tid);
  fflush(ZGT_Sh->logfile);
  return;
}

//do not perform operations for aborted or completed transactions
if(tx->status == TR_ABORT || tx->status == TR_END)
{
  return;
}

//save object's current value before applying operation
int oldVal = ZGT_Sh->objarray[obno]->value;

//apply operation required by assignment and read decrements value
//write increments value
if(lockmode == 'S')
{
  ZGT_Sh->objarray[obno]->value--;
}
else if (lockmode == 'X')
{
  ZGT_Sh->objarray[obno]->value++;
}
else
{
  //unknown lock modes must not change the object
  fprintf(ZGT_Sh->logfile,
          "Error: invalid lock mode '%c' for transaction %ld\n", lockmode, tid);
  fflush(ZGT_Sh->logfile);
  return;
}

//save updated value for operation log 
int newValue = ZGT_Sh->objarray[obno]->value;

//simulate disk I/O delay assigned to the transaction, optime is configured by the
//transaction manager constructor. Use usleep only if project headers and timing convention
//specify microseconds, otherwise use supplied convention
 int optime = ZGT_Sh->optime[tid];
 //usleep(ZGT_Sh->optime[tid]);

 //record operation in transaction log and keep format consistent with assignment's sample logs
 fprintf(ZGT_Sh->logfile,
      "T%ld\t    \t%s\t\t%ld:%d:%d\t\t%s\tGranted\t\t %c\n",
       tid, (lockmode == 'S') ? "ReadTx" : "WriteTx", obno,
       ZGT_Sh->objarray[obno]->value, optime,
       (lockmode == 'S') ? "ReadLock" : "WriteLock", this->status);

 //flush log so record is written immediately
 fflush(ZGT_Sh->logfile);
 zgt_v(0);
 usleep(optime);
}

// routine that sets the semno in the Tx when another tx waits on it.
// the same number is the same as the tx number on which a Tx is waiting
int zgt_tx::setTx_semno(long tid, int semno){
  zgt_tx *txptr;
  
  txptr = get_tx(tid);
  if (txptr == NULL){
    printf("\n:::ERROR:Txid %ld wants to wait on sem:%d of tid:%ld which does not exist\n", this->tid, semno, tid);
    fflush(stdout);
    exit(1);
  }
  if ((txptr->semno == -1)|| (txptr->semno == semno)){  //just to be safe
    txptr->semno = semno;
    return(0);
  }
  else if (txptr->semno != semno){
#ifdef TX_DEBUG
    printf(":::ERROR Trying to wait on sem:%d, but on Tx:%ld\n", semno, txptr->tid);
    fflush(stdout);
#endif
    exit(1);
  }
  return(0);
}

void *start_operation(long tid, long count){
  
  pthread_mutex_lock(&ZGT_Sh->mutexpool[tid]);  // Lock mutex[t] to make other
  // threads of same transaction to wait
  
  while(ZGT_Sh->condset[tid] != count)      // wait if condset[t] is != count
    pthread_cond_wait(&ZGT_Sh->condpool[tid],&ZGT_Sh->mutexpool[tid]);
  return(0);
}

// Otherside of teh start operation;
// signals the conditional broadcast

void *finish_operation(long tid){
  ZGT_Sh->condset[tid]--;   // decr condset[tid] for allowing the next op
  pthread_cond_broadcast(&ZGT_Sh->condpool[tid]);// other waiting threads of same tx
  pthread_mutex_unlock(&ZGT_Sh->mutexpool[tid]); 
  return(0);
}