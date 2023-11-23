// Execute environment
WSL(Ubuntu 22.04.3 LTS)                   // since 2023/11/22 A.M. 00:58 

// GNU C++ Compiler version
g++ (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0 // since 2023/11/22 A.M. 00:58 
build-essential 12.9ubuntu3               // since 2023/11/22 A.M. 00:58

// GNU Debugger
GNU gdb (Ubuntu 12.1-0ubuntu1~22.04) 12.1 // since 2023/11/22 A.M. 01:00

// how to compile
(If you want to name program by yourself)
g++ -o Program_Name Source_Code.cpp

(If you want to use default name)
g++ Source_Code.cpp

// how to use
To get unsimplified dot file, simplified kiss file, and simplified dot file: 
./Program_Name kiss_in_file dot_in_file kiss_out_file dot_out_file

To draw the graph of the unsimplified kiss file: dot -T png dot_in_file > before_minimization.png

To draw the graph of the simplified kiss file: dot -T png dot_out_file > after_minimization.png