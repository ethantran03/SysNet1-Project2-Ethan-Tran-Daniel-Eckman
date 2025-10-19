# SysNet1-Project2-Ethan-Tran-Daniel-Eckman

## Multi-Threaded Collatz Stopping Time Generaator

### Description:
This project computes the stopping time of the collatz sequence for each number between 1-1,000,000 using different amount of threads from 1-8. The program generates the the "N" range of collatz sequences, then uses the amount of threads specified to generate and compute the stopping time.

#### How to Build:
To run the program type "make" in the terminal.

After the -o file is created type "./mt-collatz <any range of number for the collatz sequence> <amount of threads you want to use>"
This will run the collatz sequence and give you the number of the collatz sequence, the amount of threads, and the stopping time used to generate the histogram. This will also print the 1-1000 frequency of stopping time.

You can also run the program with the "[-nolock]" function to allow race conditions due to concurrent threads sharing variables.

##### Example
"./mt-collatz 1000000 4 -nolock > histogram.csv 2> runtime.csv"
This will run the collatz sequence and create/modify two text files: one for the histogram.csv in the format of (stoppingTime,frequency) and one for runtime.csv in the format of (maxCollatzValue,threadCount,computationTime).
