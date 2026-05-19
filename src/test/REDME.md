# RedFs testing suit


In this folder "**format.c**", "**dir.c**" and "**file.c**" provide different tests of possible usage of redFs. Those are 
called through the "main.c" program that lauch the test and collect the output, highlighting if any error 
occurred. 


## How to lauch the test suit of redFs


> You can compile the test program by using the main makefile located in the root of the project. You can compile 
> using the "*test*" target: 

```bash

make -B test 

```

This will create a binary "**test**" inside "*src/test*" which is the main testing program. It also compile three different 
executable inside "*src/test/bin*" of "**format.c**", "**dir.c**" and "**file.c**". Those are the testing program for:


* the formatting process of redFs: initializing disks and creating partitions, manipulating them and so on 

* for directory and tree navigation: it create different folders and perform different operations inside them 

* for file manipulation: it create and elaborate on different files. 


> It's suggested to test the library first before using it for your own projects.


