#include <stdio.h>
#include <string.h>
#include <math.h>

#include "NewDel.h"
#include "fifo.h"

//! Construct the fifo object.
//! Allocate memory for the specified number of elements
//! and set the internal value to indicate the size of the fifo.
//! Reset the start and end data points to their clear state. The
//! clear function is called to maintain consitency and insure that
//! clear() always does the right thing.
Fifo::Fifo(uint8_t size)
{
    _size = size;
    _pdata = (FifoType*)malloc(_size * sizeof(FifoType));
    clear();
}

//! Clear the fifo by resetting the start and end pointer.
void Fifo::clear()
{
    _start = _end = _pdata;
}

//! Return the number of elements currently in the fifo
//! if the end and start pointers are the same then the fifo
//! is empty and count == 0.
//! If they differ, then we need to check for wrap-around in order
//! to properly determine the size. In the following examples a
//! = marks empty spots, while an x marks filled spots. <p>

//! <pre>
//!         s                       e                        
//!  =======xxxxxxxxxxxxxxxxxxxxxxxxx=======================
//! </pre>
//! In this case end > start, so count is equal to the distance between them or
//! \f$end - start\f$.<p>

//! <pre>
//!         e                       s                       
//!  xxxxxxxx=======================xxxxxxxxxxxxxxxxxxxxxxxx
//! </pre>
//! In this case end < start, so data wraps around. The total count is equal to the
//! size of the buffer, minus the number of blank spots, or
//! \f$size - (start - end)\f$.<p>
//! The total number of possible elements that can be stored is size -1, so
//! 
uint8_t Fifo::count()
{
    if (_end == _start) return 0;
    if (_end > _start) {
        return _end - _start;
    }
    return _size - (_start - _end);
}

//! Return true if the fifo is full.
bool Fifo::full()
{
    return (_start - _end)  == 1 || (_end - _start) == _size;
}

//! Return true if the fifo is empty.
bool Fifo::empty()
{
    return (_start == _end);
}

//! Push a new value onto the fifo. 
//! This function returns 0 if the operation succeeds, and a negative
//! value if the operation fails.
int8_t Fifo::push(FifoType* d) 
{
    if (full()) return -1;
    *(_end++) = *d;
    
    // Wrap the end back to the beginning.
    if ((_end - _pdata) > _size) {
        _end = _pdata;
    }

	return 0;
}

//! Remove the top value from the Fifo. 
//! We do not have exceptions in this simple C++ implementation, so this function
//! is not able to do anything to indicate that the called tried to pop a value
//! from an empty fifo. In that case, a zero value is returned, which is not unique
//! so the caller will have to insure that pop is never called on an empty fifo.
int8_t Fifo::pop(FifoType* pD) 
{
    if (empty()) {
        return -1; // Nothing else to do
    }
    *pD = *(_start++);
    if ((_start - _pdata) > _size) {
        _start = _pdata;
    }
    return 0;
}

//! Test function to validate that the fifo works as expected.
//! It is always a good idea to validate your data structures. Languages like
//! Python or Perl have such validation built in, while C++ does not. This function
//! can be used in a special build to run through some test cases and make sure
//! that the fifo behaves as we exect, especially in the edge cases, such as when
//! it gets full, wraps around, etc.
void FifoTest(HardwareSerial& ds)
{
    Fifo f1(20);
    Fifo::FifoType x;
    char    buffer[128];
    
    for (x=1;x<25;x++) {
        f1.push(&x);
        sprintf(buffer,"Fifo push %d count %d\n",x,f1.count());
        ds.print(buffer);
    }
    f1.clear();
    sprintf(buffer,"Fifo count %d\n",f1.count());
    ds.print(buffer);
    
    for (x=0;x<10;x++) {
        f1.push(&x);
    }
    sprintf(buffer,"Fifo count %d. Expected 10\n",f1.count());
    ds.print(buffer);
    
    Fifo::FifoType y;
    for (x=1;x<15;x++) {
        f1.pop(&y);
        sprintf(buffer,"Fifo pop %d count %d\n",y,f1.count());
        ds.print(buffer);
    }
}

