#!/bin/sh
grep "new " *.h > new_h.txt
grep "new " *.cpp > new_cpp.txt
grep "delete " *.h > delete_h.txt
grep "delete " *.cpp > delete_cpp.txt
grep "malloc" *.h > malloc_h.txt
grep "malloc" *.cpp > malloc_cpp.txt
grep "calloc" *.h > calloc_h.txt
grep "calloc" *.cpp > calloc_cpp.txt
grep "free" *.h > free_h.txt
grep "free" *.cpp > free_cpp.txt
