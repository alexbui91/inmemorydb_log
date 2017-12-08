--------------------
How to build and run this code
--------------------
- Prepare to build:
  + gcc 5.4+
  + make sure your machine supports g++ 11
  # install boost
  sudo apt-get install libboost-all-dev
  # http://www.boost.org/doc/libs/1_65_1/libs/gil/doc/html/gildesignguide.html
  # install libpng libjpeg
  sudo apt-get install libpng-dev
  
- Build: 
  $ unzip assignment2
  $ cd assignment2/Debug
  # before build, please edit data path in main.cpp to your local data file
  # path look like this: string path = "/home/alex/Documents/database/assignment2/raw/sample-game.csv";
  $ make clean
  $ make all
- Run (maybe use run file contained):
  $ ./assignment2 port_no
  
  
